#pragma once

#include <aios/core/EventLog.hpp>
#include <aios/core/SimulationClock.hpp>
#include <aios/core/types.hpp>

#include <cstdint>
#include <deque>
#include <map>
#include <vector>

namespace aios {

class Memory;

// ---------------------------------------------------------------------------
// Memory Manager (docs/07).
// Owns the frame table, per-process page tables, simulated swap space, page
// replacement (FIFO baseline) and memory statistics. Physical words live in
// the flat Memory; frame i occupies words [i*pageSize, (i+1)*pageSize).
//
// Demand paging (docs/07 section 29): pages are not preloaded. A process's
// program image is stored in swap at creation; the first access to each page
// faults and loads it.  [decision D8]
//
// Backward compatibility: the manager is optional. The CPU / ProcessManager
// use the legacy flat-Memory path when no MemoryManager is attached, so the
// Stage I tests keep passing unchanged.
// ---------------------------------------------------------------------------
class MemoryManager {
public:
    MemoryManager() = default;

    void setMemory(Memory* m) { memory_ = m; }
    void setEventLog(EventLog* log) { eventLog_ = log; }
    void setClock(SimulationClock* clock) { clock_ = clock; }

    // Sets page size and frame count. Must be called before use.
    // Defaults (4 words, 8 frames) match docs/07 section 5.
    void initialize(uint32_t pageSize, uint32_t frameCount);
    
    uint32_t getPageSize() const { return pageSize_; }
    uint32_t getFrameCount() const { return frameCount_; }

    // Creates a page table for `pid` and stores the program image in swap.
    // Pages are valid but not present (demand paging). Returns false if the
    // program is empty or memory is not attached.
    bool allocateProcessMemory(int pid, const std::vector<int32_t>& program);

    // Frees every frame owned by `pid`, then drops its page table + swap
    // image (docs/07 section 38).
    void releaseProcessMemory(int pid);

    // Translates a logical address and reads (write=false) or writes
    // (write=true) one word. Returns OK with value/physicalAddress, or
    // PAGE_FAULT (valid but not resident) / INVALID (docs/07 section 36).
    MemoryAccessResult accessMemory(int pid, uint32_t logicalAddress,
                                    bool write, int32_t value);

    // Loads the faulting page into a frame: free frame if available, otherwise
    // FIFO replacement. Called by the interrupt manager when a PAGE_FAULT
    // interrupt is serviced (docs/07 section 16).
    bool handlePageFault(int pid, uint32_t page);

    const PageTable* getPageTable(int pid) const;
    const std::vector<FrameEntry>& frames() const { return frames_; }
    // True when `logicalAddress` lies inside the process's valid logical space
    // (page table present and page valid). Used by the CPU to validate jump
    // targets in paged mode, where logical space may exceed physical RAM.
    bool isValidLogicalAddress(int pid, uint32_t logicalAddress) const;

    MemoryStatistics getStatistics() const;

    void reset();

private:
    PageTable* mutablePageTable(int pid);
    bool translate(int pid, uint32_t logicalAddress, uint32_t& page,
                   uint32_t& offset, uint32_t& frame) const;
    // Marks a frame occupied by (pid, page); performs FIFO eviction when no
    // frame is free. Returns false only when memory is not attached.
    bool allocateFrame(int pid, uint32_t page, uint32_t& frameOut);
    // Evicts the FIFO victim (oldest allocated frame). Writes the victim page
    // back to swap if dirty; updates its owner's page table. Returns the
    // freed frame index, or false when no frame is occupied.
    bool evictFifo(uint32_t& frameOut);
    void setSwapPage(int pid, uint32_t page, const std::vector<int32_t>& words);
    bool getSwapPage(int pid, uint32_t page, std::vector<int32_t>& out) const;
    void loadFrameFromSwap(int pid, uint32_t page, uint32_t frame);
    void writeFrameToSwap(int pid, uint32_t page, uint32_t frame);
    void record(EventType type, int pid, const std::string& detail);

    Memory* memory_ = nullptr;
    EventLog* eventLog_ = nullptr;
    SimulationClock* clock_ = nullptr;

    uint32_t pageSize_ = 4;
    uint32_t frameCount_ = 8;
    std::vector<FrameEntry> frames_;
    std::deque<uint32_t> fifoQueue_; // allocated frame indices, oldest first
    std::map<int, PageTable> pageTables_;
    // Simulated swap: pid -> page -> words (each page padded to pageSize).
    std::map<int, std::map<uint32_t, std::vector<int32_t>>> swap_;

    uint64_t pageFaultCount_ = 0;
    uint64_t pageReplacementCount_ = 0;
    uint64_t swapWords_ = 0;
    std::map<int, uint64_t> perProcessFaults_;
};

} // namespace aios