#include <aios/memory/MemoryManager.hpp>

#include <aios/memory/Memory.hpp>

#include <algorithm>
#include <string>
#include <utility>

namespace aios {

void MemoryManager::initialize(uint32_t pageSize, uint32_t frameCount) {
    pageSize_ = pageSize > 0 ? pageSize : 4;
    frameCount_ = frameCount;
    frames_.assign(frameCount_, FrameEntry{});
    fifoQueue_.clear();
}

bool MemoryManager::allocateProcessMemory(int pid,
                                          const std::vector<int32_t>& program) {
    if (!memory_ || program.empty()) {
        return false;
    }
    const uint32_t pageCount =
        (static_cast<uint32_t>(program.size()) + pageSize_ - 1) / pageSize_;

    PageTable pt;
    pt.pid = pid;
    pt.pageCount = pageCount;
    pt.entries.assign(pageCount, PageTableEntry{}); // valid=true, present=false

    // Store the whole image in swap; each page padded to pageSize words.
    for (uint32_t p = 0; p < pageCount; ++p) {
        std::vector<int32_t> words(pageSize_, 0);
        const size_t start = static_cast<size_t>(p) * pageSize_;
        const size_t count = std::min<size_t>(pageSize_, program.size() - start);
        std::copy(program.begin() + static_cast<long>(start),
                  program.begin() + static_cast<long>(start + count), words.begin());
        setSwapPage(pid, p, std::move(words));
    }

    pageTables_.emplace(pid, std::move(pt));
    record(EventType::PAGE_ALLOCATED, pid, "pages=" + std::to_string(pageCount) +
                                            " pageSize=" + std::to_string(pageSize_));
    return true;
}

void MemoryManager::releaseProcessMemory(int pid) {
    const auto ptIt = pageTables_.find(pid);
    if (ptIt == pageTables_.end()) {
        return;
    }

    // Free every frame owned by this process.
    for (auto& frame : frames_) {
        if (frame.occupied && frame.pid == pid) {
            frame = FrameEntry{};
        }
    }
    // Drop freed frames from the FIFO order (occupied frames are kept in order).
    std::deque<uint32_t> kept;
    for (const uint32_t f : fifoQueue_) {
        if (frames_[f].occupied) {
            kept.push_back(f);
        }
    }
    fifoQueue_ = std::move(kept);

    // Release swap words, then drop the page table and swap image.
    const auto swapIt = swap_.find(pid);
    if (swapIt != swap_.end()) {
        for (const auto& [page, words] : swapIt->second) {
            (void)page;
            swapWords_ -= words.size();
        }
        swap_.erase(swapIt);
    }
    pageTables_.erase(ptIt);
}

MemoryAccessResult MemoryManager::accessMemory(int pid, uint32_t logicalAddress,
                                               bool write, int32_t value) {
    MemoryAccessResult result;
    if (!memory_) {
        return result; // INVALID
    }
    PageTable* pt = mutablePageTable(pid);
    if (!pt) {
        return result; // INVALID
    }
    const uint32_t page = logicalAddress / pageSize_;
    const uint32_t offset = logicalAddress % pageSize_;
    if (page >= pt->pageCount || !pt->entries[page].valid) {
        return result; // INVALID
    }

    PageTableEntry& entry = pt->entries[page];
    if (!entry.present) {
        ++pageFaultCount_;
        ++perProcessFaults_[pid];
        record(EventType::PAGE_FAULT, pid, "page=" + std::to_string(page));
        result.status = MemoryAccessStatus::PAGE_FAULT;
        result.page = page;
        return result;
    }

    entry.referenced = true;
    result.physicalAddress = entry.frame * pageSize_ + offset;
    record(EventType::PAGE_ACCESSED, pid,
           "addr=" + std::to_string(logicalAddress) + " -> phys=" +
               std::to_string(result.physicalAddress) + " " + (write ? "W" : "R"));
    if (write) {
        entry.dirty = true;
        if (memory_->write(result.physicalAddress, value)) {
            result.status = MemoryAccessStatus::OK;
        }
        return result;
    }
    const auto word = memory_->read(result.physicalAddress);
    if (word) {
        result.status = MemoryAccessStatus::OK;
        result.value = *word;
    }
    return result;
}

bool MemoryManager::handlePageFault(int pid, uint32_t page) {
    if (!memory_) {
        return false;
    }
    PageTable* pt = mutablePageTable(pid);
    if (!pt || page >= pt->pageCount || !pt->entries[page].valid) {
        return false;
    }
    PageTableEntry& entry = pt->entries[page];
    if (entry.present) {
        return true; // already resident (double fault)
    }

    uint32_t frame = 0;
    if (!allocateFrame(pid, page, frame)) {
        return false;
    }
    loadFrameFromSwap(pid, page, frame);

    entry.frame = frame;
    entry.present = true;
    entry.referenced = false;
    entry.dirty = false;
    record(EventType::PAGE_LOADED, pid,
           "page=" + std::to_string(page) + " frame=" + std::to_string(frame));
    return true;
}

const PageTable* MemoryManager::getPageTable(int pid) const {
    const auto it = pageTables_.find(pid);
    return it == pageTables_.end() ? nullptr : &it->second;
}

bool MemoryManager::isValidLogicalAddress(int pid, uint32_t logicalAddress) const {
    const PageTable* pt = getPageTable(pid);
    if (!pt) {
        return false;
    }
    const uint32_t page = logicalAddress / pageSize_;
    return page < pt->pageCount && pt->entries[page].valid;
}

MemoryStatistics MemoryManager::getStatistics() const {
    MemoryStatistics stats;
    stats.pageSize = pageSize_;
    stats.totalFrames = frameCount_;
    stats.totalWords = frameCount_ * pageSize_;
    stats.usedWords = 0;
    for (const auto& frame : frames_) {
        if (frame.occupied) {
            ++stats.usedFrames;
            stats.usedWords += pageSize_;
        }
    }
    stats.pageFaultCount = pageFaultCount_;
    stats.pageReplacementCount = pageReplacementCount_;
    stats.swapWords = swapWords_;
    stats.perProcessFaults = perProcessFaults_;
    return stats;
}

void MemoryManager::reset() {
    frames_.clear();
    fifoQueue_.clear();
    pageTables_.clear();
    swap_.clear();
    pageFaultCount_ = 0;
    pageReplacementCount_ = 0;
    swapWords_ = 0;
    perProcessFaults_.clear();
    frames_.assign(frameCount_, FrameEntry{});
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

PageTable* MemoryManager::mutablePageTable(int pid) {
    const auto it = pageTables_.find(pid);
    return it == pageTables_.end() ? nullptr : &it->second;
}

bool MemoryManager::translate(int pid, uint32_t logicalAddress, uint32_t& page,
                              uint32_t& offset, uint32_t& frame) const {
    const PageTable* pt = getPageTable(pid);
    if (!pt) {
        return false;
    }
    page = logicalAddress / pageSize_;
    offset = logicalAddress % pageSize_;
    if (page >= pt->pageCount || !pt->entries[page].valid) {
        return false;
    }
    frame = pt->entries[page].frame;
    return true;
}

bool MemoryManager::allocateFrame(int pid, uint32_t page, uint32_t& frameOut) {
    for (uint32_t f = 0; f < frameCount_; ++f) {
        if (!frames_[f].occupied) {
            frames_[f] = FrameEntry{true, pid, page};
            fifoQueue_.push_back(f);
            record(EventType::FRAME_ALLOCATED, pid,
                   "frame=" + std::to_string(f) + " page=" + std::to_string(page));
            frameOut = f;
            return true;
        }
    }
    uint32_t victim = 0;
    if (!evictFifo(victim)) {
        return false;
    }
    frames_[victim] = FrameEntry{true, pid, page};
    fifoQueue_.push_back(victim);
    frameOut = victim;
    return true;
}

bool MemoryManager::evictFifo(uint32_t& frameOut) {
    if (fifoQueue_.empty()) {
        return false;
    }
    const uint32_t frame = fifoQueue_.front();
    fifoQueue_.pop_front();

    FrameEntry victim = frames_[frame];
    if (!victim.occupied) {
        frameOut = frame;
        return true;
    }

    // Write a dirty victim back to swap before reusing its frame
    // (docs/07 section 21; simplified: clean pages keep their swap copy).
    PageTable* victimPt = mutablePageTable(victim.pid);
    if (victimPt && victim.page < victimPt->pageCount) {
        PageTableEntry& victimEntry = victimPt->entries[victim.page];
        if (victimEntry.dirty) {
            writeFrameToSwap(victim.pid, victim.page, frame);
        }
        victimEntry.present = false;
        victimEntry.frame = 0;
        victimEntry.referenced = false;
        victimEntry.dirty = false;
    }

    ++pageReplacementCount_;
    record(EventType::PAGE_REPLACED, victim.pid,
           "victim_page=" + std::to_string(victim.page) +
               " frame=" + std::to_string(frame));
    frames_[frame] = FrameEntry{};
    frameOut = frame;
    return true;
}

void MemoryManager::setSwapPage(int pid, uint32_t page,
                                const std::vector<int32_t>& words) {
    auto& pages = swap_[pid];
    const auto it = pages.find(page);
    if (it != pages.end()) {
        swapWords_ -= it->second.size();
    }
    pages[page] = words;
    swapWords_ += words.size();
}

bool MemoryManager::getSwapPage(int pid, uint32_t page,
                                std::vector<int32_t>& out) const {
    const auto pidIt = swap_.find(pid);
    if (pidIt == swap_.end()) {
        return false;
    }
    const auto pageIt = pidIt->second.find(page);
    if (pageIt == pidIt->second.end()) {
        return false;
    }
    out = pageIt->second;
    return true;
}

void MemoryManager::loadFrameFromSwap(int pid, uint32_t page, uint32_t frame) {
    std::vector<int32_t> words;
    if (!getSwapPage(pid, page, words)) {
        words.assign(pageSize_, 0);
    }
    const uint32_t base = frame * pageSize_;
    for (uint32_t i = 0; i < pageSize_; ++i) {
        memory_->write(base + i, words[i]);
    }
}

void MemoryManager::writeFrameToSwap(int pid, uint32_t page, uint32_t frame) {
    std::vector<int32_t> words(pageSize_, 0);
    const uint32_t base = frame * pageSize_;
    for (uint32_t i = 0; i < pageSize_; ++i) {
        const auto word = memory_->read(base + i);
        words[i] = word ? *word : 0;
    }
    setSwapPage(pid, page, std::move(words));
    record(EventType::PAGE_SWAPPED_OUT, pid,
           "page=" + std::to_string(page) + " frame=" + std::to_string(frame));
}

void MemoryManager::record(EventType type, int pid, const std::string& detail) {
    if (!eventLog_) {
        return;
    }
    eventLog_->record(type, pid, clock_ ? clock_->cycle() : 0, detail);
}

} // namespace aios