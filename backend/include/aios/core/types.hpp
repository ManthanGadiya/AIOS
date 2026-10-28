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
constexpr int INVALID_INTERRUPT = 0;

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
// I/O Operation types (docs/11 section 7).
// ---------------------------------------------------------------------------
enum class IOOperation : int32_t {
    READ = 0,
    WRITE = 1
};

std::string ioOperationToString(IOOperation op);

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

// Process kind (docs/06 section 27: the process table has a Type column).
enum class ProcessType : int32_t {
    NORMAL = 0,
    AI_AGENT = 1
};

std::string processTypeToString(ProcessType type);

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
    ERR = 3,
    PAGE_FAULT = 4
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
std::string ioOperationToString(IOOperation op);

// ---------------------------------------------------------------------------
// System calls (docs/05 section 4):
//   1 READ, 2 WRITE, 3 ALLOCATE, 4 EXIT, 5 IPC_SEND, 6 IO_REQUEST,
//   7 LOCK, 8 UNLOCK, 9 WAIT, 10 SIGNAL, 11 IPC_RECEIVE
// ---------------------------------------------------------------------------
constexpr int32_t SYSCALL_READ = 1;
constexpr int32_t SYSCALL_WRITE = 2;
constexpr int32_t SYSCALL_ALLOCATE = 3;
constexpr int32_t SYSCALL_EXIT = 4;
constexpr int32_t SYSCALL_IPC_SEND = 5;
constexpr int32_t SYSCALL_IO_REQUEST = 6;
constexpr int32_t SYSCALL_LOCK = 7;
constexpr int32_t SYSCALL_UNLOCK = 8;
constexpr int32_t SYSCALL_WAIT = 9;
constexpr int32_t SYSCALL_SIGNAL = 10;
constexpr int32_t SYSCALL_IPC_RECEIVE = 11;

enum class SyscallStatus : int32_t {
    OK = 0,
    ERR = 1,
    UNSUPPORTED = 2,
    INVALID = 3
};

struct SyscallRequest {
    int pid = INVALID_PID;
    int32_t syscallNumber = 0;
    std::string payload; // Additional parameters for complex syscalls (e.g., I/O)
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
    CPU_ERROR = 14,
    PAGE_ALLOCATED = 15,
    PAGE_ACCESSED = 16,
    FRAME_ALLOCATED = 17,
    PAGE_REPLACED = 18,
    PAGE_LOADED = 19,
    PAGE_SWAPPED_OUT = 20,
    PAGE_SWAPPED_IN = 21,
    INVALID_MEMORY_ACCESS = 22,
    SCHEDULER_STARTED = 23,
    PROCESS_SELECTED = 24,
    PROCESS_PREEMPTED = 25,
    TIME_QUANTUM_EXPIRED = 26,
    // Synchronization (docs/09 section 33)
    MUTEX_CREATED = 27,
    MUTEX_LOCKED = 28,
    MUTEX_UNLOCKED = 29,
    PROCESS_BLOCKED_ON_MUTEX = 30,
    PROCESS_WAKEUP = 31,
    SEMAPHORE_CREATED = 32,
    SEMAPHORE_WAIT = 33,
    SEMAPHORE_SIGNAL = 34,
    DEADLOCK_DETECTED = 35,
    SYNCHRONIZATION_ERROR = 36,
    // IPC (docs/10 section 38)
    IPC_CHANNEL_CREATED = 37,
    IPC_SEND = 38,
    IPC_RECEIVE = 39,
    IPC_MESSAGE_QUEUED = 40,
    IPC_MESSAGE_DELIVERED = 41,
    IPC_QUEUE_FULL = 42,
    IPC_PERMISSION_DENIED = 43,
    IPC_ERROR = 44,
    // I/O (docs/11 section 40)
    IO_ERROR = 45,
    IO_STARTED = 46,
    IO_BLOCKED = 47,
    // AI Agent Layer (docs/12 section 49)
    AGENT_CREATED = 48,
    AGENT_STARTED = 49,
    AGENT_CHECKPOINTED = 50,
    AGENT_TOOL_REQUEST = 51,
    AGENT_TOOL_DENIED = 52,
    AGENT_FAILURE = 53,
    AGENT_RECOVERY_STARTED = 54,
    AGENT_RECOVERED = 55,
    AGENT_TERMINATED = 56
};

