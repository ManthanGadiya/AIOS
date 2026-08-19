#pragma once

#include <aios/core/OSState.hpp>

#include <cstdint>

namespace aios::demo {

// ---------------------------------------------------------------------------
// Stage I runner (demo layer).
// Thin wrapper around OSState::runStageI so the demo narrative is explicit.
// ---------------------------------------------------------------------------
class StageIRunner {
public:
    // Runs process `pid` to completion; returns the pid (or INVALID_PID if
    // the process does not exist).
    static int run(OSState& os, int pid);
};

} // namespace aios::demo