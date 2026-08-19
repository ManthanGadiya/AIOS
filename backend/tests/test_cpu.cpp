#include <aios/core/EventLog.hpp>
#include <aios/core/SimulationClock.hpp>
#include <aios/core/types.hpp>
#include <aios/cpu/CPU.hpp>
#include <aios/interrupt/InterruptManager.hpp>
#include <aios/memory/Memory.hpp>

#include <doctest/doctest.h>

#include <deque>
#include <sstream>
#include <vector>

using namespace aios;

namespace {

void wire(CPU& cpu, Memory& mem, EventLog& log, SimulationClock& clock,
          InterruptManager& im, std::deque<int32_t>& input, std::ostringstream& out) {
    cpu.setMemory(&mem);
    cpu.setEventLog(&log);
    cpu.setClock(&clock);
    cpu.setInterruptManager(&im);
    cpu.setInputQueue(&input);
    cpu.setOutputStream(&out);
}

bool hasEvent(const EventLog& log, EventType type) {
    for (const auto& e : log.events()) {
        if (e.type == type) {
            return true;
        }
    }
    return false;
}

} // namespace

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------
TEST_CASE("T-CPU-001: initialize sets PC, clears registers, stays IDLE") {
    CPU cpu;
    cpu.initialize(100);
    CHECK(cpu.state() == CpuState::IDLE);
    CHECK(cpu.currentProcess() == INVALID_PID);
    CHECK(cpu.registers().pc == 100);
    CHECK(cpu.registers().acc == 0);
    CHECK(cpu.registers().ir.isEmpty());
    CHECK_FALSE(cpu.isHalted());
}

TEST_CASE("T-CPU-002: loadContext marks RUNNING and saveContext snapshots registers") {
    CPU cpu;
    CpuRegisters regs;
    regs.pc = 42;
    regs.acc = 7;
    regs.ir = Instruction{Opcode::ADD, 3};
    cpu.loadContext(regs, 5);

    CHECK(cpu.state() == CpuState::RUNNING);
    CHECK(cpu.currentProcess() == 5);
    CHECK(cpu.registers().pc == 42);
    CHECK(cpu.registers().acc == 7);

    const CpuRegisters saved = cpu.saveContext();
    CHECK(saved.pc == 42);
    CHECK(saved.acc == 7);
    CHECK(saved.ir.opcode == Opcode::ADD);
}

TEST_CASE("T-CPU-003: reset returns to IDLE with cleared registers") {
    CPU cpu;
    cpu.loadContext(CpuRegisters{}, 3);
    cpu.reset();
    CHECK(cpu.state() == CpuState::IDLE);
    CHECK(cpu.currentProcess() == INVALID_PID);
    CHECK(cpu.registers().ir.isEmpty());
}

TEST_CASE("T-CPU-004: executeCycle is a no-op while IDLE") {
    CPU cpu;
    Memory mem;
    EventLog log;
    SimulationClock clock;
    InterruptManager im;
    std::deque<int32_t> input;
    std::ostringstream out;
    wire(cpu, mem, log, clock, im, input, out);

    cpu.initialize(0);
    cpu.executeCycle();
    CHECK(cpu.state() == CpuState::IDLE);
    CHECK(cpu.registers().pc == 0);
    CHECK(log.size() == 0);
}

// ---------------------------------------------------------------------------
// Data movement
// ---------------------------------------------------------------------------
TEST_CASE("T-CPU-005: LOAD copies memory into ACC and sets flags") {
    CPU cpu;
    Memory mem;
    EventLog log;
    SimulationClock clock;
    InterruptManager im;
    std::deque<int32_t> input;
    std::ostringstream out;
    wire(cpu, mem, log, clock, im, input, out);

    mem.write(10, -5);
    mem.write(0, Instruction{Opcode::LOAD, 10}.toWord());
    cpu.loadContext(CpuRegisters{}, 1);
    cpu.executeCycle();

    CHECK(cpu.registers().acc == -5);
    CHECK(cpu.registers().flags.negative);
    CHECK_FALSE(cpu.registers().flags.zero);
    CHECK(cpu.registers().pc == 1);
    CHECK(hasEvent(log, EventType::INSTRUCTION_FETCH));
    CHECK(hasEvent(log, EventType::INSTRUCTION_EXECUTE));
}

