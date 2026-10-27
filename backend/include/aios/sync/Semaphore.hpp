#pragma once

#include <aios/core/types.hpp>

#include <deque>
#include <string>

namespace aios {

// ---------------------------------------------------------------------------
// Semaphore (docs/09 sections 12-14, 26-27).
// Counting semaphore for coordinating access to a limited number of resource
// units. No strict ownership — any process can wait/signal.
// ---------------------------------------------------------------------------
class Semaphore {
public:
    Semaphore(std::string id = "", int initialCount = 0)
        : id_(std::move(id)), count_(initialCount) {}

    // Unique semaphore identifier (e.g., "S1", "io_slots").
    const std::string& id() const { return id_; }

    // Current available count (can be negative when waiters exceed resources).
    int count() const { return count_; }

    // Copy of the waiting queue (FIFO order).
    std::deque<int> waitingQueue() const { return waiting_; }

    // WAIT operation (docs/09 section 13).
    // Decrements count. If count >= 0 after decrement, returns true (acquired).
    // If count < 0, the caller should be blocked and enqueued as a waiter.
    bool wait(int pid) {
        --count_;
        if (count_ >= 0) {
            return true; // acquired immediately
        }
        waiting_.push_back(pid);
        return false; // blocked
    }

    // SIGNAL operation (docs/09 section 13).
    // Increments count. If there are waiters, wakes the first one (FIFO).
    // Returns the woken PID, or INVALID_PID if no one was waiting.
    int signal() {
        ++count_;
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
        std::string s = "Semaphore[" + id_ + "] count=" + std::to_string(count_);
        if (!waiting_.empty()) {
            s += " waiters=" + std::to_string(waiting_.size());
        }
        return s;
    }

private:
    std::string id_;
    int count_ = 0;
    std::deque<int> waiting_;
};

} // namespace aios