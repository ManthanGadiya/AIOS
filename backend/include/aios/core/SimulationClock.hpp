#pragma once

#include <cstdint>

namespace aios {

// ---------------------------------------------------------------------------
// Simulated system clock (docs/04 section 5).
// Header-only: the clock is a trivial counter.
// ---------------------------------------------------------------------------
class SimulationClock {
public:
    SimulationClock() = default;

    uint64_t cycle() const { return cycle_; }
    void tick() { ++cycle_; }
    void reset() { cycle_ = 0; }

private:
    uint64_t cycle_ = 0;
};

} // namespace aios