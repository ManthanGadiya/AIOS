#include <aios/ipc/IPCManager.hpp>

#include <aios/core/SimulationClock.hpp>

#include <algorithm>
#include <sstream>

namespace aios {

bool IPCManager::createChannel(int receiverPid, size_t capacity) {
    if (channels_.find(receiverPid) != channels_.end()) {
        return false;
    }
    channels_.emplace(receiverPid, MessageQueue("Q" + std::to_string(receiverPid), capacity));
    record(EventType::IPC_CHANNEL_CREATED, receiverPid, "Channel created for PID " + std::to_string(receiverPid));
    return true;
}

std::optional<std::string> IPCManager::getChannel(int receiverPid) const {
    auto it = channels_.find(receiverPid);
    if (it != channels_.end()) {
        return it->second.id();
    }
    return std::nullopt;
}

bool IPCManager::closeChannel(int receiverPid) {
    auto it = channels_.find(receiverPid);
    if (it == channels_.end()) {
        return false;
    }
    it->second.clear();
    channels_.erase(it);
    return true;
}

int IPCManager::sendMessage(int senderPid, int receiverPid, const std::string& type,
                            const std::string& payload) {
    // Check permissions
    if (!checkPermission(senderPid, receiverPid)) {
        stats_.permissionDenials++;
        record(EventType::IPC_PERMISSION_DENIED, senderPid,
               "Permission denied: PID " + std::to_string(senderPid) + " -> " + std::to_string(receiverPid));
        return INVALID_PID;
    }

    // Ensure channel exists
    if (channels_.find(receiverPid) == channels_.end()) {
        createChannel(receiverPid);
    }

    auto& queue = channels_[receiverPid];
    IPCMessage msg;
    msg.id = nextMessageId();
    msg.senderPid = senderPid;
    msg.receiverPid = receiverPid;
    msg.type = type;
    msg.payload = payload;
    msg.timestamp = clock_ ? clock_->cycle() : 0;

    if (!queue.enqueue(msg)) {
        stats_.failedSends++;
        record(EventType::IPC_QUEUE_FULL, senderPid,
               "Queue full for PID " + std::to_string(receiverPid));
        return INVALID_PID;
    }

    stats_.messagesSent++;
    record(EventType::IPC_SEND, senderPid,
           "PID " + std::to_string(senderPid) + " -> " + std::to_string(receiverPid) + " [" + type + "]");
    record(EventType::IPC_MESSAGE_QUEUED, receiverPid,
           "Message " + std::to_string(msg.id) + " queued for PID " + std::to_string(receiverPid));

    return msg.id;
}

std::optional<IPCMessage> IPCManager::receiveMessage(int receiverPid, bool blocking) {
    auto it = channels_.find(receiverPid);
    if (it == channels_.end()) {
        if (!blocking) return std::nullopt;
        // Channel doesn't exist - in blocking mode, caller should block process
        return std::nullopt;
    }

    auto msgOpt = it->second.dequeue();
    if (!msgOpt) {
        if (!blocking) return std::nullopt;
        // Queue empty - in blocking mode, caller should block process
        return std::nullopt;
    }

    stats_.messagesReceived++;
    record(EventType::IPC_RECEIVE, receiverPid,
           "PID " + std::to_string(receiverPid) + " received from " +
           std::to_string(msgOpt->senderPid) + " [" + msgOpt->type + "]");
    record(EventType::IPC_MESSAGE_DELIVERED, receiverPid,
           "Message " + std::to_string(msgOpt->id) + " delivered to PID " + std::to_string(receiverPid));

    return msgOpt;
}

bool IPCManager::hasMessage(int receiverPid) const {
    auto it = channels_.find(receiverPid);
    if (it == channels_.end()) return false;
    return !it->second.empty();
}

std::vector<IPCMessage> IPCManager::getMessages(int receiverPid) const {
    auto it = channels_.find(receiverPid);
    if (it == channels_.end()) return {};
    return it->second.getMessagesFor(receiverPid);
}

void IPCManager::grantPermission(int senderPid, int receiverPid) {
    auto& allow = allowList_[senderPid];
    if (std::find(allow.begin(), allow.end(), receiverPid) == allow.end()) {
        allow.push_back(receiverPid);
    }
    // Also remove from deny list if present
    auto& deny = denyList_[senderPid];
    deny.erase(std::remove(deny.begin(), deny.end(), receiverPid), deny.end());
}

void IPCManager::revokePermission(int senderPid, int receiverPid) {
    // Remove from allow list if present
    auto allowIt = allowList_.find(senderPid);
    if (allowIt != allowList_.end()) {
        allowIt->second.erase(std::remove(allowIt->second.begin(), allowIt->second.end(), receiverPid), allowIt->second.end());
    }
    // Add to deny list
    auto& deny = denyList_[senderPid];
    if (std::find(deny.begin(), deny.end(), receiverPid) == deny.end()) {
        deny.push_back(receiverPid);
    }
}

bool IPCManager::checkPermission(int senderPid, int receiverPid) const {
    // Check deny list first
    auto denyIt = denyList_.find(senderPid);
    if (denyIt != denyList_.end()) {
        if (std::find(denyIt->second.begin(), denyIt->second.end(), receiverPid) != denyIt->second.end()) {
            return false; // Explicitly denied
        }
    }

    // Check allow list
    auto allowIt = allowList_.find(senderPid);
    if (allowIt != allowList_.end()) {
        // If allow list exists, only allow those in it
        return std::find(allowIt->second.begin(), allowIt->second.end(), receiverPid) != allowIt->second.end();
    }

    // Default: allow all if no explicit allow/deny configured
    return true;
}

IPCManager::IPCStatistics IPCManager::getProcessStatistics(int pid) const {
    IPCStatistics ps{};
    for (const auto& [receiverPid, queue] : channels_) {
        for (const auto& msg : queue.getMessagesFor(receiverPid)) {
            if (msg.senderPid == pid) ps.messagesSent++;
            if (msg.receiverPid == pid) ps.messagesReceived++;
        }
    }
    // Note: pending messages not tracked per-process in this simplified version
    return ps;
}

void IPCManager::cleanupProcess(int pid) {
    // Remove all messages addressed TO this process (receiverPid == pid)
    // Messages sent BY this process are preserved in other processes' channels
    for (auto& [receiverPid, queue] : channels_) {
        queue.removeMessagesFor(pid);
    }
    // Remove permissions involving this process
    allowList_.erase(pid);
    denyList_.erase(pid);
    for (auto& [senderPid, list] : allowList_) {
        list.erase(std::remove(list.begin(), list.end(), pid), list.end());
    }
    for (auto& [senderPid, list] : denyList_) {
        list.erase(std::remove(list.begin(), list.end(), pid), list.end());
    }
    // Add deny entries for the terminated sender to prevent further sends
    // This ensures terminated processes cannot send messages
    for (const auto& [receiverPid, _] : channels_) {
        if (receiverPid != pid) {
            auto& deny = denyList_[pid];
            if (std::find(deny.begin(), deny.end(), receiverPid) == deny.end()) {
                deny.push_back(receiverPid);
            }
        }
    }
    // Remove channel if it belongs to this process
    channels_.erase(pid);
}

void IPCManager::reset() {
    channels_.clear();
    allowList_.clear();
    denyList_.clear();
    stats_ = IPCStatistics{};
    nextMsgId_ = 1;
}

void IPCManager::record(EventType type, int pid, const std::string& detail) {
    if (eventLog_ && clock_) {
        eventLog_->record(type, pid, clock_->cycle(), detail);
    }
}

} // namespace aios