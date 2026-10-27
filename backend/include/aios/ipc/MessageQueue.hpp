#pragma once

#include <aios/core/types.hpp>

#include <algorithm>
#include <deque>
#include <optional>
#include <string>
#include <vector>

namespace aios {

// ---------------------------------------------------------------------------
// IPC Message (docs/10 section 6).
// ---------------------------------------------------------------------------
struct IPCMessage {
    int id = 0;
    int senderPid = INVALID_PID;
    int receiverPid = INVALID_PID;
    std::string type;        // e.g., "TASK_REQUEST", "TASK_RESULT", "DATA"
    std::string payload;     // message content
    uint64_t timestamp = 0;  // simulation cycle when sent

    std::string toString() const;
};

// ---------------------------------------------------------------------------
// Message Queue (docs/10 sections 8-9, 34).
// FIFO queue with capacity limit. Messages addressed to a specific receiver
// or channel. Supports blocking/non-blocking receive.
// ---------------------------------------------------------------------------
class MessageQueue {
public:
    MessageQueue() = default;
    explicit MessageQueue(std::string id, size_t capacity = 10)
        : id_(std::move(id)), capacity_(capacity) {}

    const std::string& id() const { return id_; }
    size_t capacity() const { return capacity_; }
    size_t size() const { return messages_.size(); }
    bool empty() const { return messages_.empty(); }
    bool full() const { return messages_.size() >= capacity_; }

    // Enqueues a message. Returns false if queue is full.
    bool enqueue(const IPCMessage& msg) {
        if (full()) return false;
        messages_.push_back(msg);
        return true;
    }

    // Dequeues the oldest message. Returns nullopt if empty.
    std::optional<IPCMessage> dequeue() {
        if (messages_.empty()) return std::nullopt;
        IPCMessage msg = messages_.front();
        messages_.pop_front();
        return msg;
    }

    // Peeks at the oldest message without removing it.
    std::optional<IPCMessage> peek() const {
        if (messages_.empty()) return std::nullopt;
        return messages_.front();
    }

    // Returns all messages for a specific receiver (for dashboard).
    std::vector<IPCMessage> getMessagesFor(int receiverPid) const {
        std::vector<IPCMessage> result;
        for (const auto& msg : messages_) {
            if (msg.receiverPid == receiverPid || msg.receiverPid == -1) {
                result.push_back(msg);
            }
        }
        return result;
    }

    // Removes all messages addressed TO a terminated process (receiverPid == pid).
    // Messages sent BY the process are preserved.
    void removeMessagesFor(int pid) {
        messages_.erase(
            std::remove_if(messages_.begin(), messages_.end(),
                           [pid](const IPCMessage& m) { return m.receiverPid == pid; }),
            messages_.end());
    }

    void clear() { messages_.clear(); }

private:
    std::string id_;
    size_t capacity_ = 10;
    std::deque<IPCMessage> messages_;
};

} // namespace aios