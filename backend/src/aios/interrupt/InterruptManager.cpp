#include <aios/interrupt/InterruptManager.hpp>

#include <aios/interrupt/SystemCallManager.hpp>
#include <aios/memory/MemoryManager.hpp>
#include <aios/process/ProcessManager.hpp>
#include <aios/scheduling/Scheduler.hpp>

#include <iterator>

namespace aios {

int InterruptManager::priorityOf(InterruptType type) {
    // docs/05 section 19: lower value = higher priority. Order finalized
    // during implementation: ERROR > PAGE_FAULT > SYSTEM_CALL > IO_COMPLETE >
    // TIMER. Changing the ordering here does not require touching dispatch().
    switch (type) {
        case InterruptType::ERROR: return 10;
        case InterruptType::PAGE_FAULT: return 20;
        case InterruptType::SYSTEM_CALL: return 30;
        case InterruptType::IO_COMPLETE: return 40;
        case InterruptType::TIMER: return 50;
    }
    return 100;
}

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
    // Nested interrupts are deferred: while an ISR is executing the manager
    // does not start another one (docs/05 section 20).
    if (phase_ == InterruptPhase::SERVICING) {
        return false;
    }
    if (pending_.empty()) {
        return false;
    }
    // Service the highest-priority pending request (docs/05 section 19). Among
    // requests of equal priority the oldest is serviced first (FIFO within a
    // priority class).
    auto best = pending_.begin();
    for (auto it = std::next(pending_.begin()); it != pending_.end(); ++it) {
        if (priorityOf(it->type) < priorityOf(best->type)) {
            best = it;
        }
    }
    const InterruptRequest req = *best;
    pending_.erase(best);
    dispatch(req);
    return true;
}

std::vector<InterruptRequest> InterruptManager::pendingInterrupts() const {
    return {pending_.begin(), pending_.end()};
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
    for (auto& entry : log_) {
        if (entry.id == req.id) {
            entry.phase = InterruptPhase::SERVICING;
            break;
        }
    }
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
    } else if (req.type == InterruptType::TIMER) {
        // The time quantum expired: Round Robin preempts the running process
        // (docs/08 section 24). FCFS and PRIORITY ignore the timer.
        if (scheduler_) {
            scheduler_->onTimeQuantumExpired();
        }
    }
    // The IO_COMPLETE handler is a placeholder in this milestone: it will drive
    // the I/O manager (docs/11). It is still logged and completed.

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