#include <aios/core/EventLog.hpp>
#include <aios/core/SimulationClock.hpp>
#include <aios/core/types.hpp>
#include <aios/cpu/CPU.hpp>
#include <aios/interrupt/InterruptManager.hpp>
#include <aios/memory/Memory.hpp>
#include <aios/process/ProcessManager.hpp>
#include <aios/scheduling/Scheduler.hpp>

#include <doctest/doctest.h>

#include <string>
#include <vector>

using namespace aios;

namespace {

// Minimal wiring: memory + event log + clock + CPU + process manager +
// scheduler, all owned by the test (docs/15 section 10).
struct Harness {
    Memory memory;
    EventLog log;
    SimulationClock clock;
    CPU cpu;
    ProcessManager pm;
    Scheduler sched;

    Harness() {
        pm.setMemory(&memory);
        pm.setEventLog(&log);
        pm.setCPU(&cpu);
        pm.setClock(&clock);
        cpu.setMemory(&memory);
        cpu.setEventLog(&log);
        cpu.setClock(&clock);
        sched.setProcessManager(&pm);
        sched.setEventLog(&log);
        sched.setClock(&clock);
    }

    int create(const std::string& name, int priority) {
        // One HALT instruction is enough: the tests assert scheduling order,
        // not execution results.
        return pm.createProcess(name, priority, {Instruction{Opcode::HALT, 0}.toWord()}, 0);
    }

    bool hasEvent(EventType type) const {
        for (const auto& e : log.events()) {
            if (e.type == type) {
                return true;
            }
        }
        return false;
    }
};

} // namespace

TEST_CASE("T-SCHED-001: schedulingPolicyToString maps every policy") {
    CHECK(schedulingPolicyToString(SchedulingPolicy::FCFS) == "FCFS");
    CHECK(schedulingPolicyToString(SchedulingPolicy::ROUND_ROBIN) == "ROUND_ROBIN");
    CHECK(schedulingPolicyToString(SchedulingPolicy::PRIORITY) == "PRIORITY");
}

TEST_CASE("T-SCHED-002: FCFS selects processes in arrival order (docs/15 section 11)") {
    Harness h;
    h.sched.setPolicy(SchedulingPolicy::FCFS);
    const int p1 = h.create("p1", 0);
    const int p2 = h.create("p2", 0);
    const int p3 = h.create("p3", 0);
    REQUIRE(h.pm.admit(p1));
    REQUIRE(h.pm.admit(p2));
    REQUIRE(h.pm.admit(p3));

    CHECK(h.sched.schedule() == p1); // P1 -> P2 -> P3
    CHECK(h.pm.runningPid() == p1);
    CHECK(h.sched.schedule() == p1); // FCFS does not switch while P1 runs
    REQUIRE(h.pm.terminate(p1));
    CHECK(h.sched.schedule() == p2);
    REQUIRE(h.pm.terminate(p2));
    CHECK(h.sched.schedule() == p3);
    REQUIRE(h.pm.terminate(p3));
}

TEST_CASE("T-SCHED-003: FCFS is non-preemptive (docs/08 section 7.2)") {
    Harness h;
    h.sched.setPolicy(SchedulingPolicy::FCFS);
    const int p1 = h.create("p1", 0);
    const int p2 = h.create("p2", 0);
    REQUIRE(h.pm.admit(p1));
    REQUIRE(h.pm.admit(p2));

    CHECK(h.sched.schedule() == p1);
    // A new READY process does not preempt the running one.
    CHECK(h.sched.schedule() == p1);
    CHECK(h.pm.runningPid() == p1);
    CHECK(h.sched.contextSwitchCount() == 1);
}

TEST_CASE("T-SCHED-004: Round Robin preempts on quantum expiry (docs/15 section 12)") {
    Harness h;
    h.sched.setPolicy(SchedulingPolicy::ROUND_ROBIN);
    h.sched.setTimeQuantum(2);
    const int p1 = h.create("p1", 0);
    const int p2 = h.create("p2", 0);
    const int p3 = h.create("p3", 0);
    REQUIRE(h.pm.admit(p1));
    REQUIRE(h.pm.admit(p2));
    REQUIRE(h.pm.admit(p3));

    CHECK(h.sched.schedule() == p1);
    // Each quantum expiry rotates the running process to the back.
    CHECK(h.sched.onTimeQuantumExpired() == p2);
    CHECK(h.sched.onTimeQuantumExpired() == p3);
    CHECK(h.sched.onTimeQuantumExpired() == p1);
    CHECK(h.sched.onTimeQuantumExpired() == p2);
}

