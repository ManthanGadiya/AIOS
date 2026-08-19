#include <aios/core/types.hpp>

#include <doctest/doctest.h>

using namespace aios;

// ---------------------------------------------------------------------------
// T-ENC: word encoding / decoding (decision D2)
// ---------------------------------------------------------------------------
TEST_CASE("T-ENC-001: packed word encoding round trip") {
    CHECK(Instruction{Opcode::LOAD, 10}.toWord() == 10);
    CHECK(Instruction{Opcode::ADD, 3}.toWord() == ((2 << 16) | 3));
    CHECK(Instruction{Opcode::HALT, 0}.toWord() == (11 << 16));
}

TEST_CASE("T-ENC-002: negative operands survive encoding via sign extension") {
    const int32_t word = Instruction{Opcode::ADD, -5}.toWord();
    const Instruction decoded = Instruction::fromWord(word);
    CHECK(decoded.opcode == Opcode::ADD);
    CHECK(decoded.operand == -5);
}

TEST_CASE("T-ENC-003: reserved opcode ids decode to INVALID") {
    const Instruction decoded = Instruction::fromWord(31 << 16);
    CHECK(decoded.opcode == Opcode::INVALID);
}

TEST_CASE("T-ENC-004: default IR is the empty IR") {
    const Instruction empty;
    CHECK(empty.isEmpty());
}

// ---------------------------------------------------------------------------
// T-STR: string conversions
// ---------------------------------------------------------------------------
TEST_CASE("T-STR-001: opcodeToString covers all opcodes") {
    CHECK(opcodeToString(Opcode::LOAD) == "LOAD");
    CHECK(opcodeToString(Opcode::STORE) == "STORE");
    CHECK(opcodeToString(Opcode::ADD) == "ADD");
    CHECK(opcodeToString(Opcode::SUB) == "SUB");
    CHECK(opcodeToString(Opcode::MUL) == "MUL");
    CHECK(opcodeToString(Opcode::DIV) == "DIV");
    CHECK(opcodeToString(Opcode::JMP) == "JMP");
    CHECK(opcodeToString(Opcode::JZ) == "JZ");
    CHECK(opcodeToString(Opcode::READ) == "READ");
    CHECK(opcodeToString(Opcode::WRITE) == "WRITE");
    CHECK(opcodeToString(Opcode::SYSCALL) == "SYSCALL");
    CHECK(opcodeToString(Opcode::HALT) == "HALT");
    CHECK(opcodeToString(Opcode::INVALID) == "INVALID");
}

TEST_CASE("T-STR-002: Instruction::toString formats mnemonics and operands") {
    CHECK(Instruction{Opcode::LOAD, 10}.toString() == "LOAD 10");
    CHECK(Instruction{Opcode::HALT, 0}.toString() == "HALT");
    CHECK(Instruction{}.toString() == "INVALID");
}

TEST_CASE("T-STR-003: Flags::toString lists all five flags") {
    Flags f;
    f.zero = true;
    f.overflow = true;
    CHECK(f.toString() == "Z=1 N=0 C=0 V=1 E=0");
}

TEST_CASE("T-STR-004: CpuRegisters::toString includes IR and flags") {
    CpuRegisters r;
    r.pc = 4;
    r.ir = Instruction{Opcode::ADD, 3};
    CHECK(r.toString().find("PC=4") != std::string::npos);
    CHECK(r.toString().find("IR=ADD 3") != std::string::npos);
}

TEST_CASE("T-STR-005: state and interrupt to-string helpers") {
    CHECK(processStateToString(ProcessState::NEW) == "NEW");
    CHECK(processStateToString(ProcessState::TERMINATED) == "TERMINATED");
    CHECK(processStateToString(ProcessState::FAILED) == "FAILED");
    CHECK(cpuStateToString(CpuState::IDLE) == "IDLE");
    CHECK(cpuStateToString(CpuState::HALTED) == "HALTED");
    CHECK(interruptTypeToString(InterruptType::SYSTEM_CALL) == "SYSTEM_CALL");
    CHECK(eventTypeToString(EventType::CPU_ERROR) == "CPU_ERROR");
    CHECK(eventTypeToString(EventType::PROCESS_STATE_CHANGED) == "PROCESS_STATE_CHANGED");
}

// ---------------------------------------------------------------------------
// T-PRS: Instruction::parse
// ---------------------------------------------------------------------------
TEST_CASE("T-PRS-001: parses mnemonics with operands") {
    Instruction out;
    REQUIRE(Instruction::parse("LOAD 10", out));
    CHECK(out.opcode == Opcode::LOAD);
    CHECK(out.operand == 10);
}

TEST_CASE("T-PRS-002: parses HALT without an operand") {
    Instruction out;
    REQUIRE(Instruction::parse("HALT", out));
    CHECK(out.opcode == Opcode::HALT);
}

TEST_CASE("T-PRS-003: mnemonics are case-insensitive") {
    Instruction out;
    REQUIRE(Instruction::parse("  load   10  ", out));
    CHECK(out.opcode == Opcode::LOAD);
    CHECK(out.operand == 10);
}

TEST_CASE("T-PRS-004: negative operands parse") {
    Instruction out;
    REQUIRE(Instruction::parse("ADD -5", out));
    CHECK(out.opcode == Opcode::ADD);
    CHECK(out.operand == -5);
}

TEST_CASE("T-PRS-005: rejects unknown mnemonics, missing operands, junk") {
    Instruction out;
    CHECK_FALSE(Instruction::parse("FOO 1", out));
    CHECK_FALSE(Instruction::parse("LOAD", out));
    CHECK_FALSE(Instruction::parse("HALT 5", out));
    CHECK_FALSE(Instruction::parse("", out));
    CHECK_FALSE(Instruction::parse("LOAD 1x", out));
}