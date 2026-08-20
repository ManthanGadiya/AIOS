#include <aios/interrupt/Timer.hpp>

#include <aios/interrupt/InterruptManager.hpp>

namespace aios {

int Timer::tick() {
    ++cycles_;
    ++elapsed_;
    if (quantum_ > 0 && elapsed_ >= quantum_) {
        elapsed_ = 0;
        if (interrupts_) {
            return interrupts_->generateInterrupt(InterruptType::TIMER, INVALID_PID,
                                                  quantum_);
        }
    }
    return INVALID_INTERRUPT;
}

void Timer::reset() {
    cycles_ = 0;
    elapsed_ = 0;
}

} // namespace aios