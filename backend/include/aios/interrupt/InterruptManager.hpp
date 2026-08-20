#pragma once

#include <aios/core/EventLog.hpp>
#include <aios/core/types.hpp>

#include <deque>
#include <vector>

namespace aios {

class SystemCallManager;
class ProcessManager;
class MemoryManager;

// ---------------------------------------------------------------------------
// Interrupt manager (docs/05).
// Requests pass through PENDING -> SERVICING -> COMPLETED (docs/05 section 29).
// The pending queue is serviced in priority order (docs/05 section 19); among
// requests of equal priority the oldest is serviced first (docs/05 section 21).
// Interrupts that arrive while an ISR is executing are queued/deferred, never
// nested (docs/05 section 20). dispatch() is the interrupt vector mapping each
// interrupt type to its handler (docs/05 section 9).
// ---------------------------------------------------------------------------
class InterruptManager {
public:
    InterruptManager() = default;

    void setEventLog(EventLog* log) { eventLog_ = log; }
    void setSystemCallManager(SystemCallManager* m) { syscalls_ = m; }
    void setProcessManager(ProcessManager* m) { processes_ = m; }
    void setMemoryManager(MemoryManager* m) { memoryManager_ = m; }

    // Priority of an interrupt type. Lower value = higher priority
    // (docs/05 section 19, finalized during implementation):
    // ERROR > PAGE_FAULT > SYSTEM_CALL > IO_COMPLETE > TIMER.
    // This is the single place to change the ordering.
    static int priorityOf(InterruptType type);

    // Adds an interrupt to the pending queue; returns its id.
    int generateInterrupt(InterruptType type, int pid, int32_t data);

    // Dispatches the highest-priority pending interrupt and marks it COMPLETED.
    // Returns false when the queue is empty or an ISR is already executing
    // (nested interrupts are deferred, docs/05 section 20).
    bool serviceNextInterrupt();

    size_t pendingCount() const { return pending_.size(); }
    bool hasPending() const { return !pending_.empty(); }
    InterruptPhase phase() const { return phase_; }
    const std::vector<InterruptRequest>& log() const { return log_; }

    // Pending queue snapshot (docs/05 section 34 getPendingInterrupts()).
    std::vector<InterruptRequest> pendingInterrupts() const;

    void reset();

private:
    void dispatch(const InterruptRequest& req);
    void record(EventType type, const std::string& detail);

    std::deque<InterruptRequest> pending_;
    std::vector<InterruptRequest> log_;
    InterruptPhase phase_ = InterruptPhase::IDLE;
    int nextId_ = 1;

    EventLog* eventLog_ = nullptr;
    SystemCallManager* syscalls_ = nullptr;
    ProcessManager* processes_ = nullptr;
    MemoryManager* memoryManager_ = nullptr;
};

} // namespace aios