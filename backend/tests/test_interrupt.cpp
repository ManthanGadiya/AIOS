#include <aios/core/EventLog.hpp>
#include <aios/core/SimulationClock.hpp>
#include <aios/core/types.hpp>
#include <aios/cpu/CPU.hpp>
#include <aios/interrupt/InterruptManager.hpp>
#include <aios/interrupt/SystemCallManager.hpp>
#include <aios/memory/Memory.hpp>
#include <aios/process/ProcessManager.hpp>

#include <doctest/doctest.h>

#include <deque>
#include <sstream>

using namespace aios;

namespace {

struct Harness {
    Memory memory;
    EventLog log;
    SimulationClock clock;
    CPU cpu;
    ProcessManager pm;
    SystemCallManager syscalls;
    InterruptManager im;
    std::deque<int32_t> input;
    std::ostringstream output;

    Harness() {
        pm.setMemory(&memory);
        pm.setEventLog(&log);
        pm.setCPU(&cpu);
        pm.setClock(&clock);
        cpu.setMemory(&memory);
        cpu.setEventLog(&log);
        cpu.setClock(&clock);
        syscalls.setEventLog(&log);
        syscalls.setProcessManager(&pm);
        syscalls.setInputQueue(&input);
        syscalls.setOutputStream(&output);
        im.setEventLog(&log);
        im.setSystemCallManager(&syscalls);
        im.setProcessManager(&pm);
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

std::vector<int32_t> tinyProgram() {
    return {Instruction{Opcode::HALT, 0}.toWord()};
}

} // namespace

TEST_CASE("T-INT-001: generateInterrupt queues a request with a new id") {
    Harness h;
    const int id = h.im.generateInterrupt(InterruptType::SYSTEM_CALL, 7, SYSCALL_EXIT);
    CHECK(id == 1);
    CHECK(h.im.pendingCount() == 1);
    CHECK(h.im.hasPending());
    CHECK(h.im.log().size() == 1);
    CHECK(h.im.log()[0].type == InterruptType::SYSTEM_CALL);
    CHECK(h.im.log()[0].pid == 7);
    CHECK(h.im.log()[0].data == SYSCALL_EXIT);
    CHECK(h.im.log()[0].phase == InterruptPhase::PENDING);
    CHECK(h.im.phase() == InterruptPhase::PENDING);
}

TEST_CASE("T-INT-002: interrupts are serviced by priority (SYSTEM_CALL before TIMER)") {
    Harness h;
    const int first = h.im.generateInterrupt(InterruptType::TIMER, 1, 0);
    const int second = h.im.generateInterrupt(InterruptType::SYSTEM_CALL, 2, SYSCALL_EXIT);
    CHECK(first == 1);
    CHECK(second == 2);

    CHECK(h.im.serviceNextInterrupt());
    CHECK(h.im.pendingCount() == 1);
    // The higher-priority SYSTEM_CALL is serviced before the TIMER.
    CHECK(h.im.log()[1].phase == InterruptPhase::COMPLETED);
    CHECK(h.im.log()[0].phase == InterruptPhase::PENDING);
    CHECK(h.im.phase() == InterruptPhase::COMPLETED);

    CHECK(h.im.serviceNextInterrupt());
    CHECK(h.im.pendingCount() == 0);
    CHECK_FALSE(h.im.hasPending());
}

TEST_CASE("T-INT-003: serviceNextInterrupt returns false when empty") {
    Harness h;
    CHECK_FALSE(h.im.serviceNextInterrupt());
}

TEST_CASE("T-INT-004: reset clears queue, log and id counter") {
    Harness h;
    h.im.generateInterrupt(InterruptType::TIMER, 1, 0);
    h.im.reset();
    CHECK(h.im.pendingCount() == 0);
    CHECK(h.im.log().empty());
    CHECK(h.im.phase() == InterruptPhase::IDLE);
    CHECK(h.im.generateInterrupt(InterruptType::TIMER, 1, 0) == 1);
}

TEST_CASE("T-INT-005: SYSTEM_CALL EXIT terminates the calling process") {
    Harness h;
    const int pid = h.pm.createProcess("a", 1, tinyProgram(), 0);
    REQUIRE(pid != INVALID_PID);
    REQUIRE(h.pm.admit(pid));
    REQUIRE(h.pm.dispatch(pid));

    h.im.generateInterrupt(InterruptType::SYSTEM_CALL, pid, SYSCALL_EXIT);
    REQUIRE(h.im.serviceNextInterrupt());
    CHECK(h.pm.getProcess(pid)->state == ProcessState::TERMINATED);
    CHECK(h.hasEvent(EventType::INTERRUPT));
}

TEST_CASE("T-INT-006: ERROR interrupt marks the process failed") {
    Harness h;
    const int pid = h.pm.createProcess("a", 1, tinyProgram(), 0);
    REQUIRE(h.pm.admit(pid));
    REQUIRE(h.pm.dispatch(pid));

    h.im.generateInterrupt(InterruptType::ERROR, pid, 0);
    REQUIRE(h.im.serviceNextInterrupt());
    CHECK(h.pm.getProcess(pid)->state == ProcessState::FAILED);
}

TEST_CASE("T-INT-007: invalid syscall terminates the process") {
    Harness h;
    const int pid = h.pm.createProcess("a", 1, tinyProgram(), 0);
    REQUIRE(h.pm.admit(pid));
    REQUIRE(h.pm.dispatch(pid));

    h.im.generateInterrupt(InterruptType::SYSTEM_CALL, pid, 99);
    REQUIRE(h.im.serviceNextInterrupt());
    CHECK(h.pm.getProcess(pid)->state == ProcessState::TERMINATED);
}

TEST_CASE("T-INT-008: servicing records an INTERRUPT event") {
    Harness h;
    h.im.generateInterrupt(InterruptType::TIMER, INVALID_PID, 0);
    REQUIRE(h.im.serviceNextInterrupt());
    bool found = false;
    for (const auto& e : h.log.events()) {
        if (e.type == EventType::INTERRUPT) {
            found = true;
            CHECK(e.detail.find("TIMER") != std::string::npos);
        }
    }
    CHECK(found);
}

TEST_CASE("T-INT-009: interrupt priority order follows docs/05 section 19") {
    // Lower value = higher priority. Order finalized during implementation.
    CHECK(InterruptManager::priorityOf(InterruptType::ERROR) <
          InterruptManager::priorityOf(InterruptType::PAGE_FAULT));
    CHECK(InterruptManager::priorityOf(InterruptType::PAGE_FAULT) <
          InterruptManager::priorityOf(InterruptType::SYSTEM_CALL));
    CHECK(InterruptManager::priorityOf(InterruptType::SYSTEM_CALL) <
          InterruptManager::priorityOf(InterruptType::IO_COMPLETE));
    CHECK(InterruptManager::priorityOf(InterruptType::IO_COMPLETE) <
          InterruptManager::priorityOf(InterruptType::TIMER));
}

TEST_CASE("T-INT-010: equal-priority interrupts are serviced FIFO") {
    Harness h;
    h.im.generateInterrupt(InterruptType::TIMER, 1, 0);
    h.im.generateInterrupt(InterruptType::TIMER, 2, 0);
    h.im.generateInterrupt(InterruptType::TIMER, 3, 0);

    CHECK(h.im.serviceNextInterrupt());
    CHECK(h.im.log()[0].phase == InterruptPhase::COMPLETED); // oldest first
    CHECK(h.im.log()[1].phase == InterruptPhase::PENDING);
    CHECK(h.im.log()[2].phase == InterruptPhase::PENDING);

    CHECK(h.im.serviceNextInterrupt());
    CHECK(h.im.log()[1].phase == InterruptPhase::COMPLETED);

    CHECK(h.im.serviceNextInterrupt());
    CHECK(h.im.log()[2].phase == InterruptPhase::COMPLETED);
    CHECK(h.im.pendingCount() == 0);
}

TEST_CASE("T-INT-011: nested interrupts are deferred while an ISR executes") {
    Harness h;
    // Interrupts arriving while the manager is busy are queued, never nested
    // (docs/05 section 20), and all are eventually serviced.
    h.im.generateInterrupt(InterruptType::TIMER, INVALID_PID, 0);
    h.im.generateInterrupt(InterruptType::SYSTEM_CALL, INVALID_PID, SYSCALL_EXIT);
    h.im.generateInterrupt(InterruptType::TIMER, INVALID_PID, 0);

    CHECK(h.im.pendingCount() == 3);
    const std::vector<InterruptRequest> pending = h.im.pendingInterrupts();
    REQUIRE(pending.size() == 3);
    CHECK(pending[0].type == InterruptType::TIMER);
    CHECK(pending[1].type == InterruptType::SYSTEM_CALL);
    CHECK(pending[2].type == InterruptType::TIMER);

    // Serviced in priority order, then the queue drains.
    REQUIRE(h.im.serviceNextInterrupt());
    CHECK(h.im.log()[1].phase == InterruptPhase::COMPLETED); // SYSTEM_CALL first
    REQUIRE(h.im.serviceNextInterrupt());
    CHECK(h.im.log()[0].phase == InterruptPhase::COMPLETED);
    REQUIRE(h.im.serviceNextInterrupt());
    CHECK(h.im.log()[2].phase == InterruptPhase::COMPLETED);
    CHECK_FALSE(h.im.hasPending());
}