TEST_CASE("T-CPU-006: LOAD sets the zero flag for a zero word") {
    CPU cpu;
    Memory mem;
    EventLog log;
    SimulationClock clock;
    InterruptManager im;
    std::deque<int32_t> input;
    std::ostringstream out;
    wire(cpu, mem, log, clock, im, input, out);

    mem.write(10, 0);
    mem.write(0, Instruction{Opcode::LOAD, 10}.toWord());
    cpu.loadContext(CpuRegisters{}, 1);
    cpu.executeCycle();

    CHECK(cpu.registers().acc == 0);
    CHECK(cpu.registers().flags.zero);
    CHECK_FALSE(cpu.registers().flags.negative);
}

TEST_CASE("T-CPU-007: STORE copies ACC into memory") {
    CPU cpu;
    Memory mem;
    EventLog log;
    SimulationClock clock;
    InterruptManager im;
    std::deque<int32_t> input;
    std::ostringstream out;
    wire(cpu, mem, log, clock, im, input, out);

    mem.write(0, Instruction{Opcode::STORE, 10}.toWord());
    CpuRegisters regs;
    regs.acc = 99;
    cpu.loadContext(regs, 1);
    cpu.executeCycle();

    CHECK(mem.read(10).value() == 99);
    CHECK(cpu.registers().acc == 99);
}

// ---------------------------------------------------------------------------
// Arithmetic
// ---------------------------------------------------------------------------
TEST_CASE("T-CPU-008: ADD adds an immediate and updates flags") {
    CPU cpu;
    Memory mem;
    EventLog log;
    SimulationClock clock;
    InterruptManager im;
    std::deque<int32_t> input;
    std::ostringstream out;
    wire(cpu, mem, log, clock, im, input, out);

    mem.write(0, Instruction{Opcode::ADD, 3}.toWord());
    CpuRegisters regs;
    regs.acc = 5;
    cpu.loadContext(regs, 1);
    cpu.executeCycle();
    CHECK(cpu.registers().acc == 8);

    // 8 + (-8) == 0 -> zero flag (PC has advanced to 1)
    mem.write(1, Instruction{Opcode::ADD, -8}.toWord());
    cpu.executeCycle();
    CHECK(cpu.registers().acc == 0);
    CHECK(cpu.registers().flags.zero);
}

TEST_CASE("T-CPU-009: SUB subtracts an immediate") {
    CPU cpu;
    Memory mem;
    EventLog log;
    SimulationClock clock;
    InterruptManager im;
    std::deque<int32_t> input;
    std::ostringstream out;
    wire(cpu, mem, log, clock, im, input, out);

    mem.write(0, Instruction{Opcode::SUB, 4}.toWord());
    CpuRegisters regs;
    regs.acc = 10;
    cpu.loadContext(regs, 1);
    cpu.executeCycle();
    CHECK(cpu.registers().acc == 6);
    CHECK(cpu.registers().flags.negative == false);
}

TEST_CASE("T-CPU-010: MUL multiplies an immediate") {
    CPU cpu;
    Memory mem;
    EventLog log;
    SimulationClock clock;
    InterruptManager im;
    std::deque<int32_t> input;
    std::ostringstream out;
    wire(cpu, mem, log, clock, im, input, out);

    mem.write(0, Instruction{Opcode::MUL, 7}.toWord());
    CpuRegisters regs;
    regs.acc = 6;
    cpu.loadContext(regs, 1);
    cpu.executeCycle();
    CHECK(cpu.registers().acc == 42);
}

TEST_CASE("T-CPU-011: DIV divides an immediate") {
    CPU cpu;
    Memory mem;
    EventLog log;
    SimulationClock clock;
    InterruptManager im;
    std::deque<int32_t> input;
    std::ostringstream out;
    wire(cpu, mem, log, clock, im, input, out);

    mem.write(0, Instruction{Opcode::DIV, 4}.toWord());
    CpuRegisters regs;
    regs.acc = 20;
    cpu.loadContext(regs, 1);
    cpu.executeCycle();
    CHECK(cpu.registers().acc == 5);
}

