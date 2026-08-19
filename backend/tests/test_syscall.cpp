#include <aios/core/EventLog.hpp>
#include <aios/core/SimulationClock.hpp>
#include <aios/core/types.hpp>
#include <aios/cpu/CPU.hpp>
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
    }

    int startRunningProcess() {
        const int pid =
            pm.createProcess("a", 1, {Instruction{Opcode::HALT, 0}.toWord()}, 0);
        REQUIRE(pid != INVALID_PID);
        REQUIRE(pm.admit(pid));
        REQUIRE(pm.dispatch(pid));
        return pid;
    }
};

} // namespace

TEST_CASE("T-SC-001: READ pops the next input value") {
    Harness h;
    h.input.push_back(42);
    h.input.push_back(-7);
    const SyscallResult r = h.syscalls.handleSystemCall(SyscallRequest{1, SYSCALL_READ});
    CHECK(r.status == SyscallStatus::OK);
    CHECK(r.returnValue == 42);
    CHECK(h.input.size() == 1);
    CHECK(h.syscalls.handleSystemCall(SyscallRequest{1, SYSCALL_READ}).returnValue == -7);
}

TEST_CASE("T-SC-002: READ on an empty queue returns ERROR") {
    Harness h;
    const SyscallResult r = h.syscalls.handleSystemCall(SyscallRequest{1, SYSCALL_READ});
    CHECK(r.status == SyscallStatus::ERROR);
}

TEST_CASE("T-SC-003: WRITE emits PID <pid> WRITE") {
    Harness h;
    const SyscallResult r = h.syscalls.handleSystemCall(SyscallRequest{3, SYSCALL_WRITE});
    CHECK(r.status == SyscallStatus::OK);
    CHECK(h.output.str() == "PID 3 WRITE\n");
}

TEST_CASE("T-SC-004: ALLOCATE is a stub returning OK") {
    Harness h;
    const SyscallResult r = h.syscalls.handleSystemCall(SyscallRequest{1, SYSCALL_ALLOCATE});
    CHECK(r.status == SyscallStatus::OK);
    CHECK(r.returnValue == 0);
}

TEST_CASE("T-SC-005: EXIT terminates the calling process") {
    Harness h;
    const int pid = h.startRunningProcess();
    const SyscallResult r = h.syscalls.handleSystemCall(SyscallRequest{pid, SYSCALL_EXIT});
    CHECK(r.status == SyscallStatus::OK);
    CHECK(h.pm.getProcess(pid)->state == ProcessState::TERMINATED);
}

TEST_CASE("T-SC-006: IPC_SEND and IO_REQUEST are recognized but unsupported") {
    Harness h;
    CHECK(h.syscalls.handleSystemCall(SyscallRequest{1, SYSCALL_IPC_SEND}).status ==
          SyscallStatus::UNSUPPORTED);
    CHECK(h.syscalls.handleSystemCall(SyscallRequest{1, SYSCALL_IO_REQUEST}).status ==
          SyscallStatus::UNSUPPORTED);
}

TEST_CASE("T-SC-007: unknown syscall numbers return INVALID") {
    Harness h;
    const SyscallResult r = h.syscalls.handleSystemCall(SyscallRequest{1, 99});
    CHECK(r.status == SyscallStatus::INVALID);
}