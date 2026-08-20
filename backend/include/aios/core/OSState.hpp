#pragma once

#include <aios/core/EventLog.hpp>
#include <aios/core/SimulationClock.hpp>
#include <aios/cpu/CPU.hpp>
#include <aios/interrupt/InterruptManager.hpp>
#include <aios/interrupt/SystemCallManager.hpp>
#include <aios/memory/Memory.hpp>
#include <aios/memory/MemoryManager.hpp>
#include <aios/process/ProcessManager.hpp>

#include <cstdint>
#include <deque>
#include <ostream>

namespace aios {

// ---------------------------------------------------------------------------
// OS state container: owns every subsystem and wires their dependencies
// (docs/03). The frontend talks to this object's subsystems only through
// the documented interfaces — it never touches simulated hardware directly.
// ---------------------------------------------------------------------------
class OSState {
public:
    OSState() { wire(); }

    SimulationClock& clock() { return clock_; }
    EventLog& eventLog() { return eventLog_; }
    Memory& memory() { return memory_; }
    MemoryManager& memoryManager() { return memoryManager_; }
    CPU& cpu() { return cpu_; }
    InterruptManager& interruptManager() { return interrupts_; }
    SystemCallManager& systemCallManager() { return syscalls_; }
    ProcessManager& processManager() { return processes_; }

    void setOutputStream(std::ostream* out);
    void setInputQueue(std::deque<int32_t> input);

    void reset();

    // Stage I runner: executes process `pid` (already admitted + dispatched)
    // until it halts, exits, or fails. [decision D7]
    void runStageI(int pid);

private:
    void wire();

    SimulationClock clock_;
    EventLog eventLog_;
    Memory memory_;
    MemoryManager memoryManager_;
    SystemCallManager syscalls_;
    ProcessManager processes_;
    InterruptManager interrupts_;
    CPU cpu_;

    std::ostream* output_ = nullptr;
    std::deque<int32_t> input_;
};

} // namespace aios