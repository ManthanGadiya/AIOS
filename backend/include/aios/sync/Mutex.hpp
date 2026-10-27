#pragma once

#include <aios/core/types.hpp>

#include <deque>
#include <string>

namespace aios {

// ---------------------------------------------------------------------------
// Mutex (docs/09 sections 8-11, 26-27).
// Provides mutual exclusion for a critical section. Binary lock with ownership
// and a FIFO waiting queue of blocked PIDs.
// ---------------------------------------------------------------------------
class Mutex {
public:
    explicit Mutex(std::string id = "") : id_(std::move(id)) {}

    // Unique mutex identifier (e.g., "M1", "knowledge_bus").
    const std::string& id() const { return id_; }

    // Current state.
    bool isLocked() const { return locked_; }

    // Owning process PID, or INVALID_PID if unlocked.
    int ownerPid() const { return owner_; }

    // Copy of the waiting queue (FIFO order).
    std::deque<int> waitingQueue() const { return waiting_; }

    // Attempts to lock the mutex for `pid`.
    // Returns true on success (mutex was free, now owned by pid).
    // Returns false if already locked (caller should block the process).
    bool tryLock(int pid) {
        if (!locked_) {
            locked_ = true;
            owner_ = pid;
            return true;
        }
        return false;
    }

    // Enqueues `pid` as a waiter (used when tryLock fails).
    void enqueueWaiter(int pid) {
        waiting_.push_back(pid);
    }

    // Unlocks the mutex. Caller must be the owner.
    // Returns the next waiter PID to wake (or INVALID_PID if none).
    int unlock(int pid) {
        if (!locked_ || owner_ != pid) {
            return INVALID_PID; // error: not owner or already unlocked
        }
        locked_ = false;
        owner_ = INVALID_PID;
        if (!waiting_.empty()) {
            int next = waiting_.front();
            waiting_.pop_front();
            // The next waiter will be granted the lock by the manager
            // after being woken and transitioned to READY.
            return next;
        }
        return INVALID_PID;
    }

    // Force-release (e.g., on process termination/crash).
    // Returns the next waiter PID to wake, if any.
    int forceRelease() {
        if (!locked_) return INVALID_PID;
        locked_ = false;
        owner_ = INVALID_PID;
        if (!waiting_.empty()) {
            int next = waiting_.front();
            waiting_.pop_front();
            return next;
        }
        return INVALID_PID;
    }

    // Removes a specific PID from the waiting queue (e.g., on process termination).
    void removeWaiter(int pid) {
        for (auto it = waiting_.begin(); it != waiting_.end(); ++it) {
            if (*it == pid) {
                waiting_.erase(it);
                break;
            }
        }
    }

    std::string toString() const {
        std::string s = "Mutex[" + id_ + "] " + (locked_ ? "LOCKED" : "FREE");
        if (locked_) s += " owner=" + std::to_string(owner_);
        if (!waiting_.empty()) {
            s += " waiters=" + std::to_string(waiting_.size());
        }
        return s;
    }

private:
    std::string id_;
    bool locked_ = false;
    int owner_ = INVALID_PID;
    std::deque<int> waiting_;
};

} // namespace aios