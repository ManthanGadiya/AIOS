#include <aios/core/EventLog.hpp>
#include <aios/core/SimulationClock.hpp>
#include <aios/core/types.hpp>
#include <aios/process/ProcessManager.hpp>
#include <aios/sync/SynchronizationManager.hpp>

#include <doctest/doctest.h>

#include <sstream>
#include <vector>

using namespace aios;

namespace {

// Minimal harness for synchronization tests
struct SyncHarness {
    EventLog log;
    SimulationClock clock;
    ProcessManager pm;
    SynchronizationManager sync;

    SyncHarness() {
        pm.setEventLog(&log);
        pm.setClock(&clock);
        sync.setEventLog(&log);
        sync.setProcessManager(&pm);
        sync.setClock(&clock);
    }

    bool hasEvent(EventType type) const {
        for (const auto& e : log.events()) {
            if (e.type == type) return true;
        }
        return false;
    }

    size_t eventCount(EventType type) const {
        size_t count = 0;
        for (const auto& e : log.events()) {
            if (e.type == type) ++count;
        }
        return count;
    }

    void advanceClock(int cycles = 1) {
        for (int i = 0; i < cycles; ++i) clock.tick();
    }
};

} // namespace

// ============================================================================
// Mutex Tests
// ============================================================================

TEST_CASE("T-SYNC-001: Mutex creation") {
    SyncHarness h;
    CHECK(h.sync.createMutex("M1"));
    CHECK_FALSE(h.sync.createMutex("M1")); // duplicate
    CHECK(h.hasEvent(EventType::MUTEX_CREATED));
    CHECK(h.sync.mutexCount() == 1);
}

TEST_CASE("T-SYNC-002: Mutex lock/unlock by owner") {
    SyncHarness h;
    h.sync.createMutex("M1");

    // Lock succeeds for first process
    CHECK(h.sync.lockMutex("M1", 1));
    CHECK(h.hasEvent(EventType::MUTEX_LOCKED));

    // Unlock by owner succeeds
    int next = h.sync.unlockMutex("M1", 1);
    CHECK(next == INVALID_PID);
    CHECK(h.hasEvent(EventType::MUTEX_UNLOCKED));
}

TEST_CASE("T-SYNC-003: Mutex lock contention blocks second process") {
    SyncHarness h;
    h.sync.createMutex("M1");

    CHECK(h.sync.lockMutex("M1", 1));     // P1 acquires
    CHECK_FALSE(h.sync.lockMutex("M1", 2)); // P2 blocked
    CHECK(h.hasEvent(EventType::PROCESS_BLOCKED_ON_MUTEX));
}

TEST_CASE("T-SYNC-004: Mutex wakeup on unlock") {
    SyncHarness h;
    h.sync.createMutex("M1");

    CHECK(h.sync.lockMutex("M1", 1));     // P1 acquires
    CHECK_FALSE(h.sync.lockMutex("M1", 2)); // P2 blocked

    int next = h.sync.unlockMutex("M1", 1); // P1 releases
    CHECK(next == 2);                       // P2 is next waiter
    CHECK(h.hasEvent(EventType::PROCESS_WAKEUP));
}

TEST_CASE("T-SYNC-005: Mutex unlock by non-owner fails") {
    SyncHarness h;
    h.sync.createMutex("M1");
    h.sync.lockMutex("M1", 1);

    int next = h.sync.unlockMutex("M1", 2); // Not owner
    CHECK(next == INVALID_PID);
    CHECK(h.hasEvent(EventType::SYNCHRONIZATION_ERROR));
}

TEST_CASE("T-SYNC-006: Mutex force-release on process termination") {
    SyncHarness h;
    h.sync.createMutex("M1");
    h.sync.lockMutex("M1", 1);
    CHECK_FALSE(h.sync.lockMutex("M1", 2)); // P2 blocked

    // P1 terminates - force release
    h.sync.cleanupProcess(1);

    // M1 should be free now, P2 should have been woken
    auto m = h.sync.getMutex("M1");
    CHECK(m.has_value());
    CHECK_FALSE(m->isLocked());
    CHECK(m->ownerPid() == INVALID_PID);
}

// ============================================================================
// Semaphore Tests
// ============================================================================

TEST_CASE("T-SYNC-007: Semaphore creation") {
    SyncHarness h;
    CHECK(h.sync.createSemaphore("S1", 3));
    CHECK_FALSE(h.sync.createSemaphore("S1", 0)); // duplicate
    CHECK(h.hasEvent(EventType::SEMAPHORE_CREATED));
    CHECK(h.sync.semaphoreCount() == 1);
}