std::string eventTypeToString(EventType type);

struct Event {
    EventType type = EventType::PROCESS_CREATED;
    int pid = INVALID_PID;
    uint64_t cycle = 0;
    std::string detail;
};

// ---------------------------------------------------------------------------
// Process statistics (docs/06 sections 28-29).
// ---------------------------------------------------------------------------
struct ProcessStatistics {
    int pid = INVALID_PID;
    std::string name;
    ProcessState state = ProcessState::NEW;
    ProcessType type = ProcessType::NORMAL;
    int priority = 0;
    uint32_t baseAddress = 0;
    uint32_t programSize = 0;
    uint64_t arrivalTime = 0;    // creation cycle
    uint64_t cpuTime = 0;        // cycles on the CPU
    uint64_t waitingTime = 0;    // cycles in the READY queue
    uint64_t turnaroundTime = 0; // completion - arrival (0 while alive)
    uint64_t responseTime = 0;   // first-run - arrival (0 if never run)
    uint64_t stateChanges = 0;
    uint64_t contextSwitches = 0;
    uint64_t pageFaults = 0;
    uint64_t ioRequests = 0;
    uint64_t ipcOperations = 0;
};

// ---------------------------------------------------------------------------
// Paging structures (docs/07).
// ---------------------------------------------------------------------------

// One page-table entry (docs/07 section 8): maps a logical page to a frame
// and tracks present/valid/reference/dirty state.
struct PageTableEntry {
    uint32_t frame = 0;
    bool present = false;   // page is resident in RAM (docs/07 section 9)
    bool valid = true;      // page belongs to the process's valid space (section 10)
    bool referenced = false;
    bool dirty = false;     // page was written since load (docs/07 section 21)
};

// A process's logical -> physical mapping (docs/07 sections 7-10, 26).
struct PageTable {
    int pid = INVALID_PID;
    uint32_t pageCount = 0;
    std::vector<PageTableEntry> entries;
};

// Result of a single logical-memory access (docs/07 section 36).
enum class MemoryAccessStatus {
    OK,
    PAGE_FAULT, // valid page, not resident
    INVALID     // invalid page / out of bounds
};

struct MemoryAccessResult {
    MemoryAccessStatus status = MemoryAccessStatus::INVALID;
    int32_t value = 0;          // valid when status == OK and it was a read
    uint32_t page = 0;          // faulting page when status == PAGE_FAULT
    uint32_t physicalAddress = 0;
};

// Frame occupancy row (docs/07 section 30).
struct FrameEntry {
    bool occupied = false;
    int pid = INVALID_PID;
    uint32_t page = 0;
};

// Collected memory statistics (docs/07 sections 34-35).
struct MemoryStatistics {
    uint32_t pageSize = 0;
    uint32_t totalFrames = 0;
    uint32_t usedFrames = 0;
    uint32_t totalWords = 0;
    uint32_t usedWords = 0;
    uint64_t pageFaultCount = 0;
    uint64_t pageReplacementCount = 0;
    uint64_t swapWords = 0; // words currently held in simulated swap
    std::map<int, uint64_t> perProcessFaults;
};

// ---------------------------------------------------------------------------
// AI Agent Layer (docs/12)
// ---------------------------------------------------------------------------

// AI Agent types (docs/12 section 7)
enum class AgentType : int32_t {
    RESEARCH = 0,
    CODING = 1
};

std::string agentTypeToString(AgentType type);

// AI Agent internal state (docs/12 section 13)
enum class AgentState : int32_t {
    ACTIVE = 0,        // Running normally
    WAITING = 1,       // Waiting for IPC/I/O/sync
    CHECKPOINTED = 2,  // Checkpoint created, ready for recovery
    FAILED = 3,        // Simulated failure
    RECOVERED = 4      // Restored from checkpoint, going to READY
};