TEST_CASE("T-CPU-012: arithmetic wraparound sets overflow and carry") {
    CPU cpu;
    Memory mem;
    EventLog log;
    SimulationClock clock;
    InterruptManager im;
    std::deque<int32_t> input;
    std::ostringstream out;
    wire(cpu, mem, log, clock, im, input, out);

    mem.write(0, Instruction{Opcode::MUL, 32767}.toWord());
    CpuRegisters regs;
    regs.acc = 100000;
    cpu.loadContext(regs, 1);
    cpu.executeCycle();

    // 100000 * 32767 = 3,276,700,000 wraps to -1,018,267,296 in int32
    // (32767 is the largest immediate that fits the 16-bit operand field).
    CHECK(cpu.registers().acc == -1018267296);
    CHECK(cpu.registers().flags.overflow);
    CHECK(cpu.registers().flags.carry);
    CHECK_FALSE(cpu.registers().flags.zero);
    CHECK_FALSE(cpu.isHalted());
}

TEST_CASE("T-CPU-013: DIV by zero raises CPU error and halts") {
    CPU cpu;
    Memory mem;
    EventLog log;
    SimulationClock clock;
    InterruptManager im;
    std::deque<int32_t> input;
    std::ostringstream out;
    wire(cpu, mem, log, clock, im, input, out);

    mem.write(0, Instruction{Opcode::DIV, 0}.toWord());
    CpuRegisters regs;
    regs.acc = 5;
    cpu.loadContext(regs, 1);
    cpu.executeCycle();

    CHECK(cpu.isHalted());
    CHECK(cpu.registers().flags.error);
    CHECK(hasEvent(log, EventType::CPU_ERROR));
}

// ---------------------------------------------------------------------------
// Control flow
// ---------------------------------------------------------------------------
TEST_CASE("T-CPU-014: JMP redirects the program counter") {
    CPU cpu;
    Memory mem;
    EventLog log;
    SimulationClock clock;
    InterruptManager im;
    std::deque<int32_t> input;
    std::ostringstream out;
    wire(cpu, mem, log, clock, im, input, out);

    // 0: JMP 3 ; 1: ADD 99 ; 3: ADD 5
    mem.write(0, Instruction{Opcode::JMP, 3}.toWord());
    mem.write(1, Instruction{Opcode::ADD, 99}.toWord());
    mem.write(3, Instruction{Opcode::ADD, 5}.toWord());
    cpu.loadContext(CpuRegisters{}, 1);
    cpu.executeCycle();
    CHECK(cpu.registers().pc == 3);
    cpu.executeCycle();
    CHECK(cpu.registers().acc == 5); // skipped the ADD 99 at address 1
}

TEST_CASE("T-CPU-015: JZ jumps when the zero flag is set") {
    CPU cpu;
    Memory mem;
    EventLog log;
    SimulationClock clock;
    InterruptManager im;
    std::deque<int32_t> input;
    std::ostringstream out;
    wire(cpu, mem, log, clock, im, input, out);

    // 0: ADD 0 ; 1: JZ 3 ; 2: ADD 99 ; 3: ADD 5
    mem.write(0, Instruction{Opcode::ADD, 0}.toWord());
    mem.write(1, Instruction{Opcode::JZ, 3}.toWord());
    mem.write(2, Instruction{Opcode::ADD, 99}.toWord());
    mem.write(3, Instruction{Opcode::ADD, 5}.toWord());
    cpu.loadContext(CpuRegisters{}, 1);
    cpu.executeCycle(); // ADD 0 -> zero flag
    CHECK(cpu.registers().flags.zero);
    cpu.executeCycle(); // JZ 3 -> pc = 3
    CHECK(cpu.registers().pc == 3);
    cpu.executeCycle(); // ADD 5
    CHECK(cpu.registers().acc == 5);
}

TEST_CASE("T-CPU-016: JZ falls through when the zero flag is clear") {
    CPU cpu;
    Memory mem;
    EventLog log;
    SimulationClock clock;
    InterruptManager im;
    std::deque<int32_t> input;
    std::ostringstream out;
    wire(cpu, mem, log, clock, im, input, out);

    // 0: ADD 1 ; 1: JZ 3 ; 2: ADD 99
    mem.write(0, Instruction{Opcode::ADD, 1}.toWord());
    mem.write(1, Instruction{Opcode::JZ, 3}.toWord());
    mem.write(2, Instruction{Opcode::ADD, 99}.toWord());
    cpu.loadContext(CpuRegisters{}, 1);
    cpu.executeCycle(); // ACC = 1, zero clear
    cpu.executeCycle(); // JZ falls through
    CHECK(cpu.registers().pc == 2);
    cpu.executeCycle(); // ADD 99
    CHECK(cpu.registers().acc == 100);
}

