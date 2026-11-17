#include <aios/core/EventLog.hpp>
#include <aios/core/SimulationClock.hpp>
#include <aios/core/types.hpp>
#include <aios/cpu/CPU.hpp>
#include <aios/io/IOManager.hpp>
#include <aios/memory/Memory.hpp>
#include <aios/process/ProcessManager.hpp>

#include <doctest/doctest.h>

#include <string>

using namespace aios;

namespace {

// Minimal I/O wiring: the manager needs its clock/log for events and the
// process manager to wake owners of failed requests (docs/15 section 10).
struct IOHarness {
    Memory memory;
    EventLog log;
    SimulationClock clock;
    CPU cpu;
    ProcessManager pm;
    IOManager io;

    IOHarness() {
        pm.setMemory(&memory);
        pm.setEventLog(&log);
        pm.setCPU(&cpu);
        pm.setClock(&clock);
        cpu.setMemory(&memory);
        cpu.setEventLog(&log);
        cpu.setClock(&clock);
        io.setEventLog(&log);
        io.setProcessManager(&pm);
        io.setClock(&clock);
    }

    int createRunningProcess(const std::string& name) {
        const int pid = pm.createProcess(
            name, 0, {Instruction{Opcode::HALT, 0}.toWord()}, 0);
        pm.admit(pid);
        pm.dispatch(pid);
        return pid;
    }
};

} // namespace

TEST_CASE("T-IO-001: buffers track state transitions (docs/11 sections 18-25)") {
    IOHarness h;
    REQUIRE(h.io.createBuffer("BUF", 4));
    CHECK_FALSE(h.io.createBuffer("BUF", 4)); // duplicate id rejected

    auto buffer = h.io.getBuffer("BUF");
    REQUIRE(buffer.has_value());
    CHECK(buffer->state() == BufferState::EMPTY);

    CHECK(h.io.writeBuffer("BUF", "abc"));
    buffer = h.io.getBuffer("BUF");
    REQUIRE(buffer.has_value());
    CHECK(buffer->size() == 3);
    CHECK(buffer->state() == BufferState::PARTIAL);

    CHECK(h.io.readBuffer("BUF", 4) == "abc");
    buffer = h.io.getBuffer("BUF");
    REQUIRE(buffer.has_value());
    CHECK(buffer->state() == BufferState::EMPTY);
}

TEST_CASE("T-IO-002: device error fails queued requests and wakes their owners") {
    IOHarness h;
    REQUIRE(h.io.createDevice("DISK", DeviceType::DISK, 5));
    const int pid = h.createRunningProcess("p1");

    REQUIRE(h.io.requestIO(pid, "DISK", IOOperation::WRITE, "x") > 0);
    REQUIRE(h.pm.blockProcess(pid));
    CHECK(h.pm.getProcess(pid)->state == ProcessState::WAITING);

    CHECK(h.io.setDeviceError("DISK", "simulated failure"));
    auto device = h.io.getDevice("DISK");
    REQUIRE(device.has_value());
    CHECK(device->state() == DeviceState::ERROR);
    CHECK(device->queue().empty());
    // A stranded WAITING process is worse than a resumed one: the simulator
    // has no I/O error return channel, so the owner is woken.
    CHECK(h.pm.getProcess(pid)->state == ProcessState::READY);
    CHECK(h.io.getStatistics().failedRequests >= 1);

    // Requests are rejected while the device is in ERROR.
    CHECK(h.io.requestIO(pid, "DISK", IOOperation::WRITE, "y") == INVALID_PID);

    CHECK(h.io.recoverDevice("DISK"));
    device = h.io.getDevice("DISK");
    REQUIRE(device.has_value());
    CHECK(device->state() == DeviceState::AVAILABLE);
    CHECK(h.io.requestIO(pid, "DISK", IOOperation::WRITE, "z") > 0);
}

TEST_CASE("T-IO-003: unknown device ids are rejected by error/recover") {
    IOHarness h;
    REQUIRE(h.io.createDevice("PRINTER", DeviceType::PRINTER, 8));

    CHECK_FALSE(h.io.setDeviceError("NOPE", "missing"));
    CHECK_FALSE(h.io.recoverDevice("NOPE"));
    CHECK(h.io.listDevices().size() == 1);
    CHECK(h.io.getAllDevices().size() == 1);
}

TEST_CASE("T-IO-004: queued requests start automatically after each completion") {
    IOHarness h;
    REQUIRE(h.io.createDevice("DISK2", DeviceType::DISK, 2));
    // Each process is dispatched just before its request so it is RUNNING
    // when blockProcess runs (dispatching one would preempt the previous).
    // First request starts immediately; the rest queue behind it.
    const int p1 = h.createRunningProcess("p1");
    REQUIRE(h.io.requestIO(p1, "DISK2", IOOperation::WRITE, "a") > 0);
    REQUIRE(h.pm.blockProcess(p1));
    const int p2 = h.createRunningProcess("p2");
    REQUIRE(h.io.requestIO(p2, "DISK2", IOOperation::WRITE, "b") > 0);
    REQUIRE(h.pm.blockProcess(p2));
    const int p3 = h.createRunningProcess("p3");
    REQUIRE(h.io.requestIO(p3, "DISK2", IOOperation::WRITE, "c") > 0);
    REQUIRE(h.pm.blockProcess(p3));
    auto device = h.io.getDevice("DISK2");
    REQUIRE(device.has_value());
    CHECK(device->queue().size() == 2);

    // Tick enough cycles for all three 2-cycle requests to drain (FIFO).
    for (int cycle = 0; cycle < 20; ++cycle) {
        h.clock.tick();
        h.io.tick();
    }
    device = h.io.getDevice("DISK2");
    REQUIRE(device.has_value());
    CHECK(device->queue().empty());
    CHECK(device->state() == DeviceState::AVAILABLE);
    CHECK(h.io.getStatistics().completedRequests == 3);
    // Every owner was woken, none are stranded in WAITING.
    CHECK(h.pm.getProcess(p1)->state == ProcessState::READY);
    CHECK(h.pm.getProcess(p2)->state == ProcessState::READY);
    CHECK(h.pm.getProcess(p3)->state == ProcessState::READY);
}
