#pragma once

#include <aios/core/EventLog.hpp>
#include <aios/core/types.hpp>
#include <aios/ipc/MessageQueue.hpp>

#include <map>
#include <optional>
#include <string>
#include <vector>

namespace aios {

class ProcessManager;
class SimulationClock;

// ---------------------------------------------------------------------------
// IPC Manager (docs/10 sections 35, 37).
// Central communication layer for message passing between processes and AI agents.
// Manages message queues, send/receive operations, permissions, and statistics.
// ---------------------------------------------------------------------------
class IPCManager {
public:
    IPCManager() = default;

    void setEventLog(EventLog* log) { eventLog_ = log; }
    void setProcessManager(ProcessManager* pm) { processes_ = pm; }
    void setClock(SimulationClock* clock) { clock_ = clock; }

    // ----- Channel/Queue Management (docs/10 section 37) -----

    // Creates a communication channel (message queue) for `receiverPid`.
    // Returns false if channel already exists.
    bool createChannel(int receiverPid, size_t capacity = 10);

    // Returns the channel ID for a receiver.
    std::optional<std::string> getChannel(int receiverPid) const;

    // Closes a channel and removes all its messages.
    bool closeChannel(int receiverPid);

    // ----- Send/Receive Operations (docs/10 sections 10-13, 37) -----

    // Sends a message from `senderPid` to `receiverPid`.
    // If `receiverPid` has no channel, one is created automatically.
    // Returns: message ID on success, INVALID_PID on failure (queue full, etc.).
    int sendMessage(int senderPid, int receiverPid, const std::string& type,
                    const std::string& payload);

    // Receives a message for `receiverPid`.
    // If `blocking` is true and queue is empty, returns nullopt (caller should block process).
    // If `blocking` is false, returns nullopt immediately when empty.
    // On success, returns the message.
    std::optional<IPCMessage> receiveMessage(int receiverPid, bool blocking = true);

    // Non-blocking check for pending messages.
    bool hasMessage(int receiverPid) const;

    // Returns all pending messages for a receiver (for dashboard).
    std::vector<IPCMessage> getMessages(int receiverPid) const;

    // ----- Permissions (docs/10 section 27) -----

    // Grants send permission from `senderPid` to `receiverPid`.
    void grantPermission(int senderPid, int receiverPid);

    // Revokes send permission.
    void revokePermission(int senderPid, int receiverPid);

    // Checks if `senderPid` is allowed to send to `receiverPid`.
    bool checkPermission(int senderPid, int receiverPid) const;

    // ----- Statistics (docs/10 section 39) -----

    struct IPCStatistics {
        uint64_t messagesSent = 0;
        uint64_t messagesReceived = 0;
        uint64_t messagesPending = 0;
        uint64_t failedSends = 0;
        uint64_t permissionDenials = 0;
    };

    IPCStatistics getStatistics() const { return stats_; }
    IPCStatistics getProcessStatistics(int pid) const;

    // ----- Cleanup -----

    // Removes all messages for a terminated/failed process.
    void cleanupProcess(int pid);

    void reset();

private:
    void record(EventType type, int pid, const std::string& detail);
    int nextMessageId() { return nextMsgId_++; }

    std::map<int, MessageQueue> channels_; // receiverPid -> queue
    std::map<int, std::vector<int>> allowList_; // senderPid -> list of allowed receiverPids
    std::map<int, std::vector<int>> denyList_;  // senderPid -> list of denied receiverPids
    IPCStatistics stats_{};
    int nextMsgId_ = 1;

    EventLog* eventLog_ = nullptr;
    ProcessManager* processes_ = nullptr;
    SimulationClock* clock_ = nullptr;
};

} // namespace aios