TEST_CASE("T-SYNC-008: Semaphore wait/signal with count > 0") {
    SyncHarness h;
    h.sync.createSemaphore("S1", 2);

    // First wait - count becomes 1, acquired
    CHECK(h.sync.waitSemaphore("S1", 1));
    CHECK(h.hasEvent(EventType::SEMAPHORE_WAIT));

    // Second wait - count becomes 0, acquired
    CHECK(h.sync.waitSemaphore("S1", 2));
    CHECK(h.eventCount(EventType::SEMAPHORE_WAIT) == 2);

    // Signal - wakes no one (count becomes 1)
    int next = h.sync.signalSemaphore("S1");
    CHECK(next == INVALID_PID);
    CHECK(h.hasEvent(EventType::SEMAPHORE_SIGNAL));
}

TEST_CASE("T-SYNC-009: Semaphore wait blocks when count exhausted") {
    SyncHarness h;
    h.sync.createSemaphore("S1", 1);

    CHECK(h.sync.waitSemaphore("S1", 1));  // count=0, acquired
    CHECK_FALSE(h.sync.waitSemaphore("S1", 2)); // count=-1, blocked
    CHECK(h.hasEvent(EventType::SEMAPHORE_WAIT));
}

TEST_CASE("T-SYNC-010: Semaphore signal wakes waiter") {
    SyncHarness h;
    h.sync.createSemaphore("S1", 1);

    CHECK(h.sync.waitSemaphore("S1", 1));  // acquired
    CHECK_FALSE(h.sync.waitSemaphore("S1", 2)); // blocked

    int next = h.sync.signalSemaphore("S1"); // count=0, wakes P2
    CHECK(next == 2);
    CHECK(h.hasEvent(EventType::SEMAPHORE_SIGNAL));
    CHECK(h.hasEvent(EventType::PROCESS_WAKEUP));
}

TEST_CASE("T-SYNC-011: Semaphore cleanup on process termination") {
    SyncHarness h;
    h.sync.createSemaphore("S1", 1);
    CHECK(h.sync.waitSemaphore("S1", 1));  // acquired
    CHECK_FALSE(h.sync.waitSemaphore("S1", 2)); // blocked

    h.sync.cleanupProcess(2); // P2 terminates while waiting

    auto s = h.sync.getSemaphore("S1");
    CHECK(s.has_value());
    CHECK(s->waitingQueue().empty());
}

// ============================================================================
// Deadlock Detection Tests
// ============================================================================

TEST_CASE("T-SYNC-012: Deadlock detection - mutex cycle") {
    SyncHarness h;
    h.sync.createMutex("M1");
    h.sync.createMutex("M2");

    // P1 holds M1
    CHECK(h.sync.lockMutex("M1", 1));
    // P2 holds M2
    CHECK(h.sync.lockMutex("M2", 2));

    // P1 tries to get M2 (held by P2) -> blocked
    CHECK_FALSE(h.sync.lockMutex("M2", 1));
    // P2 tries to get M1 (held by P1) -> blocked
    CHECK_FALSE(h.sync.lockMutex("M1", 2));

    // Deadlock should be detected
    CHECK(h.sync.detectDeadlock());
    auto cycle = h.sync.getDeadlockCycle();
    CHECK_FALSE(cycle.empty());
    CHECK(h.hasEvent(EventType::DEADLOCK_DETECTED));
}

TEST_CASE("T-SYNC-013: No deadlock when no cycle") {
    SyncHarness h;
    h.sync.createMutex("M1");
    h.sync.createMutex("M2");

    CHECK(h.sync.lockMutex("M1", 1));
    CHECK(h.sync.lockMutex("M2", 2));

    // P1 waits for M2, P2 holds M2 but doesn't wait for M1
    CHECK_FALSE(h.sync.lockMutex("M2", 1)); // blocked

    CHECK_FALSE(h.sync.detectDeadlock());
}

// ============================================================================
// Integration with ProcessManager Tests
// ============================================================================

TEST_CASE("T-SYNC-014: Process blocks on mutex via ProcessManager") {
    SyncHarness h;
    h.sync.createMutex("M1");

    // P1 acquires
    CHECK(h.sync.lockMutex("M1", 1));
    // P1 would be RUNNING in real OS

    // P2 tries to acquire, gets blocked
    CHECK_FALSE(h.sync.lockMutex("M1", 2));

    // In real OS, ProcessManager would call blockProcess(2)
    // Simulate: P2 goes WAITING
    CHECK(h.pm.blockProcess(2));
    auto p2 = h.pm.getProcess(2);
    CHECK(p2 != nullptr);
    CHECK(p2->state == ProcessState::WAITING);

    // P1 releases, P2 woken
    int next = h.sync.unlockMutex("M1", 1);
    CHECK(next == 2);

    // ProcessManager would call wakeProcess(2)
    CHECK(h.pm.wakeProcess(2));
    CHECK(p2->state == ProcessState::READY);
}

