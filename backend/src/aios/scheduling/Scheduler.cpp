#include <aios/scheduling/Scheduler.hpp>
#include <aios/agent/AIAgentManager.hpp>

#include <aios/core/SimulationClock.hpp>
#include <aios/process/ProcessControlBlock.hpp>
#include <aios/process/ProcessManager.hpp>

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <utility>

namespace aios {

std::string schedulingPolicyToString(SchedulingPolicy policy) {
    switch (policy) {
        case SchedulingPolicy::FCFS:        return "FCFS";
        case SchedulingPolicy::ROUND_ROBIN: return "ROUND_ROBIN";
        case SchedulingPolicy::PRIORITY:    return "PRIORITY";
        case SchedulingPolicy::AI_ADAPTIVE: return "AI_ADAPTIVE";
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

std::vector<int> Scheduler::readyQueue() const {
    return std::vector<int>(readyOrder_.begin(), readyOrder_.end());
}

// AI Adaptive: Score = Priority + Waiting + Urgency - ResourceCost
// (docs/12 section 15, 16)
double Scheduler::calculateAIScore(int pid) const {
    if (!processes_) return 0.0;
    
    const auto* pcb = processes_->getProcess(pid);
    if (!pcb || pcb->state != ProcessState::READY) {
        return -1.0;
    }
    
    double score = 0.0;
    std::ostringstream reason;
    
    // Priority Contribution (0-20 range, scaled)
    double priorityContrib = static_cast<double>(pcb->priority) * 2.0;
    score += priorityContrib;
    reason << "+ Priority(" << pcb->priority << "=" << priorityContrib << ")";
    
    // Waiting Contribution (time in READY queue)
    uint64_t waitingCycles = 0;
    if (pcb->inReadyQueue && clock_) {
        waitingCycles = clock_->cycle() - pcb->readySinceCycle;
    } else {
        waitingCycles = pcb->waitingTime;
    }
    double waitingContrib = std::min(static_cast<double>(waitingCycles) * 0.5, 20.0);
    score += waitingContrib;
    reason << " + Waiting(" << waitingCycles << "=" << waitingContrib << ")";
    
    // Urgency Contribution (for AI agents)
    double urgencyContrib = 0.0;
    if (aiAgents_ && pcb->type == ProcessType::AI_AGENT) {
        auto agentMeta = aiAgents_->getAgentByPid(pid);
        if (agentMeta) {
            // Urgency based on token budget remaining ratio
            double tokenRatio = static_cast<double>(agentMeta->tokenUsed) / 
                                std::max<uint64_t>(1, agentMeta->tokenBudget);
            urgencyContrib = (1.0 - tokenRatio) * 15.0; // Higher urgency when tokens low
            // Also consider agent state
            if (agentMeta->agentState == AgentState::WAITING) {
                urgencyContrib += 10.0;
            }
            reason << " + Urgency(" << urgencyContrib << ")";
        }
    }
    score += urgencyContrib;
    
    // Resource Cost Penalty
    double resourceCost = 0.0;
    if (aiAgents_ && pcb->type == ProcessType::AI_AGENT) {
        auto agentMeta = aiAgents_->getAgentByPid(pid);
        if (agentMeta) {
            // CPU usage ratio
            double cpuRatio = agentMeta->quota.cpuQuota > 0 ? 
                static_cast<double>(agentMeta->quota.cpuUsed) / agentMeta->quota.cpuQuota : 0.0;
            // Memory usage ratio
            double memRatio = agentMeta->quota.memoryQuota > 0 ? 
                static_cast<double>(agentMeta->quota.memoryUsed) / agentMeta->quota.memoryQuota : 0.0;
            resourceCost = (cpuRatio + memRatio) * 10.0;
            reason << " - ResourceCost(" << resourceCost << ")";
        }
    } else {
        // For normal processes, use cpuTime as proxy
        resourceCost = std::min(pcb->cpuTime * 0.1, 15.0);
        reason << " - ResourceCost(" << resourceCost << ")";
    }
    score -= resourceCost;
    
    return score;
}

int Scheduler::pickAIAdaptive() {
    reconcile();
    if (readyOrder_.empty()) {
        return INVALID_PID;
    }
    
    int bestPid = INVALID_PID;
    double bestScore = -1.0;
    std::string bestReason;
    
    for (int pid : readyOrder_) {
        const auto* pcb = processes_ ? processes_->getProcess(pid) : nullptr;
        if (!pcb || pcb->state != ProcessState::READY) {
            continue;
        }
        
        double score = calculateAIScore(pid);
        if (score > bestScore) {
            bestScore = score;
            bestPid = pid;
            // Build reason string
            std::ostringstream reason;
            reason << "AI score=" << std::fixed << std::setprecision(1) << score;
            bestReason = reason.str();
        }
    }
    
    // Store the reason for the selected process
    if (bestPid != INVALID_PID) {
        // We'll pass the reason through dispatchSelected
    }
    
    return bestPid;
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
                                 const std::string& reason, double score, const std::string& scoreReason) {
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
                           preempted, reason, score, scoreReason});
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

    if (policy_ == SchedulingPolicy::AI_ADAPTIVE) {
        const int best = pickAIAdaptive();
        if (best == INVALID_PID) {
            return running;
        }
        if (running == INVALID_PID) {
            double score = calculateAIScore(best);
            dispatchSelected(best, INVALID_PID, "cpu idle", score, "AI adaptive: highest score");
            return currentRunning();
        }
        const auto* runPcb = processes_->getProcess(running);
        const auto* bestPcb = processes_->getProcess(best);
        if (runPcb && bestPcb) {
            double runScore = calculateAIScore(running);
            double bestScore = calculateAIScore(best);
            if (bestScore > runScore + 5.0) { // Threshold to avoid thrashing
                dispatchSelected(best, running, "higher AI score", bestScore, 
                                "AI adaptive: score " + std::to_string(bestScore) + " > " + std::to_string(runScore));
                return currentRunning();
            }
        }
        return running;
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