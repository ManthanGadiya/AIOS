#pragma once

#include <aios/core/EventLog.hpp>
#include <aios/core/SimulationClock.hpp>
#include <aios/core/types.hpp>
#include <aios/process/ProcessControlBlock.hpp>

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace aios {

class Memory;
class CPU;
class MemoryManager;

// ---------------------------------------------------------------------------
// Process manager (docs/06).
// Holds PCBs, allocates PIDs, and enforces the documented state machine:
//   NEW -> READY -> RUNNING -> {READY | WAITING | TERMINATED | FAILED}
//   WAITING -> READY
// Process selection is owned by the Scheduler (docs/08); ProcessManager only
// executes a dispatch (READY -> RUNNING) that the scheduler requests.
// ---------------------------------------------------------------------------
class ProcessManager {
public:
    ProcessManager() = default;

    void setMemory(Memory* m) { memory_ = m; }
    void setEventLog(EventLog* log) { eventLog_ = log; }
    void setCPU(CPU* cpu) { cpu_ = cpu; }
    void setClock(SimulationClock* clock) { clock_ = clock; }
    void setMemoryManager(MemoryManager* mm) { memoryManager_ = mm; }

    // Creates a PCB and loads `program` into memory at `base`.
    // When a MemoryManager is attached, the program image goes to swap and the
    // logical space starts at 0 (demand paging, docs/07 section 29); otherwise
    // the Stage I flat-Memory path is used.  Returns the new PID, or
    // INVALID_PID on failure (e.g. overflow).
    int createProcess(const std::string& name, int priority,
                      const std::vector<int32_t>& program, uint32_t base,
                      ProcessType type = ProcessType::NORMAL);

    // Creates an AI-agent process (docs/06 section 34: createAgentProcess()).
    // AI agents are ordinary OS processes plus AI metadata (docs/06 section 19);
    // the AI Agent Manager milestone adds the metadata layer.
    int createAgentProcess(const std::string& name, int priority,
                           const std::vector<int32_t>& program, uint32_t base);

    bool admit(int pid);      // NEW -> READY
    bool dispatch(int pid);   // READY -> RUNNING (preempts current if any)
    bool terminate(int pid);  // RUNNING -> TERMINATED
    bool blockProcess(int pid); // RUNNING -> WAITING
    bool wakeProcess(int pid);  // WAITING -> READY
    bool markFailed(int pid);   // -> FAILED (unrecoverable error)

    ProcessControlBlock* getProcess(int pid);
    const ProcessControlBlock* getProcess(int pid) const;

    std::vector<int> readyQueue() const; // READY pids, by pid
    std::vector<int> processIds() const;
    size_t processCount() const { return pcbs_.size(); }
    int runningPid() const;
    bool isAlive(int pid) const {
        const auto* pcb = getProcess(pid);
        return pcb && pcb->isAlive();
    }

    // Process statistics (docs/06 sections 28-29). Returns nullopt for an
    // unknown pid.
    std::optional<ProcessStatistics> getProcessStatistics(int pid) const;
    std::vector<ProcessStatistics> getAllProcessStatistics() const;

    // Fraction of simulated time during which the CPU executed a process
    // (docs/06 section 29). Includes the currently running process's uncharged
    // run time.
    double cpuUtilization() const;

    void reset();

private:
    bool transition(int pid, ProcessState to, const std::string& reason);
    void saveRunningContext();
    void releaseCpuIfRunning(int pid);
    void chargeCpuTime(ProcessControlBlock& pcb);
    void chargeWaitingTime(ProcessControlBlock& pcb);

    std::map<int, ProcessControlBlock> pcbs_;
    int nextPid_ = 1;

    Memory* memory_ = nullptr;
    EventLog* eventLog_ = nullptr;
    CPU* cpu_ = nullptr;
    SimulationClock* clock_ = nullptr;
    MemoryManager* memoryManager_ = nullptr;
};

} // namespace aios