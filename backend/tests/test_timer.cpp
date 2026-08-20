#include <aios/core/types.hpp>
#include <aios/interrupt/InterruptManager.hpp>
#include <aios/interrupt/Timer.hpp>

#include <doctest/doctest.h>

using namespace aios;

TEST_CASE("T-TIMER-001: timer fires a TIMER interrupt when the quantum elapses") {
    InterruptManager im;
    Timer timer;
    timer.setInterruptManager(&im);
    timer.setQuantum(4);

    CHECK(timer.cycles() == 0);
    CHECK(timer.elapsed() == 0);

    for (int i = 0; i < 3; ++i) {
        CHECK(timer.tick() == INVALID_INTERRUPT);
        CHECK_FALSE(im.hasPending());
    }

    // The 4th tick reaches the quantum and generates a TIMER interrupt.
    CHECK(timer.tick() > INVALID_INTERRUPT);
    CHECK(im.pendingCount() == 1);
    CHECK(im.log()[0].type == InterruptType::TIMER);
    CHECK(im.log()[0].phase == InterruptPhase::PENDING);
    CHECK(im.log()[0].data == 4);

    // The timer restarts after firing.
    CHECK(timer.elapsed() == 0);
    CHECK(timer.cycles() == 4);
}

TEST_CASE("T-TIMER-002: timer fires periodically every quantum") {
    InterruptManager im;
    Timer timer;
    timer.setInterruptManager(&im);
    timer.setQuantum(2);

    CHECK(timer.tick() == INVALID_INTERRUPT); // cycle 1
    CHECK(timer.tick() > INVALID_INTERRUPT);  // cycle 2 -> fire
    CHECK(im.pendingCount() == 1);

    CHECK(timer.tick() == INVALID_INTERRUPT); // cycle 3
    CHECK(timer.tick() > INVALID_INTERRUPT);  // cycle 4 -> fire
    CHECK(im.pendingCount() == 2);
    CHECK(timer.cycles() == 4);
}

TEST_CASE("T-TIMER-003: setQuantum restarts the timer") {
    InterruptManager im;
    Timer timer;
    timer.setInterruptManager(&im);

    timer.setQuantum(3);
    CHECK(timer.tick() == INVALID_INTERRUPT);
    CHECK(timer.tick() == INVALID_INTERRUPT);
    CHECK(timer.tick() > INVALID_INTERRUPT); // fires at cycle 3
    CHECK(im.pendingCount() == 1);

    timer.setQuantum(1); // restart with a shorter quantum
    CHECK(timer.cycles() == 0);
    CHECK(timer.elapsed() == 0);
    CHECK(timer.tick() > INVALID_INTERRUPT); // fires on the next tick
    CHECK(im.pendingCount() == 2);
}

TEST_CASE("T-TIMER-004: reset clears timer state") {
    InterruptManager im;
    Timer timer;
    timer.setInterruptManager(&im);
    timer.setQuantum(2);

    timer.tick();
    timer.tick(); // fires
    CHECK(timer.cycles() == 2);
    CHECK(im.pendingCount() == 1);

    timer.reset();
    CHECK(timer.cycles() == 0);
    CHECK(timer.elapsed() == 0);

    // A fresh quantum is required before the next fire.
    CHECK(timer.tick() == INVALID_INTERRUPT);
    CHECK(timer.tick() > INVALID_INTERRUPT);
}

TEST_CASE("T-TIMER-005: a quantum of zero disables the timer") {
    InterruptManager im;
    Timer timer;
    timer.setInterruptManager(&im);
    timer.setQuantum(0);

    for (int i = 0; i < 10; ++i) {
        CHECK(timer.tick() == INVALID_INTERRUPT);
    }
    CHECK_FALSE(im.hasPending());
    CHECK(timer.cycles() == 10);
}

TEST_CASE("T-TIMER-006: default quantum matches docs/05 section 32 example") {
    InterruptManager im;
    Timer timer;
    timer.setInterruptManager(&im);

    CHECK(timer.quantum() == 4); // default quantum (docs/05 section 32)
    for (int i = 0; i < 3; ++i) {
        CHECK(timer.tick() == INVALID_INTERRUPT);
    }
    CHECK(timer.tick() > INVALID_INTERRUPT);
}

TEST_CASE("T-TIMER-007: generated TIMER interrupt is serviced and completed") {
    InterruptManager im;
    Timer timer;
    timer.setInterruptManager(&im);
    timer.setQuantum(1);

    const int id = timer.tick();
    CHECK(id > INVALID_INTERRUPT);
    REQUIRE(im.serviceNextInterrupt());
    CHECK(im.log()[0].phase == InterruptPhase::COMPLETED);
    CHECK(im.pendingCount() == 0);
}