#pragma once

#include <aios/core/types.hpp>

namespace aios {

class InterruptManager;

// ---------------------------------------------------------------------------
// Timer (docs/05 sections 10 and 15).
// A periodic timer that generates TIMER interrupts when a configured quantum
// of simulated cycles elapses. This lets the OS regain control of the CPU
// periodically and is the basis for preemptive scheduling (Round Robin,
// docs/08, Week 5). The OS loop calls tick() once per simulated cycle; a
// quantum of 0 disables the timer.
// ---------------------------------------------------------------------------
class Timer {
public:
    Timer() = default;

    void setInterruptManager(InterruptManager* im) { interrupts_ = im; }

    // Sets the quantum in simulated cycles and restarts the timer.
    void setQuantum(int32_t cycles) {
        quantum_ = cycles;
        reset();
    }
    int32_t quantum() const { return quantum_; }

    // Advances the timer by one simulated cycle. When the quantum elapses a
    // TIMER interrupt is generated and the timer restarts; returns the
    // interrupt id, or INVALID_INTERRUPT when no interrupt was generated.
    int tick();

    int32_t cycles() const { return cycles_; }   // total simulated cycles seen
    int32_t elapsed() const { return elapsed_; } // cycles since last quantum

    void reset();

private:
    InterruptManager* interrupts_ = nullptr;
    int32_t quantum_ = 4; // default quantum (docs/05 section 32 example)
    int32_t cycles_ = 0;
    int32_t elapsed_ = 0;
};

} // namespace aios