TEST_CASE("T-SCHED-005: Round Robin keeps the running process when it is alone") {
    Harness h;
    h.sched.setPolicy(SchedulingPolicy::ROUND_ROBIN);
    const int p1 = h.create("p1", 0);
    REQUIRE(h.pm.admit(p1));

    CHECK(h.sched.schedule() == p1);
    CHECK(h.sched.onTimeQuantumExpired() == p1);
    CHECK(h.pm.runningPid() == p1);
    CHECK(h.sched.contextSwitchCount() == 1);
}

TEST_CASE("T-SCHED-006: FCFS and PRIORITY ignore the timer (docs/08 section 13)") {
    Harness h;
    h.sched.setPolicy(SchedulingPolicy::FCFS);
    const int p1 = h.create("p1", 0);
    const int p2 = h.create("p2", 0);
    REQUIRE(h.pm.admit(p1));
    REQUIRE(h.pm.admit(p2));

    CHECK(h.sched.schedule() == p1);
    CHECK(h.sched.onTimeQuantumExpired() == p1); // no preemption
    CHECK(h.pm.runningPid() == p1);

    h.pm.reset();
    h.sched.reset();
    h.sched.setPolicy(SchedulingPolicy::PRIORITY);
    const int q1 = h.create("q1", 1);
    const int q2 = h.create("q2", 1);
    REQUIRE(h.pm.admit(q1));
    REQUIRE(h.pm.admit(q2));
    CHECK(h.sched.schedule() == q1);
    CHECK(h.sched.onTimeQuantumExpired() == q1); // no preemption
    CHECK(h.pm.runningPid() == q1);
}

TEST_CASE("T-SCHED-007: PRIORITY selects the highest-priority READY process (docs/15 section 13)") {
    Harness h;
    h.sched.setPolicy(SchedulingPolicy::PRIORITY);
    const int p1 = h.create("p1", 1);
    const int p2 = h.create("p2", 3);
    const int p3 = h.create("p3", 2);
    REQUIRE(h.pm.admit(p1));
    REQUIRE(h.pm.admit(p2));
    REQUIRE(h.pm.admit(p3));

    CHECK(h.sched.schedule() == p2); // priority 3 wins
    CHECK(h.pm.runningPid() == p2);
}

TEST_CASE("T-SCHED-008: PRIORITY preempts for a strictly higher-priority arrival (docs/08 section 13)") {
    Harness h;
    h.sched.setPolicy(SchedulingPolicy::PRIORITY);
    const int p1 = h.create("p1", 2);
    const int p2 = h.create("p2", 5);
    REQUIRE(h.pm.admit(p1));
    CHECK(h.sched.schedule() == p1);

    REQUIRE(h.pm.admit(p2)); // higher priority becomes READY
    CHECK(h.sched.schedule() == p2);
    CHECK(h.pm.runningPid() == p2);
    CHECK(h.hasEvent(EventType::PROCESS_PREEMPTED));
}

TEST_CASE("T-SCHED-009: PRIORITY does not preempt on equal or lower priority") {
    Harness h;
    h.sched.setPolicy(SchedulingPolicy::PRIORITY);
    const int p1 = h.create("p1", 4);
    const int p2 = h.create("p2", 4);
    const int p3 = h.create("p3", 2);
    REQUIRE(h.pm.admit(p1));
    CHECK(h.sched.schedule() == p1);

    REQUIRE(h.pm.admit(p2)); // equal priority
    CHECK(h.sched.schedule() == p1);
    REQUIRE(h.pm.admit(p3)); // lower priority
    CHECK(h.sched.schedule() == p1);
    CHECK(h.pm.runningPid() == p1);
}

TEST_CASE("T-SCHED-010: PRIORITY tie-breaks by earliest ready, then lowest pid") {
    Harness h;
    h.sched.setPolicy(SchedulingPolicy::PRIORITY);
    const int p3 = h.create("p3", 2);
    const int p1 = h.create("p1", 2);
    REQUIRE(h.pm.admit(p3));
    REQUIRE(h.pm.admit(p1));

    // Same priority: earlier admission wins; equal ready cycle -> lower pid.
    CHECK(h.sched.schedule() == p3);
}

