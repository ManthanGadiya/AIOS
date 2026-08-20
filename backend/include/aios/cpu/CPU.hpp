#pragma once

#include <aios/core/EventLog.hpp>
#include <aios/core/SimulationClock.hpp>
#include <aios/core/types.hpp>

#include <deque>
#include <ostream>

namespace aios {

class Memory;
class MemoryManager;
class InterruptManager;

// ---------------------------------------------------------------------------
// Simulated CPU (docs/04).
// Fetches a word from memory (MAR -> MBR -> IR), advances PC by one word,
// then executes the instruction. Emits INSTRUCTION_FETCH / INSTRUCTION_EXECUTE
// / SYSCALL / HALT / CPU_ERROR events into the central event log.
// ---------------------------------------------------------------------------
class CPU {
public:
    CPU() = default;

    // Resets all registers; sets PC to initialPc. State stays IDLE until a
    // process context is loaded.
    void initialize(int32_t initialPc);

    // Loads a process context and marks the CPU RUNNING for that process.
    void loadContext(const CpuRegisters& regs, int pid);

    // Snapshots the current registers (used during context switch / syscall).
    CpuRegisters saveContext() const;

    // Back to IDLE with cleared registers.
    void reset();

    // Fetch + execute one instruction (only meaningful while RUNNING).
    void executeCycle();

    // Resumes after an interrupt has been serviced (INTERRUPTED -> RUNNING).
    void resume();

    CpuState state() const { return state_; }
    const CpuRegisters& registers() const { return regs_; }
    int currentProcess() const { return currentPid_; }
    bool isHalted() const { return state_ == CpuState::HALTED; }

    // Dependency injection (wired by OSState).
    void setMemory(Memory* memory) { memory_ = memory; }
    void setMemoryManager(MemoryManager* memoryManager) { memoryManager_ = memoryManager; }
    void setEventLog(EventLog* log) { eventLog_ = log; }
    void setClock(SimulationClock* clock) { clock_ = clock; }
    void setInterruptManager(InterruptManager* im) { interruptManager_ = im; }
    void setInputQueue(std::deque<int32_t>* input) { input_ = input; }
    void setOutputStream(std::ostream* output) { output_ = output; }

private:
    void fetch();
    void execute();
    void updateArithmeticFlags(int32_t result, int64_t exact, bool isAddOrSub);
    void raiseError(const std::string& detail);
    bool isJumpTargetValid(int32_t target) const;
    // On a page fault during execute the PC has already advanced past the
    // faulting instruction, so it is rolled back to MAR for the retry
    // (docs/07 section 16: "Retry Instruction").  [decision D8]
    void raisePageFault(uint32_t page, bool rollbackPc);
    void raiseInvalidAccess(uint32_t logicalAddress);
    void record(EventType type, const std::string& detail);

    CpuState state_ = CpuState::IDLE;
    CpuRegisters regs_;
    int currentPid_ = INVALID_PID;

    Memory* memory_ = nullptr;
    MemoryManager* memoryManager_ = nullptr;
    EventLog* eventLog_ = nullptr;
    SimulationClock* clock_ = nullptr;
    InterruptManager* interruptManager_ = nullptr;
    std::deque<int32_t>* input_ = nullptr;
    std::ostream* output_ = nullptr;
};

} // namespace aios