// ---------------------------------------------------------------------------
// Faults
// ---------------------------------------------------------------------------
TEST_CASE("T-CPU-017: fetch out of bounds raises CPU error") {
    CPU cpu;
    Memory mem;
    EventLog log;
    SimulationClock clock;
    InterruptManager im;
    std::deque<int32_t> input;
    std::ostringstream out;
    wire(cpu, mem, log, clock, im, input, out);

    CpuRegisters regs;
    regs.pc = 1024;
    cpu.loadContext(regs, 1);
    cpu.executeCycle();
    CHECK(cpu.isHalted());
    CHECK(cpu.registers().flags.error);
    CHECK(hasEvent(log, EventType::CPU_ERROR));
}

TEST_CASE("T-CPU-018: OOB LOAD and STORE raise CPU errors") {
    CPU cpu;
    Memory mem;
    EventLog log;
    SimulationClock clock;
    InterruptManager im;
    std::deque<int32_t> input;
    std::ostringstream out;
    wire(cpu, mem, log, clock, im, input, out);

    mem.write(0, Instruction{Opcode::LOAD, 2000}.toWord());
    cpu.loadContext(CpuRegisters{}, 1);
    cpu.executeCycle();
    CHECK(cpu.isHalted());
    CHECK(cpu.registers().flags.error);
    CHECK(hasEvent(log, EventType::CPU_ERROR));

    cpu.reset();
    log.clear();
    mem.write(0, Instruction{Opcode::STORE, 2000}.toWord());
    cpu.loadContext(CpuRegisters{}, 1);
    cpu.executeCycle();
    CHECK(cpu.isHalted());
    CHECK(hasEvent(log, EventType::CPU_ERROR));
}

TEST_CASE("T-CPU-019: invalid opcode word raises CPU error") {
    CPU cpu;
    Memory mem;
    EventLog log;
    SimulationClock clock;
    InterruptManager im;
    std::deque<int32_t> input;
    std::ostringstream out;
    wire(cpu, mem, log, clock, im, input, out);

    mem.write(0, 20 << 16); // reserved opcode id
    cpu.loadContext(CpuRegisters{}, 1);
    cpu.executeCycle();
    CHECK(cpu.isHalted());
    CHECK(cpu.registers().flags.error);
    CHECK(hasEvent(log, EventType::CPU_ERROR));
}

// ---------------------------------------------------------------------------
// I/O instructions
// ---------------------------------------------------------------------------
TEST_CASE("T-CPU-020: READ pops the input queue into memory") {
    CPU cpu;
    Memory mem;
    EventLog log;
    SimulationClock clock;
    InterruptManager im;
    std::deque<int32_t> input{7, 8};
    std::ostringstream out;
    wire(cpu, mem, log, clock, im, input, out);

    mem.write(0, Instruction{Opcode::READ, 10}.toWord());
    cpu.loadContext(CpuRegisters{}, 1);
    cpu.executeCycle();
    CHECK(mem.read(10).value() == 7);
    CHECK(input.size() == 1);
}

TEST_CASE("T-CPU-021: READ with an empty input queue raises CPU error") {
    CPU cpu;
    Memory mem;
    EventLog log;
    SimulationClock clock;
    InterruptManager im;
    std::deque<int32_t> input;
    std::ostringstream out;
    wire(cpu, mem, log, clock, im, input, out);

    mem.write(0, Instruction{Opcode::READ, 10}.toWord());
    cpu.loadContext(CpuRegisters{}, 1);
    cpu.executeCycle();
    CHECK(cpu.isHalted());
    CHECK(cpu.registers().flags.error);
    CHECK(hasEvent(log, EventType::CPU_ERROR));
}

