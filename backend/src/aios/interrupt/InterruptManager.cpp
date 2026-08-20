#include <aios/interrupt/InterruptManager.hpp>

#include <aios/interrupt/SystemCallManager.hpp>
#include <aios/memory/MemoryManager.hpp>
#include <aios/process/ProcessManager.hpp>

namespace aios {

int InterruptManager::generateInterrupt(InterruptType type, int pid, int32_t data) {
    InterruptRequest req;
    req.id = nextId_++;
    req.type = type;
    req.pid = pid;
    req.data = data;
    req.phase = InterruptPhase::PENDING;
    pending_.push_back(req);
    log_.push_back(req);
    phase_ = InterruptPhase::PENDING;
    return req.id;
}

bool InterruptManager::serviceNextInterrupt() {
    if (pending_.empty()) {
        return false;
    }
    const InterruptRequest req = pending_.front();
    pending_.pop_front();
    dispatch(req);
    return true;
}

void InterruptManager::reset() {
    pending_.clear();
    log_.clear();
    phase_ = InterruptPhase::IDLE;
    nextId_ = 1;
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

void InterruptManager::dispatch(const InterruptRequest& req) {
    phase_ = InterruptPhase::SERVICING;
    record(EventType::INTERRUPT, interruptTypeToString(req.type) + " pid=" +
                                     std::to_string(req.pid) +
                                     " data=" + std::to_string(req.data));

    if (req.type == InterruptType::SYSTEM_CALL) {
        if (syscalls_) {
            const SyscallResult result =
                syscalls_->handleSystemCall(SyscallRequest{req.pid, req.data});
            // An unrecognized syscall is fatal for the calling process
            // (docs/05 section 4).
            if (result.status == SyscallStatus::INVALID && processes_ &&
                req.pid != INVALID_PID) {
                processes_->terminate(req.pid);
            }
        }
    } else if (req.type == InterruptType::ERROR) {
        if (processes_ && req.pid != INVALID_PID) {
            processes_->markFailed(req.pid);
        }
    } else if (req.type == InterruptType::PAGE_FAULT) {
        // Load the faulting page so the CPU can retry the instruction.
        if (memoryManager_ && req.pid != INVALID_PID) {
            memoryManager_->handlePageFault(req.pid, static_cast<uint32_t>(req.data));
        }
    }
    // TIMER and IO_COMPLETE are observed and logged only in Stage I
    // (docs/05 section 20: queue policy finalized during implementation).

    for (auto& entry : log_) {
        if (entry.id == req.id) {
            entry.phase = InterruptPhase::COMPLETED;
            break;
        }
    }
    phase_ = InterruptPhase::COMPLETED;
}

void InterruptManager::record(EventType type, const std::string& detail) {
    if (!eventLog_) {
        return;
    }
    eventLog_->record(type, INVALID_PID, 0, detail);
}

} // namespace aios