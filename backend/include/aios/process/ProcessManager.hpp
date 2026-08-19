#pragma once

#include <aios/core/EventLog.hpp>
#include <aios/core/SimulationClock.hpp>
#include <aios/core/types.hpp>
#include <aios/process/ProcessControlBlock.hpp>

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace aios {

class Memory;
class CPU;

// ---------------------------------------------------------------------------
// Process manager (docs/06).
// Holds PCBs, allocates PIDs, and enforces the documented state machine:
//   NEW -> READY -> RUNNING -> {READY | WAITING | TERMINATED | FAILED}
//   WAITING -> READY
// Stage I uses direct dispatch (no scheduler object yet).  [decision D7]
// ---------------------------------------------------------------------------
class ProcessManager {
public:
    ProcessManager() = default;

    void setMemory(Memory* m) { memory_ = m; }
    void setEventLog(EventLog* log) { eventLog_ = log; }
    void setCPU(CPU* cpu) { cpu_ = cpu; }
    void setClock(SimulationClock* clock) { clock_ = clock; }

    // Creates a PCB and loads `program` into memory at `base`.
    // Returns the new PID, or INVALID_PID on failure (e.g. overflow).
    int createProcess(const std::string& name, int priority,
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

    void reset();

private:
    bool transition(int pid, ProcessState to, const std::string& reason);
    void saveRunningContext();
    void releaseCpuIfRunning(int pid);

    std::map<int, ProcessControlBlock> pcbs_;
    int nextPid_ = 1;

    Memory* memory_ = nullptr;
    EventLog* eventLog_ = nullptr;
    CPU* cpu_ = nullptr;
    SimulationClock* clock_ = nullptr;
};

} // namespace aios