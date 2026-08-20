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
// Requests are queued FIFO in Stage I (docs/05 section 20: queue policy is
// finalized during implementation; priority arbitration is Week 4+). Each
// request passes through PENDING -> SERVICING -> COMPLETED.
// ---------------------------------------------------------------------------
class InterruptManager {
public:
    InterruptManager() = default;

    void setEventLog(EventLog* log) { eventLog_ = log; }
    void setSystemCallManager(SystemCallManager* m) { syscalls_ = m; }
    void setProcessManager(ProcessManager* m) { processes_ = m; }
    void setMemoryManager(MemoryManager* m) { memoryManager_ = m; }

    // Adds an interrupt to the pending queue; returns its id.
    int generateInterrupt(InterruptType type, int pid, int32_t data);

    // Pops the oldest pending interrupt, dispatches it, marks COMPLETED.
    // Returns false when the queue is empty.
    bool serviceNextInterrupt();

    size_t pendingCount() const { return pending_.size(); }
    bool hasPending() const { return !pending_.empty(); }
    InterruptPhase phase() const { return phase_; }
    const std::vector<InterruptRequest>& log() const { return log_; }

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