std::string agentStateToString(AgentState state);

// Tool permissions (docs/12 section 27)
enum class ToolPermission : int32_t {
    READ_DATA = 0,
    SEARCH = 1,
    WRITE_CODE = 2,
    EXECUTE = 3
};

std::string toolPermissionToString(ToolPermission perm);

// Resource types for quota tracking (docs/12 section 23)
enum class ResourceType : int32_t {
    CPU = 0,
    MEMORY = 1,
    TOKEN = 2,
    IO = 3
};

std::string resourceTypeToString(ResourceType type);

// Failure types (docs/12 section 30)
enum class FailureType : int32_t {
    RESOURCE_LIMIT = 0,
    SIMULATED_CRASH = 1,
    EXECUTION_ERROR = 2,
    TOOL_PERMISSION_ERROR = 3,
    MEMORY_ERROR = 4
};

std::string failureTypeToString(FailureType type);

// Resource quotas (docs/12 section 23)
struct ResourceQuota {
    uint64_t cpuQuota = 0;       // Maximum CPU cycles per period
    uint64_t cpuUsed = 0;        // Cycles used in current period
    uint32_t memoryQuota = 0;    // Maximum pages
    uint32_t memoryUsed = 0;     // Pages currently allocated
    uint64_t tokenBudget = 0;    // Maximum tokens
    uint64_t tokenUsed = 0;      // Tokens consumed
    uint64_t ioLimit = 0;        // Maximum I/O operations
    uint64_t ioUsed = 0;         // I/O operations used
};

// Checkpoint data (docs/12 section 31)
struct Checkpoint {
    std::string checkpointId;
    uint64_t createdCycle = 0;
    std::string agentId;
    int pid = INVALID_PID;
    std::string progress;        // e.g., "Step 4"
    CpuRegisters cpuContext;     // PC, ACC, MAR, MBR, IR, FLAGS
    ProcessState processState = ProcessState::NEW;
    // Memory references are tracked via page table - we store pageCount for validation
    uint32_t pageCount = 0;
};

// Agent metadata (docs/12 section 12)
struct AgentMetadata {
    std::string agentId;         // e.g., "AGENT-001"
    std::string name;            // e.g., "Research Agent"
    AgentType type = AgentType::RESEARCH;
    int pid = INVALID_PID;
    int priority = 0;
    uint64_t tokenBudget = 0;
    uint64_t tokenUsed = 0;
    ResourceQuota quota;
    std::vector<ToolPermission> permissions;
    std::string checkpointId;    // Latest checkpoint ID
    AgentState agentState = AgentState::ACTIVE;
    FailureType lastFailure = FailureType::SIMULATED_CRASH;
    uint64_t checkpointCount = 0;
    uint64_t failureCount = 0;
    uint64_t recoveryCount = 0;
    uint64_t cpuCyclesUsed = 0;
    uint64_t ipcSent = 0;
    uint64_t ipcReceived = 0;
    uint64_t toolRequests = 0;
    uint64_t toolDenials = 0;
};

// Agent statistics for dashboard (docs/12 section 39)
struct AgentStatistics {
    int pid = INVALID_PID;
    std::string agentId;
    std::string name;
    AgentType type = AgentType::RESEARCH;
    ProcessState processState = ProcessState::NEW;
    AgentState agentState = AgentState::ACTIVE;
    int priority = 0;
    std::string task;
    uint64_t tokenUsed = 0;
    uint64_t tokenBudget = 0;
    std::map<std::string, bool> permissions;
    uint64_t cpuTime = 0;
    uint32_t memoryUsage = 0;
    uint64_t ipcSent = 0;
    uint64_t ipcReceived = 0;
    uint64_t checkpointCount = 0;
    uint64_t failureCount = 0;
    uint64_t recoveryCount = 0;
    std::string lastCheckpointId;
};

} // namespace aios