TEST_CASE("T-SCHED-011: setPolicy emits SCHEDULER_STARTED (docs/08 section 40)") {
    Harness h;
    CHECK(h.sched.policy() == SchedulingPolicy::FCFS); // default
    h.sched.setPolicy(SchedulingPolicy::ROUND_ROBIN);
    CHECK(h.sched.policyName() == "ROUND_ROBIN");
    CHECK(h.hasEvent(EventType::SCHEDULER_STARTED));
}

TEST_CASE("T-SCHED-012: schedule records decisions, events and context switches") {
    Harness h;
    h.sched.setPolicy(SchedulingPolicy::ROUND_ROBIN);
    const int p1 = h.create("p1", 0);
    const int p2 = h.create("p2", 0);
    REQUIRE(h.pm.admit(p1));
    REQUIRE(h.pm.admit(p2));

    CHECK(h.sched.schedule() == p1);
    h.sched.onTimeQuantumExpired();

    CHECK(h.hasEvent(EventType::PROCESS_SELECTED));
    CHECK(h.hasEvent(EventType::TIME_QUANTUM_EXPIRED));
    CHECK(h.sched.contextSwitchCount() == 2);
    CHECK(h.sched.decisionCount() == 2);
    REQUIRE(h.sched.history().size() == 2);
    CHECK(h.sched.history()[0].selectedPid == p1);
    CHECK(h.sched.history()[1].selectedPid == p2);
    CHECK(h.sched.history()[1].preemptedPid == p1);
    CHECK(h.sched.history()[1].policy == SchedulingPolicy::ROUND_ROBIN);
    CHECK_FALSE(h.sched.history()[1].reason.empty());
}

TEST_CASE("T-SCHED-013: schedule returns INVALID_PID when nothing is READY") {
    Harness h;
    h.sched.setPolicy(SchedulingPolicy::FCFS);
    const int p1 = h.create("p1", 0);
    REQUIRE(h.pm.admit(p1));
    // FAILED is the terminal state reachable from READY (docs/06 section 15);
    // terminate() requires RUNNING or WAITING.
    REQUIRE(h.pm.markFailed(p1));

    CHECK(h.sched.schedule() == INVALID_PID);
    CHECK(h.pm.runningPid() == INVALID_PID);
}

TEST_CASE("T-SCHED-014: reconcile drops blocked/terminated pids and admits new ones") {
    Harness h;
    h.sched.setPolicy(SchedulingPolicy::FCFS);
    const int p1 = h.create("p1", 0);
    const int p2 = h.create("p2", 0);
    const int p3 = h.create("p3", 0);
    REQUIRE(h.pm.admit(p1));
    REQUIRE(h.pm.admit(p2));
    CHECK(h.sched.schedule() == p1);
    REQUIRE(h.pm.terminate(p1));

    // p3 admitted after the first schedule() must still be picked in order.
    REQUIRE(h.pm.admit(p3));
    CHECK(h.sched.schedule() == p2);
    REQUIRE(h.pm.terminate(p2));
    CHECK(h.sched.schedule() == p3);
}

TEST_CASE("T-SCHED-015: reset clears policy, history and counters") {
    Harness h;
    h.sched.setPolicy(SchedulingPolicy::ROUND_ROBIN);
    const int p1 = h.create("p1", 0);
    REQUIRE(h.pm.admit(p1));
    CHECK(h.sched.schedule() == p1);

    h.sched.reset();
    CHECK(h.sched.policy() == SchedulingPolicy::FCFS);
    CHECK(h.sched.decisionCount() == 0);
    CHECK(h.sched.contextSwitchCount() == 0);
    CHECK(h.sched.history().empty());
}

TEST_CASE("T-SCHED-016: TIMER interrupt drives Round Robin through the interrupt manager (docs/08 section 24)") {
    Harness h;
    InterruptManager im;
    im.setEventLog(&h.log);
    im.setScheduler(&h.sched);
    h.sched.setPolicy(SchedulingPolicy::ROUND_ROBIN);
    const int p1 = h.create("p1", 0);
    const int p2 = h.create("p2", 0);
    REQUIRE(h.pm.admit(p1));
    REQUIRE(h.pm.admit(p2));
    CHECK(h.sched.schedule() == p1);

    CHECK(im.generateInterrupt(InterruptType::TIMER, INVALID_PID, 4) > INVALID_INTERRUPT);
    REQUIRE(im.serviceNextInterrupt());
    CHECK(im.phase() == InterruptPhase::COMPLETED);
    CHECK(h.pm.runningPid() == p2);
    CHECK(h.sched.contextSwitchCount() == 2);
}