TEST_CASE("T-CPU-022: WRITE emits the memory word and a newline") {
    CPU cpu;
    Memory mem;
    EventLog log;
    SimulationClock clock;
    InterruptManager im;
    std::deque<int32_t> input;
    std::ostringstream out;
    wire(cpu, mem, log, clock, im, input, out);

    mem.write(10, 13);
    mem.write(0, Instruction{Opcode::WRITE, 10}.toWord());
    cpu.loadContext(CpuRegisters{}, 1);
    cpu.executeCycle();
    CHECK(out.str() == "13\n");
}

// ---------------------------------------------------------------------------
// Traps
// ---------------------------------------------------------------------------
TEST_CASE("T-CPU-023: SYSCALL generates a SYSTEM_CALL interrupt and pauses the CPU") {
    CPU cpu;
    Memory mem;
    EventLog log;
    SimulationClock clock;
    InterruptManager im;
    std::deque<int32_t> input;
    std::ostringstream out;
    wire(cpu, mem, log, clock, im, input, out);

    mem.write(0, Instruction{Opcode::SYSCALL, 2}.toWord());
    cpu.loadContext(CpuRegisters{}, 1);
    cpu.executeCycle();

    CHECK(cpu.state() == CpuState::INTERRUPTED);
    CHECK(im.pendingCount() == 1);
    REQUIRE(im.log().size() == 1);
    CHECK(im.log()[0].type == InterruptType::SYSTEM_CALL);
    CHECK(im.log()[0].pid == 1);
    CHECK(im.log()[0].data == 2);
    CHECK(hasEvent(log, EventType::SYSCALL));
}

TEST_CASE("T-CPU-024: HALT records the HALT event and halts") {
    CPU cpu;
    Memory mem;
    EventLog log;
    SimulationClock clock;
    InterruptManager im;
    std::deque<int32_t> input;
    std::ostringstream out;
    wire(cpu, mem, log, clock, im, input, out);

    mem.write(0, Instruction{Opcode::HALT, 0}.toWord());
    cpu.loadContext(CpuRegisters{}, 1);
    cpu.executeCycle();
    CHECK(cpu.isHalted());
    CHECK(cpu.state() == CpuState::HALTED);
    CHECK(hasEvent(log, EventType::HALT));
}

TEST_CASE("T-CPU-025: resume only returns an INTERRUPTED CPU to RUNNING") {
    CPU cpu;
    Memory mem;
    EventLog log;
    SimulationClock clock;
    InterruptManager im;
    std::deque<int32_t> input;
    std::ostringstream out;
    wire(cpu, mem, log, clock, im, input, out);

    mem.write(0, Instruction{Opcode::SYSCALL, 1}.toWord());
    cpu.loadContext(CpuRegisters{}, 1);

    cpu.resume(); // no-op while RUNNING
    CHECK(cpu.state() == CpuState::RUNNING);

    cpu.executeCycle(); // SYSCALL -> INTERRUPTED
    CHECK(cpu.state() == CpuState::INTERRUPTED);

    cpu.resume();
    CHECK(cpu.state() == CpuState::RUNNING);
}

// ---------------------------------------------------------------------------
// Whole programs
// ---------------------------------------------------------------------------
TEST_CASE("T-CPU-026: full program LOAD/ADD/STORE/WRITE/HALT") {
    CPU cpu;
    Memory mem;
    EventLog log;
    SimulationClock clock;
    InterruptManager im;
    std::deque<int32_t> input;
    std::ostringstream out;
    wire(cpu, mem, log, clock, im, input, out);

    // 0: LOAD 5 ; 1: ADD 3 ; 2: STORE 10 ; 3: WRITE 10 ; 4: HALT
    const std::vector<int32_t> program = {
        Instruction{Opcode::LOAD, 5}.toWord(),
        Instruction{Opcode::ADD, 3}.toWord(),
        Instruction{Opcode::STORE, 10}.toWord(),
        Instruction{Opcode::WRITE, 10}.toWord(),
        Instruction{Opcode::HALT, 0}.toWord(),
    };
    REQUIRE(mem.loadProgram(program, 0));
    mem.write(5, 10);

    cpu.loadContext(CpuRegisters{}, 1);
    for (int i = 0; i < 5; ++i) {
        cpu.executeCycle();
    }

    CHECK(cpu.isHalted());
    CHECK(cpu.registers().acc == 13);
    CHECK(mem.read(10).value() == 13);
    CHECK(out.str() == "13\n");
    CHECK(hasEvent(log, EventType::HALT));
}