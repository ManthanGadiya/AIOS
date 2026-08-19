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
    ProcessState state = ProcessState::NEW;

    uint32_t baseAddress = 0;
    uint32_t programSize = 0;

    CpuRegisters context; // PC/ACC/MAR/MBR/IR/FLAGS snapshot

    uint64_t createdCycle = 0;
    uint64_t terminatedCycle = 0;
    uint64_t cpuTime = 0; // simulated cycles actually on the CPU

    bool isAlive() const {
        return state != ProcessState::TERMINATED && state != ProcessState::FAILED;
    }
};

} // namespace aios