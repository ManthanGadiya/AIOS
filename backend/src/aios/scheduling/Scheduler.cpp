#include <aios/scheduling/Scheduler.hpp>

#include <aios/core/SimulationClock.hpp>
#include <aios/process/ProcessControlBlock.hpp>
#include <aios/process/ProcessManager.hpp>

#include <algorithm>
#include <utility>

namespace aios {

std::string schedulingPolicyToString(SchedulingPolicy policy) {
    switch (policy) {
        case SchedulingPolicy::FCFS:        return "FCFS";
        case SchedulingPolicy::ROUND_ROBIN: return "ROUND_ROBIN";
        case SchedulingPolicy::PRIORITY:    return "PRIORITY";
    }
    return "UNKNOWN";
}

void Scheduler::setPolicy(SchedulingPolicy policy) {
    if (policy_ == policy) {
        return;
    }
    policy_ = policy;
    // A new policy starts with a fresh ready order (docs/08 section 3).
    readyOrder_.clear();
    record(EventType::SCHEDULER_STARTED, INVALID_PID, policyName());
}

int Scheduler::currentRunning() const {
    return processes_ ? processes_->runningPid() : INVALID_PID;
}

void Scheduler::reconcile() {
    // Drop pids that are no longer READY (dispatched, blocked, terminated).
    std::deque<int> kept;
    for (const int pid : readyOrder_) {
        const auto* pcb = processes_ ? processes_->getProcess(pid) : nullptr;
        if (pcb && pcb->state == ProcessState::READY) {
            kept.push_back(pid);
        }
    }
    readyOrder_ = std::move(kept);
    // Append newly-READY pids (admit / wake / preempt) in pid order. Pids that
    // became READY in the same reconcile keep arrival order by construction:
    // earlier admits are appended on earlier calls.
    if (processes_) {
        for (const int pid : processes_->readyQueue()) {
            if (std::find(readyOrder_.begin(), readyOrder_.end(), pid) ==
                readyOrder_.end()) {
                readyOrder_.push_back(pid);
            }
        }
    }
}

int Scheduler::pickReady() {
    reconcile();
    if (readyOrder_.empty()) {
        return INVALID_PID;
    }
    return readyOrder_.front();
}

int Scheduler::pickHighestPriority() const {
    // Larger priority value = higher priority (docs/08 section 11). Ties go to
    // the earliest READY arrival, then the lowest pid.
    int best = INVALID_PID;
    if (!processes_) {
        return best;
    }
    for (const int pid : processes_->readyQueue()) {
        const auto* pcb = processes_->getProcess(pid);
        if (!pcb || pcb->state != ProcessState::READY) {
            continue;
        }
        if (best == INVALID_PID) {
            best = pid;
            continue;
        }
        const auto* cur = processes_->getProcess(best);
        if (pcb->priority > cur->priority) {
            best = pid;
        } else if (pcb->priority == cur->priority) {
            if (pcb->readySinceCycle < cur->readySinceCycle ||
                (pcb->readySinceCycle == cur->readySinceCycle && pid < best)) {
                best = pid;
            }
        }
    }
    return best;
}

void Scheduler::dispatchSelected(int pid, int preempted,
                                 const std::string& reason) {
    if (!processes_ || !processes_->dispatch(pid)) {
        return;
    }
    // The dispatched pid leaves the ready order; a Round Robin preemption is
    // re-enqueued at the back (docs/08 section 9).
    readyOrder_.erase(std::remove(readyOrder_.begin(), readyOrder_.end(), pid),
                      readyOrder_.end());
    if (preempted != INVALID_PID && policy_ == SchedulingPolicy::ROUND_ROBIN) {
        readyOrder_.push_back(preempted);
    }
    ++contextSwitchCount_;
    history_.push_back(
        SchedulingDecision{clock_ ? clock_->cycle() : 0, policy_, pid,
                           preempted, reason});
    record(EventType::PROCESS_SELECTED, pid,
           policyName() + " selected pid=" + std::to_string(pid) + " [" +
               reason + "]");
    if (preempted != INVALID_PID) {
        record(EventType::PROCESS_PREEMPTED, preempted,
               "pid=" + std::to_string(preempted) + " preempted by pid=" +
                   std::to_string(pid));
    }
}

int Scheduler::schedule() {
    if (!processes_) {
        return INVALID_PID;
    }
    const int running = currentRunning();

    if (policy_ == SchedulingPolicy::PRIORITY) {
        const int best = pickHighestPriority();
        if (best == INVALID_PID) {
            return running;
        }
        if (running == INVALID_PID) {
            dispatchSelected(best, INVALID_PID, "cpu idle");
            return currentRunning();
        }
        const auto* runPcb = processes_->getProcess(running);
        const auto* bestPcb = processes_->getProcess(best);
        if (runPcb && bestPcb && bestPcb->priority > runPcb->priority) {
            dispatchSelected(best, running, "higher priority");
            return currentRunning();
        }
        return running; // nothing strictly better; keep the running process
    }

    // FCFS / Round Robin: dispatch from the front of the ready order when the
    // CPU is free. Round Robin preemption happens on the timer instead
    // (onTimeQuantumExpired).
    if (running != INVALID_PID) {
        return running;
    }
    const int next = pickReady();
    if (next == INVALID_PID) {
        return INVALID_PID;
    }
    dispatchSelected(next, INVALID_PID, "cpu idle");
    return currentRunning();
}

int Scheduler::onTimeQuantumExpired() {
    if (!processes_) {
        return INVALID_PID;
    }
    const int running = currentRunning();
    if (policy_ != SchedulingPolicy::ROUND_ROBIN) {
        // FCFS and Priority are not preempted by the timer.
        return running;
    }
    if (running == INVALID_PID) {
        return schedule();
    }
    record(EventType::TIME_QUANTUM_EXPIRED, running,
           "pid=" + std::to_string(running) +
               " quantum=" + std::to_string(timeQuantum_));
    const int next = pickReady();
    if (next == INVALID_PID || next == running) {
        return running; // nothing else to run
    }
    dispatchSelected(next, running, "quantum expired");
    return currentRunning();
}

void Scheduler::record(EventType type, int pid, const std::string& detail) {
    if (eventLog_) {
        eventLog_->record(type, pid, clock_ ? clock_->cycle() : 0, detail);
    }
}

void Scheduler::reset() {
    readyOrder_.clear();
    history_.clear();
    contextSwitchCount_ = 0;
    policy_ = SchedulingPolicy::FCFS;
}

} // namespace aios