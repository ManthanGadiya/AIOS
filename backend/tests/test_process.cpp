#include <aios/core/EventLog.hpp>
#include <aios/core/SimulationClock.hpp>
#include <aios/core/types.hpp>
#include <aios/cpu/CPU.hpp>
#include <aios/memory/Memory.hpp>
#include <aios/process/ProcessManager.hpp>

#include <doctest/doctest.h>

#include <sstream>
#include <vector>

using namespace aios;

namespace {

// Minimal wiring: memory + event log + clock + CPU, all owned by the test.
struct Harness {
    Memory memory;
    EventLog log;
    SimulationClock clock;
    CPU cpu;
    ProcessManager pm;

    Harness() {
        pm.setMemory(&memory);
        pm.setEventLog(&log);
        pm.setCPU(&cpu);
        pm.setClock(&clock);
        cpu.setMemory(&memory);
        cpu.setEventLog(&log);
        cpu.setClock(&clock);
    }

    int create(const std::string& name, int priority, std::vector<int32_t> program,
               uint32_t base) {
        return pm.createProcess(name, priority, program, base);
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

// Program: ACC = mem[base+0]; store to mem[base+1]; write mem[base+1]; halt.
std::vector<int32_t> smallProgram() {
    return {Instruction{Opcode::LOAD, 0}.toWord(), Instruction{Opcode::STORE, 1}.toWord(),
            Instruction{Opcode::WRITE, 1}.toWord(), Instruction{Opcode::HALT, 0}.toWord()};
}

} // namespace

TEST_CASE("T-PROC-001: createProcess loads program and initializes PCB") {
    Harness h;
    const int pid = h.create("agent-a", 3, smallProgram(), 0);
    CHECK(pid == 1);
    CHECK(h.memory.read(0) == Instruction{Opcode::LOAD, 0}.toWord());

    const auto* pcb = h.pm.getProcess(pid);
    REQUIRE(pcb != nullptr);
    CHECK(pcb->name == "agent-a");
    CHECK(pcb->priority == 3);
    CHECK(pcb->state == ProcessState::NEW);
    CHECK(pcb->baseAddress == 0);
    CHECK(pcb->programSize == 4);
    CHECK(pcb->context.pc == 0);
    CHECK(h.hasEvent(EventType::PROCESS_CREATED));
}

TEST_CASE("T-PROC-002: createProcess fails on memory overflow") {
    Harness h;
    std::vector<int32_t> big(Memory::MEMORY_SIZE + 1, 0);
    CHECK(h.pm.createProcess("overflow", 0, big, 0) == INVALID_PID);
}

TEST_CASE("T-PROC-003: admit moves NEW to READY only once") {
    Harness h;
    const int pid = h.create("a", 1, smallProgram(), 0);
    CHECK(h.pm.admit(pid));
    CHECK(h.pm.getProcess(pid)->state == ProcessState::READY);
    CHECK_FALSE(h.pm.admit(pid)); // READY -> READY is not legal
    CHECK_FALSE(h.pm.admit(999)); // unknown pid
}

TEST_CASE("T-PROC-004: dispatch moves READY to RUNNING and loads CPU") {
    Harness h;
    const int pid = h.create("a", 1, smallProgram(), 0);
    REQUIRE(h.pm.admit(pid));
    CHECK(h.pm.dispatch(pid));
    CHECK(h.pm.getProcess(pid)->state == ProcessState::RUNNING);
    CHECK(h.cpu.state() == CpuState::RUNNING);
    CHECK(h.cpu.currentProcess() == pid);
    CHECK(h.hasEvent(EventType::CONTEXT_SWITCH));
    CHECK_FALSE(h.pm.dispatch(pid)); // RUNNING -> RUNNING is not legal
}

TEST_CASE("T-PROC-005: dispatch preempts the running process") {
    Harness h;
    const int a = h.create("a", 1, smallProgram(), 0);
    const int b = h.create("b", 2, smallProgram(), 10);
    REQUIRE(h.pm.admit(a));
    REQUIRE(h.pm.admit(b));
    REQUIRE(h.pm.dispatch(a));
    CHECK(h.cpu.currentProcess() == a);

    REQUIRE(h.pm.dispatch(b));
    CHECK(h.pm.getProcess(a)->state == ProcessState::READY);
    CHECK(h.pm.getProcess(b)->state == ProcessState::RUNNING);
    CHECK(h.cpu.currentProcess() == b);
}

TEST_CASE("T-PROC-006: terminate ends a running process and resets the CPU") {
    Harness h;
    const int pid = h.create("a", 1, smallProgram(), 0);
    REQUIRE(h.pm.admit(pid));
    REQUIRE(h.pm.dispatch(pid));
    CHECK(h.pm.terminate(pid));
    CHECK(h.pm.getProcess(pid)->state == ProcessState::TERMINATED);
    CHECK_FALSE(h.pm.isAlive(pid));
    CHECK(h.cpu.state() == CpuState::IDLE);
    CHECK(h.cpu.currentProcess() == INVALID_PID);
    CHECK(h.hasEvent(EventType::PROCESS_TERMINATED));
}

TEST_CASE("T-PROC-007: terminate rejects a non-running process") {
    Harness h;
    const int pid = h.create("a", 1, smallProgram(), 0);
    REQUIRE(h.pm.admit(pid));
    CHECK_FALSE(h.pm.terminate(pid)); // READY cannot terminate
    CHECK(h.pm.getProcess(pid)->state == ProcessState::READY);
    CHECK_FALSE(h.pm.terminate(999));
}

TEST_CASE("T-PROC-008: block and wake round-trip") {
    Harness h;
    const int pid = h.create("a", 1, smallProgram(), 0);
    REQUIRE(h.pm.admit(pid));
    REQUIRE(h.pm.dispatch(pid));
    CHECK(h.pm.blockProcess(pid));
    CHECK(h.pm.getProcess(pid)->state == ProcessState::WAITING);
    CHECK(h.cpu.currentProcess() == INVALID_PID);
    CHECK(h.pm.wakeProcess(pid));
    CHECK(h.pm.getProcess(pid)->state == ProcessState::READY);
    CHECK_FALSE(h.pm.blockProcess(pid)); // READY cannot block
}

TEST_CASE("T-PROC-009: markFailed flags an unrecoverable error") {
    Harness h;
    const int pid = h.create("a", 1, smallProgram(), 0);
    REQUIRE(h.pm.admit(pid));
    REQUIRE(h.pm.dispatch(pid));
    CHECK(h.pm.markFailed(pid));
    CHECK(h.pm.getProcess(pid)->state == ProcessState::FAILED);
    CHECK_FALSE(h.pm.isAlive(pid));
    CHECK_FALSE(h.pm.terminate(pid)); // terminal state has no outgoing edges
}

TEST_CASE("T-PROC-010: readyQueue and processIds reflect PCB table") {
    Harness h;
    const int a = h.create("a", 1, smallProgram(), 0);
    const int b = h.create("b", 2, smallProgram(), 10);
    const int c = h.create("c", 3, smallProgram(), 20);
    REQUIRE(h.pm.admit(a));
    REQUIRE(h.pm.admit(b));
    REQUIRE(h.pm.admit(c));
    REQUIRE(h.pm.dispatch(b));

    const auto ready = h.pm.readyQueue();
    REQUIRE(ready.size() == 2);
    CHECK(ready[0] == a);
    CHECK(ready[1] == c);

    const auto all = h.pm.processIds();
    CHECK(all.size() == 3);
    CHECK(h.pm.processCount() == 3);
    CHECK(h.pm.runningPid() == b);
}

TEST_CASE("T-PROC-011: invalid transitions are rejected") {
    Harness h;
    const int pid = h.create("a", 1, smallProgram(), 0);
    CHECK_FALSE(h.pm.dispatch(pid));      // NEW cannot dispatch
    CHECK_FALSE(h.pm.terminate(pid));     // NEW cannot terminate
    CHECK_FALSE(h.pm.markFailed(999));    // unknown pid
}

TEST_CASE("T-PROC-012: reset clears the table and PID counter") {
    Harness h;
    const int a = h.create("a", 1, smallProgram(), 0);
    const int b = h.create("b", 2, smallProgram(), 10);
    CHECK(a == 1);
    CHECK(b == 2);
    h.pm.reset();
    CHECK(h.pm.processCount() == 0);
    CHECK(h.create("c", 3, smallProgram(), 20) == 1);
}

TEST_CASE("T-PROC-013: preemption saves and restores CPU context") {
    Harness h;
    const int a = h.create("a", 1, smallProgram(), 0);
    const int b = h.create("b", 2, smallProgram(), 10);
    REQUIRE(h.pm.admit(a));
    REQUIRE(h.pm.admit(b));
    REQUIRE(h.pm.dispatch(a));

    // A runs: LOAD 0 advances PC to 1 and loads mem[0] into ACC.
    h.clock.tick();
    h.cpu.executeCycle();
    REQUIRE(h.cpu.registers().pc == 1);

    // Preempt A with B, then dispatch A again: context must be restored.
    REQUIRE(h.pm.dispatch(b));
    CHECK(h.cpu.registers().pc == 10);
    REQUIRE(h.pm.dispatch(a));
    CHECK(h.cpu.currentProcess() == a);
    CHECK(h.cpu.registers().pc == 1);
    CHECK(h.cpu.registers().acc == Instruction{Opcode::LOAD, 0}.toWord());
    CHECK(h.pm.getProcess(b)->state == ProcessState::READY);
}

TEST_CASE("T-PROC-014: createAgentProcess marks the PCB as an AI agent") {
    Harness h;
    const int normal = h.create("proc", 3, smallProgram(), 0);
    const int agent = h.pm.createAgentProcess("agent-a", 2, smallProgram(), 10);
    CHECK(h.pm.getProcess(normal)->type == ProcessType::NORMAL);
    CHECK(h.pm.getProcess(agent)->type == ProcessType::AI_AGENT);
    CHECK(processTypeToString(ProcessType::NORMAL) == "NORMAL");
    CHECK(processTypeToString(ProcessType::AI_AGENT) == "AI_AGENT");
}

TEST_CASE("T-PROC-015: cpuTime counts cycles on the CPU") {
    Harness h;
    const int pid = h.create("a", 1, smallProgram(), 0);
    REQUIRE(h.pm.admit(pid));
    REQUIRE(h.pm.dispatch(pid)); // dispatched at cycle 0

    // Still uncharged while running.
    CHECK(h.pm.getProcess(pid)->cpuTime == 0);

    // Run three cycles.
    for (int i = 0; i < 3; ++i) {
        h.clock.tick();
        h.cpu.executeCycle();
    }
    CHECK(h.pm.getProcess(pid)->cpuTime == 0);

    // Charged on release: 3 - 0 = 3.
    REQUIRE(h.pm.terminate(pid));
    CHECK(h.pm.getProcess(pid)->cpuTime == 3);
}

TEST_CASE("T-PROC-016: cpuTime is charged across preemptions") {
    Harness h;
    const int a = h.create("a", 1, smallProgram(), 0);
    const int b = h.create("b", 2, smallProgram(), 10);
    REQUIRE(h.pm.admit(a));
    REQUIRE(h.pm.admit(b));
    REQUIRE(h.pm.dispatch(a)); // cycle 0

    h.clock.tick();
    h.cpu.executeCycle(); // cycle 1
    h.clock.tick();
    h.cpu.executeCycle(); // cycle 2

    // Preempt a with b: a is charged 2 - 0 = 2.
    REQUIRE(h.pm.dispatch(b));
    CHECK(h.pm.getProcess(a)->cpuTime == 2);

    h.clock.tick();
    h.cpu.executeCycle(); // cycle 3, b runs
    REQUIRE(h.pm.terminate(b)); // b charged 3 - 2 = 1
    CHECK(h.pm.getProcess(b)->cpuTime == 1);

    // Re-dispatch a: context restored, keeps accruing.
    REQUIRE(h.pm.dispatch(a)); // cycle 3
    h.clock.tick();
    h.cpu.executeCycle(); // cycle 4
    REQUIRE(h.pm.terminate(a)); // a charged 4 - 3 = 1 more; total 3
    CHECK(h.pm.getProcess(a)->cpuTime == 3);
}

TEST_CASE("T-PROC-017: waitingTime accumulates in the READY queue") {
    Harness h;
    const int a = h.create("a", 1, smallProgram(), 0);
    REQUIRE(h.pm.admit(a)); // cycle 0; ready since 0
    h.clock.tick();
    h.clock.tick(); // cycle 2
    REQUIRE(h.pm.dispatch(a)); // waiting 2 - 0 = 2
    CHECK(h.pm.getProcess(a)->waitingTime == 2);

    // Block then wake: waiting accrues again from the wake cycle.
    h.clock.tick(); // cycle 3
    REQUIRE(h.pm.blockProcess(a));
    h.clock.tick(); // cycle 4
    REQUIRE(h.pm.wakeProcess(a)); // ready since 4
    h.clock.tick(); // cycle 5
    REQUIRE(h.pm.dispatch(a)); // waiting 5 - 4 = 1 more; total 3
    CHECK(h.pm.getProcess(a)->waitingTime == 3);
}

TEST_CASE("T-PROC-018: context switches and response time are tracked") {
    Harness h;
    const int a = h.create("a", 1, smallProgram(), 0);
    const int b = h.create("b", 2, smallProgram(), 10);
    REQUIRE(h.pm.admit(a));
    REQUIRE(h.pm.admit(b));
    REQUIRE(h.pm.dispatch(a)); // first run at cycle 0
    CHECK(h.pm.getProcess(a)->contextSwitchCount == 1);
    CHECK(h.pm.getProcess(a)->firstRunCycle == 0);

    REQUIRE(h.pm.dispatch(b)); // b's first run
    CHECK(h.pm.getProcess(b)->contextSwitchCount == 1);
    CHECK(h.pm.getProcess(b)->firstRunCycle == 0);

    REQUIRE(h.pm.dispatch(a)); // a's second dispatch
    CHECK(h.pm.getProcess(a)->contextSwitchCount == 2);
    CHECK(h.pm.getProcess(a)->firstRunCycle == 0); // unchanged

    const auto s = h.pm.getProcessStatistics(a);
    REQUIRE(s.has_value());
    CHECK(s->responseTime == 0);
    CHECK(s->contextSwitches == 2);
}

TEST_CASE("T-PROC-019: state changes and response time") {
    Harness h;
    const int pid = h.create("a", 1, smallProgram(), 0); // cycle 0
    h.clock.tick();
    h.clock.tick(); // cycle 2
    REQUIRE(h.pm.admit(pid)); // NEW -> READY (1)
    h.clock.tick(); // cycle 3
    REQUIRE(h.pm.dispatch(pid)); // READY -> RUNNING (2); first run at 3
    const auto* pcb = h.pm.getProcess(pid);
    CHECK(pcb->stateChangeCount == 2);
    CHECK(pcb->firstRunCycle == 3);

    const auto s = h.pm.getProcessStatistics(pid);
    REQUIRE(s.has_value());
    CHECK(s->arrivalTime == 0);
    CHECK(s->responseTime == 3);
}

TEST_CASE("T-PROC-020: getProcessStatistics reports turnaround and unknown pid") {
    Harness h;
    const int pid = h.create("a", 1, smallProgram(), 0);
    REQUIRE(h.pm.admit(pid));
    REQUIRE(h.pm.dispatch(pid));
    h.clock.tick();
    h.cpu.executeCycle(); // cycle 1
    REQUIRE(h.pm.terminate(pid)); // terminated at cycle 1

    const auto s = h.pm.getProcessStatistics(pid);
    REQUIRE(s.has_value());
    CHECK(s->pid == pid);
    CHECK(s->name == "a");
    CHECK(s->state == ProcessState::TERMINATED);
    CHECK(s->type == ProcessType::NORMAL);
    CHECK(s->cpuTime == 1);
    CHECK(s->turnaroundTime == 1);
    CHECK(s->contextSwitches == 1);
    CHECK(s->stateChanges == 3); // NEW -> READY -> RUNNING -> TERMINATED
    CHECK(s->ioRequests == 0);
    CHECK(s->ipcOperations == 0);
    CHECK_FALSE(h.pm.getProcessStatistics(999).has_value());
}

TEST_CASE("T-PROC-021: getAllProcessStatistics covers the PCB table") {
    Harness h;
    const int a = h.create("a", 1, smallProgram(), 0);
    const int b = h.pm.createAgentProcess("agent-b", 2, smallProgram(), 10);
    const auto all = h.pm.getAllProcessStatistics();
    REQUIRE(all.size() == 2);
    CHECK(all[0].pid == a);
    CHECK(all[0].type == ProcessType::NORMAL);
    CHECK(all[1].pid == b);
    CHECK(all[1].type == ProcessType::AI_AGENT);
}

TEST_CASE("T-PROC-022: cpuUtilization is cpu time over simulated time") {
    Harness h;
    const int a = h.create("a", 1, smallProgram(), 0);
    const int b = h.create("b", 2, smallProgram(), 10);
    REQUIRE(h.pm.admit(a));
    REQUIRE(h.pm.admit(b));
    REQUIRE(h.pm.dispatch(a)); // cycle 0
    h.clock.tick();
    h.cpu.executeCycle(); // cycle 1
    h.clock.tick();
    h.cpu.executeCycle(); // cycle 2
    REQUIRE(h.pm.dispatch(b)); // a charged 2; b runs from cycle 2
    h.clock.tick();
    h.cpu.executeCycle(); // cycle 3
    REQUIRE(h.pm.terminate(b)); // b charged 1

    // a: 2, b: 1, nothing running; 3 busy cycles out of 3.
    CHECK(h.pm.cpuUtilization() == 1.0);
}

TEST_CASE("T-PROC-023: cpuUtilization includes the current uncharged run") {
    Harness h;
    const int pid = h.create("a", 1, smallProgram(), 0);
    REQUIRE(h.pm.admit(pid));
    REQUIRE(h.pm.dispatch(pid)); // cycle 0
    h.clock.tick();
    h.cpu.executeCycle(); // cycle 1
    h.clock.tick();
    h.cpu.executeCycle(); // cycle 2

    // Charged cpuTime is still 0, but the process has been on the CPU since
    // cycle 0, so the snapshot counts the uncharged run: 2 / 2.
    CHECK(h.pm.getProcess(pid)->cpuTime == 0);
    CHECK(h.pm.cpuUtilization() == 1.0);
}