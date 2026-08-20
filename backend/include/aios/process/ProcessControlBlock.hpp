#pragma once

#include <aios/core/types.hpp>

#include <cstdint>
#include <string>

namespace aios {

// ---------------------------------------------------------------------------
// Process Control Block (docs/06 section 14).
// ---------------------------------------------------------------------------
struct ProcessControlBlock {
    int pid = INVALID_PID;
    std::string name;
    int priority = 0;
    ProcessType type = ProcessType::NORMAL;
    ProcessState state = ProcessState::NEW;

    uint32_t baseAddress = 0;
    uint32_t programSize = 0;

    CpuRegisters context; // PC/ACC/MAR/MBR/IR/FLAGS snapshot

    uint64_t createdCycle = 0;
    uint64_t terminatedCycle = 0;
    uint64_t cpuTime = 0; // simulated cycles actually on the CPU

    // Resource tracking (docs/06 sections 18, 28, 29).
    uint64_t firstRunCycle = 0;  // first dispatch onto the CPU (response time)
    uint64_t lastCpuCycle = 0;   // cycle of the most recent dispatch
    bool onCpu = false;          // currently on the CPU (CPU-time accounting)
    uint64_t waitingTime = 0;    // cycles spent in the READY queue
    uint64_t readySinceCycle = 0;// cycle of the most recent transition into READY
    bool inReadyQueue = false;   // currently on the READY queue (waiting-time accounting)
    uint64_t stateChangeCount = 0;
    uint64_t contextSwitchCount = 0; // times dispatched onto the CPU
    uint64_t ioRequests = 0;
    uint64_t ipcOperations = 0;

    bool isAlive() const {
        return state != ProcessState::TERMINATED && state != ProcessState::FAILED;
    }
};

} // namespace aios