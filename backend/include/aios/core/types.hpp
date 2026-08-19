#pragma once

#include <cstdint>
#include <functional>
#include <map>
#include <optional>
#include <ostream>
#include <string>
#include <vector>

namespace aios {

constexpr int INVALID_PID = -1;

// ---------------------------------------------------------------------------
// Instruction set (docs/04 section 11).
// Word encoding: (opcode_id << 16) | (operand & 0xFFFF).  [decision D2]
// ---------------------------------------------------------------------------
enum class Opcode : int32_t {
    LOAD = 0,    // ACC = mem[operand]            (operand = address)
    STORE = 1,   // mem[operand] = ACC            (operand = address)
    ADD = 2,     // ACC = ACC + operand           (operand = immediate value)
    SUB = 3,     // ACC = ACC - operand           (operand = immediate value)
    MUL = 4,     // ACC = ACC * operand           (operand = immediate value)
    DIV = 5,     // ACC = ACC / operand           (operand = immediate value)
    JMP = 6,     // PC = operand                  (operand = address)
    JZ = 7,      // if ZERO flag: PC = operand    (operand = address)
    READ = 8,    // mem[operand] = next input     (operand = address)
    WRITE = 9,   // output mem[operand]           (operand = address)
    SYSCALL = 10,// generate SYSTEM_CALL interrupt, operand = syscall number
    HALT = 11,   // stop the CPU
    INVALID = 12 // reserved / decode error
};

constexpr int32_t OPCODE_ID_MASK = 0x1F;
constexpr int32_t OPERAND_MASK = 0xFFFF;

std::string opcodeToString(Opcode opcode);

// ---------------------------------------------------------------------------
// Flags register (docs/04 section 12).
// ---------------------------------------------------------------------------
struct Flags {
    bool zero = false;
    bool negative = false;
    bool carry = false;
    bool overflow = false;
    bool error = false;

    std::string toString() const;
};

// ---------------------------------------------------------------------------
// Instruction (docs/04 sections 11-13). The IR is decoded into this struct.
// ---------------------------------------------------------------------------
struct Instruction {
    Opcode opcode = Opcode::INVALID;
    int32_t operand = 0;

    // The empty IR used at CPU reset / before the first fetch.  [decision D4]
    bool isEmpty() const { return opcode == Opcode::INVALID && operand == 0; }

    // Encodes as (opcode_id << 16) | (operand & 0xFFFF).  [decision D2]
    int32_t toWord() const {
        return (static_cast<int32_t>(opcode) << 16) | (operand & OPERAND_MASK);
    }

    static Instruction fromWord(int32_t word) {
        const int32_t opId = (word >> 16) & OPCODE_ID_MASK;
        Opcode op = (opId >= 0 && opId <= static_cast<int32_t>(Opcode::INVALID))
                        ? static_cast<Opcode>(opId)
                        : Opcode::INVALID;
        // Sign-extend the low 16 bits so negative operands survive encoding.
        const int32_t operand = static_cast<int16_t>(word & OPERAND_MASK);
        return Instruction{op, operand};
    }

    // True for every instruction except HALT.
    bool hasOperand() const { return opcode != Opcode::HALT; }

    // Parses text like "LOAD 10", "ADD 5" or "HALT" (case-insensitive mnemonic).
    static bool parse(const std::string& text, Instruction& out);

    std::string toString() const;
};

// ---------------------------------------------------------------------------
// CPU registers (docs/04 section 13).
// ---------------------------------------------------------------------------
struct CpuRegisters {
    int32_t pc = 0;
    int32_t acc = 0;
    int32_t mar = 0;
    int32_t mbr = 0;
    Instruction ir;
    Flags flags;

    std::string toString() const;
};

// ---------------------------------------------------------------------------
// Process states (docs/06 section 15, ROADMAP section 5.3).
// ---------------------------------------------------------------------------
enum class ProcessState : int32_t {
    NEW = 0,
    READY = 1,
    RUNNING = 2,
    WAITING = 3,
    TERMINATED = 4,
    FAILED = 5
};

std::string processStateToString(ProcessState state);

// ---------------------------------------------------------------------------
// CPU states (docs/04 section 14).
// ---------------------------------------------------------------------------
enum class CpuState : int32_t {
    IDLE = 0,
    RUNNING = 1,
    INTERRUPTED = 2,
    HALTED = 3
};

std::string cpuStateToString(CpuState state);

// ---------------------------------------------------------------------------
// Interrupts (docs/05 section 4).
// ---------------------------------------------------------------------------
enum class InterruptType : int32_t {
    TIMER = 0,
    SYSTEM_CALL = 1,
    IO_COMPLETE = 2,
    ERROR = 3
};

enum class InterruptPhase : int32_t {
    IDLE = 0,
    PENDING = 1,
    SERVICING = 2,
    COMPLETED = 3
};

struct InterruptRequest {
    int id = 0;
    InterruptType type = InterruptType::TIMER;
    int pid = INVALID_PID;
    int32_t data = 0; // e.g. system call number
    InterruptPhase phase = InterruptPhase::IDLE;
};

std::string interruptTypeToString(InterruptType type);

// ---------------------------------------------------------------------------
// System calls (docs/05 section 4):
//   1 READ, 2 WRITE, 3 ALLOCATE, 4 EXIT, 5 IPC_SEND, 6 IO_REQUEST
// ---------------------------------------------------------------------------
constexpr int32_t SYSCALL_READ = 1;
constexpr int32_t SYSCALL_WRITE = 2;
constexpr int32_t SYSCALL_ALLOCATE = 3;
constexpr int32_t SYSCALL_EXIT = 4;
constexpr int32_t SYSCALL_IPC_SEND = 5;
constexpr int32_t SYSCALL_IO_REQUEST = 6;

enum class SyscallStatus : int32_t {
    OK = 0,
    ERROR = 1,
    UNSUPPORTED = 2,
    INVALID = 3
};

struct SyscallRequest {
    int pid = INVALID_PID;
    int32_t syscallNumber = 0;
};

struct SyscallResult {
    SyscallStatus status = SyscallStatus::OK;
    int32_t returnValue = 0;
    std::string message;
};

// ---------------------------------------------------------------------------
// Event log (docs/04 section 27, ROADMAP section 5.5).
// ---------------------------------------------------------------------------
enum class EventType : int32_t {
    PROCESS_CREATED = 0,
    PROCESS_STATE_CHANGED = 1,
    CONTEXT_SWITCH = 2,
    PAGE_FAULT = 3,
    INTERRUPT = 4,
    IPC_MESSAGE = 5,
    IO_REQUEST = 6,
    IO_COMPLETE = 7,
    PROCESS_TERMINATED = 8,
    INSTRUCTION_FETCH = 9,
    INSTRUCTION_EXECUTE = 10,
    REGISTER_UPDATE = 11,
    SYSCALL = 12,
    HALT = 13,
    CPU_ERROR = 14
};

std::string eventTypeToString(EventType type);

struct Event {
    EventType type = EventType::PROCESS_CREATED;
    int pid = INVALID_PID;
    uint64_t cycle = 0;
    std::string detail;
};

} // namespace aios