TEST_CASE("T-SYNC-015: Process blocks on semaphore via ProcessManager") {
    SyncHarness h;
    h.sync.createSemaphore("S1", 1);

    CHECK(h.sync.waitSemaphore("S1", 1));  // acquired
    CHECK_FALSE(h.sync.waitSemaphore("S1", 2)); // blocked

    CHECK(h.pm.blockProcess(2));
    auto p2 = h.pm.getProcess(2);
    CHECK(p2->state == ProcessState::WAITING);

    int next = h.sync.signalSemaphore("S1");
    CHECK(next == 2);

    CHECK(h.pm.wakeProcess(2));
    CHECK(p2->state == ProcessState::READY);
}

// ============================================================================
// Race Condition Demo Tests
// ============================================================================

TEST_CASE("T-SYNC-016: Race condition without mutex") {
    SyncHarness h;
    // Shared counter in simulated memory
    // We'll simulate by having two processes increment a shared value
    // without mutex - this demonstrates the race condition conceptually

    int sharedCounter = 0;

    // Process 1: read, increment, write
    int val1 = sharedCounter;
    val1++;
    sharedCounter = val1;

    // Process 2: read (same stale value), increment, write
    int val2 = sharedCounter - 1; // stale read if no mutex
    val2++;
    sharedCounter = val2;

    // Without mutex: final value = 1 (lost update)
    // With mutex: would be 2
    CHECK(sharedCounter == 1); // Race condition!
}

TEST_CASE("T-SYNC-017: No race condition with mutex") {
    SyncHarness h;
    h.sync.createMutex("M1");

    int sharedCounter = 0;

    auto incrementWithMutex = [&](int pid) {
        // Lock
        bool acquired = h.sync.lockMutex("M1", pid);
        CHECK(acquired);

        // Critical section
        int val = sharedCounter;
        val++;
        sharedCounter = val;

        // Unlock
        int next = h.sync.unlockMutex("M1", pid);
        CHECK(next == INVALID_PID);
    };

    // P1 increments
    incrementWithMutex(1);
    // P2 increments
    incrementWithMutex(2);

    // With mutex: final value = 2 (correct)
    CHECK(sharedCounter == 2);
}

// ============================================================================
// Event Logging Tests
// ============================================================================

TEST_CASE("T-SYNC-018: Events recorded for mutex operations") {
    SyncHarness h;
    h.sync.createMutex("M1");
    h.sync.lockMutex("M1", 1);
    h.sync.unlockMutex("M1", 1);

    CHECK(h.eventCount(EventType::MUTEX_CREATED) == 1);
    CHECK(h.eventCount(EventType::MUTEX_LOCKED) == 1);
    CHECK(h.eventCount(EventType::MUTEX_UNLOCKED) == 1);
}

TEST_CASE("T-SYNC-019: Events recorded for semaphore operations") {
    SyncHarness h;
    h.sync.createSemaphore("S1", 1);
    h.sync.waitSemaphore("S1", 1);
    h.sync.signalSemaphore("S1");

    CHECK(h.eventCount(EventType::SEMAPHORE_CREATED) == 1);
    CHECK(h.eventCount(EventType::SEMAPHORE_WAIT) == 1);
    CHECK(h.eventCount(EventType::SEMAPHORE_SIGNAL) == 1);
}

TEST_CASE("T-SYNC-020: State listing for dashboard") {
    SyncHarness h;
    h.sync.createMutex("M1");
    h.sync.createMutex("M2");
    h.sync.createSemaphore("S1", 2);

    auto mutexes = h.sync.listMutexes();
    CHECK(mutexes.size() == 2);
    auto hasId = [](const std::vector<std::string>& v, const std::string& id) {
        for (const auto& s : v) {
            if (s == id) return true;
        }
        return false;
    };
    CHECK(hasId(mutexes, "M1"));
    CHECK(hasId(mutexes, "M2"));

    auto semaphores = h.sync.listSemaphores();
    CHECK(semaphores.size() == 1);
    CHECK(semaphores[0] == "S1");

    auto m1 = h.sync.getMutex("M1");
    CHECK(m1.has_value());
    CHECK(m1->id() == "M1");

    auto s1 = h.sync.getSemaphore("S1");
    CHECK(s1.has_value());
    CHECK(s1->id() == "S1");
    CHECK(s1->count() == 2);
}

// ============================================================================
// Reset Tests
// ============================================================================

TEST_CASE("T-SYNC-021: Reset clears all state") {
    SyncHarness h;
    h.sync.createMutex("M1");
    h.sync.createSemaphore("S1", 3);
    h.sync.lockMutex("M1", 1);
    h.sync.waitSemaphore("S1", 2);

    h.sync.reset();

    CHECK(h.sync.mutexCount() == 0);
    CHECK(h.sync.semaphoreCount() == 0);
}