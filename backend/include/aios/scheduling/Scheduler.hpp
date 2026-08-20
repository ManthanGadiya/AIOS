#pragma once

#include <aios/core/EventLog.hpp>
#include <aios/core/types.hpp>

#include <cstdint>
#include <deque>
#include <string>
#include <vector>

namespace aios {

class ProcessManager;
class SimulationClock;

// Scheduling policy selected for a run (docs/08 sections 3 and 7-13). Only one
// policy is active per simulation; the AI adaptive policy is added in a later
// milestone (docs/08 section 44).
enum class SchedulingPolicy : int32_t {
    FCFS = 0,        // first-come first-served, non-preemptive (docs/08 section 7)
    ROUND_ROBIN = 1, // time-sliced with a quantum, preemptive (docs/08 sections 8-10)
    PRIORITY = 2     // larger value = higher priority, preemptive (docs/08 sections 11-13)
};

std::string schedulingPolicyToString(SchedulingPolicy policy);

// One scheduling decision (docs/08 section 40). The AI adaptive scheduler
// extends this with a score and reason in a later milestone (docs/08 section 44).
struct SchedulingDecision {
    uint64_t cycle = 0;
    SchedulingPolicy policy = SchedulingPolicy::FCFS;
    int selectedPid = INVALID_PID;
    int preemptedPid = INVALID_PID;
    std::string reason;
};

// ---------------------------------------------------------------------------
// Scheduler (docs/08).
// Selects the next process to run according to the configured policy. The
// scheduler keeps a persistent ready order (docs/08 section 5) that is
// reconciled with the process manager's READY set on every decision: pids that
// are no longer READY (blocked/terminated) are dropped and newly-READY pids
// (admit/wake) are appended in pid order. FCFS and Round Robin pick the front
// of that order; Round Robin moves the preempted process to the back when the
// timer quantum expires (docs/08 section 9). Priority scans the READY set and
// preempts when a strictly higher-priority process becomes ready (docs/08
// section 13). Actual context switching is delegated to ProcessManager::dispatch
// (docs/06), which saves the running context and loads the new one.
// ---------------------------------------------------------------------------
class Scheduler {
public:
    Scheduler() = default;

    void setProcessManager(ProcessManager* pm) { processes_ = pm; }
    void setEventLog(EventLog* log) { eventLog_ = log; }
    void setClock(SimulationClock* clock) { clock_ = clock; }

    void setPolicy(SchedulingPolicy policy);
    SchedulingPolicy policy() const { return policy_; }
    std::string policyName() const { return schedulingPolicyToString(policy_); }

    // Round Robin time quantum in cycles (docs/08 sections 8-10). Defaults to
    // 4, matching the docs/05 section 32 timer example.
    void setTimeQuantum(uint64_t quantum) { timeQuantum_ = quantum; }
    uint64_t timeQuantum() const { return timeQuantum_; }

    // Makes a scheduling decision now (docs/08 section 41 schedule()): picks
    // the next READY process when the CPU is free, or preempts the running
    // process for the strict-higher-priority one under PRIORITY. Returns the
    // pid now running, or INVALID_PID when the CPU stays idle.
    int schedule();

    // Round Robin: the running process's quantum expired. Preempts it to the
    // back of the ready queue and dispatches the next process (docs/08 section
    // 9). Under FCFS/PRIORITY the timer does not preempt; the running process
    // is kept. Returns the pid now running.
    int onTimeQuantumExpired();

    // Scheduler statistics (docs/08 section 25).
    size_t decisionCount() const { return history_.size(); }
    uint64_t contextSwitchCount() const { return contextSwitchCount_; }
    const std::vector<SchedulingDecision>& history() const { return history_; }

    void reset();

private:
    int currentRunning() const;
    void reconcile();   // sync the persistent ready order with the PM READY set
    int pickReady();    // front of the reconciled order (FCFS / Round Robin)
    int pickHighestPriority() const; // best READY process (PRIORITY)
    void dispatchSelected(int pid, int preempted, const std::string& reason);
    void record(EventType type, int pid, const std::string& detail);

    SchedulingPolicy policy_ = SchedulingPolicy::FCFS;
    uint64_t timeQuantum_ = 4;
    std::deque<int> readyOrder_;
    std::vector<SchedulingDecision> history_;
    uint64_t contextSwitchCount_ = 0;

    ProcessManager* processes_ = nullptr;
    EventLog* eventLog_ = nullptr;
    SimulationClock* clock_ = nullptr;
};

} // namespace aios