#include <aios/core/EventLog.hpp>
#include <aios/core/SimulationClock.hpp>
#include <aios/core/types.hpp>
#include <aios/ipc/IPCManager.hpp>
#include <aios/process/ProcessManager.hpp>

#include <doctest/doctest.h>

#include <sstream>
#include <vector>

using namespace aios;

namespace {

// Minimal harness for IPC tests
struct IPCHarness {
    EventLog log;
    SimulationClock clock;
    ProcessManager pm;
    IPCManager ipc;

    IPCHarness() {
        pm.setEventLog(&log);
        pm.setClock(&clock);
        ipc.setEventLog(&log);
        ipc.setProcessManager(&pm);
        ipc.setClock(&clock);
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
// Channel/Queue Management Tests
// ============================================================================

TEST_CASE("T-IPC-001: Channel creation") {
    IPCHarness h;
    CHECK(h.ipc.createChannel(1));
    CHECK_FALSE(h.ipc.createChannel(1)); // duplicate
    CHECK(h.hasEvent(EventType::IPC_CHANNEL_CREATED));
}

TEST_CASE("T-IPC-002: Channel get/close") {
    IPCHarness h;
    h.ipc.createChannel(1);
    auto ch = h.ipc.getChannel(1);
    CHECK(ch.has_value());
    CHECK(ch.value() == "Q1");
    CHECK(h.ipc.closeChannel(1));
    CHECK_FALSE(h.ipc.closeChannel(1)); // already closed
}

TEST_CASE("T-IPC-003: Auto-create channel on send") {
    IPCHarness h;
    // Send to non-existent channel - should auto-create
    int msgId = h.ipc.sendMessage(1, 2, "TEST", "hello");
    CHECK(msgId != INVALID_PID);
    CHECK(h.ipc.hasMessage(2));
}

// ============================================================================
// Send/Receive Tests
// ============================================================================

TEST_CASE("T-IPC-004: Basic send/receive") {
    IPCHarness h;
    h.ipc.createChannel(2);

    int msgId = h.ipc.sendMessage(1, 2, "TASK_REQUEST", "process this");
    CHECK(msgId != INVALID_PID);

    auto msg = h.ipc.receiveMessage(2);
    CHECK(msg.has_value());
    CHECK(msg->id == msgId);
    CHECK(msg->senderPid == 1);
    CHECK(msg->receiverPid == 2);
    CHECK(msg->type == "TASK_REQUEST");
    CHECK(msg->payload == "process this");

    CHECK(h.hasEvent(EventType::IPC_SEND));
    CHECK(h.hasEvent(EventType::IPC_RECEIVE));
    CHECK(h.hasEvent(EventType::IPC_MESSAGE_QUEUED));
    CHECK(h.hasEvent(EventType::IPC_MESSAGE_DELIVERED));
}

TEST_CASE("T-IPC-005: Blocking receive when empty") {
    IPCHarness h;
    h.ipc.createChannel(2);

    // Non-blocking receive on empty queue
    auto msg = h.ipc.receiveMessage(2, false);
    CHECK_FALSE(msg.has_value());

    // Blocking receive on empty queue (caller should block process)
    msg = h.ipc.receiveMessage(2, true);
    CHECK_FALSE(msg.has_value());
}

TEST_CASE("T-IPC-006: FIFO ordering") {
    IPCHarness h;
    h.ipc.createChannel(2);

    h.ipc.sendMessage(1, 2, "MSG", "first");
    h.ipc.sendMessage(1, 2, "MSG", "second");
    h.ipc.sendMessage(1, 2, "MSG", "third");

    auto m1 = h.ipc.receiveMessage(2);
    auto m2 = h.ipc.receiveMessage(2);
    auto m3 = h.ipc.receiveMessage(2);

    CHECK(m1.has_value());
    CHECK(m2.has_value());
    CHECK(m3.has_value());
    CHECK(m1->payload == "first");
    CHECK(m2->payload == "second");
    CHECK(m3->payload == "third");
}

TEST_CASE("T-IPC-007: Queue capacity limit") {
    IPCHarness h;
    h.ipc.createChannel(2, 2); // capacity 2

    CHECK(h.ipc.sendMessage(1, 2, "MSG", "1") != INVALID_PID);
    CHECK(h.ipc.sendMessage(1, 2, "MSG", "2") != INVALID_PID);
    CHECK(h.ipc.sendMessage(1, 2, "MSG", "3") == INVALID_PID); // queue full

    CHECK(h.hasEvent(EventType::IPC_QUEUE_FULL));
}

TEST_CASE("T-IPC-008: Has message check") {
    IPCHarness h;
    CHECK_FALSE(h.ipc.hasMessage(2));

    h.ipc.sendMessage(1, 2, "MSG", "test");
    CHECK(h.ipc.hasMessage(2));

    h.ipc.receiveMessage(2);
    CHECK_FALSE(h.ipc.hasMessage(2));
}

// ============================================================================
// Permissions Tests
// ============================================================================

TEST_CASE("T-IPC-009: Permission grant/revoke") {
    IPCHarness h;
    h.ipc.createChannel(2);

    // Default: allow all
    CHECK(h.ipc.checkPermission(1, 2));
    CHECK(h.ipc.sendMessage(1, 2, "MSG", "test") != INVALID_PID);

    // Revoke permission
    h.ipc.revokePermission(1, 2);
    CHECK_FALSE(h.ipc.checkPermission(1, 2));
    CHECK(h.ipc.sendMessage(1, 2, "MSG", "test") == INVALID_PID);
    CHECK(h.hasEvent(EventType::IPC_PERMISSION_DENIED));

    // Grant permission back
    h.ipc.grantPermission(1, 2);
    CHECK(h.ipc.checkPermission(1, 2));
    CHECK(h.ipc.sendMessage(1, 2, "MSG", "test") != INVALID_PID);
}

TEST_CASE("T-IPC-010: Permission only affects sender->receiver") {
    IPCHarness h;
    h.ipc.createChannel(2);
    h.ipc.createChannel(3);

    h.ipc.revokePermission(1, 2);
    CHECK_FALSE(h.ipc.checkPermission(1, 2));
    CHECK(h.ipc.checkPermission(1, 3)); // different receiver still allowed
    CHECK(h.ipc.checkPermission(4, 2)); // different sender still allowed
}

// ============================================================================
// Agent Communication Tests
// ============================================================================

TEST_CASE("T-IPC-011: Research -> Coding agent communication") {
    IPCHarness h;
    // Simulate Research Agent (PID 1) and Coding Agent (PID 2)
    h.ipc.createChannel(2);
    h.ipc.grantPermission(1, 2);
    h.ipc.grantPermission(2, 1);

    // Research agent sends result to Coding agent
    int msgId = h.ipc.sendMessage(1, 2, "RESEARCH_RESULT", "dataset analysis complete");
    CHECK(msgId != INVALID_PID);

    auto msg = h.ipc.receiveMessage(2);
    CHECK(msg.has_value());
    CHECK(msg->senderPid == 1);
    CHECK(msg->receiverPid == 2);
    CHECK(msg->type == "RESEARCH_RESULT");
    CHECK(msg->payload == "dataset analysis complete");
}

TEST_CASE("T-IPC-012: Task request/response pattern") {
    IPCHarness h;
    h.ipc.createChannel(2);
    h.ipc.grantPermission(1, 2);
    h.ipc.grantPermission(2, 1);

    // P1 sends task request
    h.ipc.sendMessage(1, 2, "TASK_REQUEST", "analyze dataset X");

    // P2 receives and responds
    auto req = h.ipc.receiveMessage(2);
    CHECK(req.has_value());
    CHECK(req->type == "TASK_REQUEST");

    h.ipc.sendMessage(2, 1, "TASK_RESULT", "analysis complete");

    auto resp = h.ipc.receiveMessage(1);
    CHECK(resp.has_value());
    CHECK(resp->type == "TASK_RESULT");
}

// ============================================================================
// Process Cleanup Tests
// ============================================================================

TEST_CASE("T-IPC-013: Cleanup on process termination") {
    IPCHarness h;
    h.ipc.createChannel(2);
    h.ipc.grantPermission(1, 2);
    h.ipc.grantPermission(2, 1);

    h.ipc.sendMessage(1, 2, "MSG", "hello");
    h.ipc.sendMessage(2, 1, "MSG", "world");

    // Process 1 terminates
    h.ipc.cleanupProcess(1);

    // Messages for P1 should be removed
    CHECK_FALSE(h.ipc.hasMessage(1));
    CHECK(h.ipc.hasMessage(2)); // P2's channel still has messages

    // Permissions for P1 should be removed
    CHECK_FALSE(h.ipc.checkPermission(1, 2));
}

TEST_CASE("T-IPC-014: Channel removed on cleanup") {
    IPCHarness h;
    h.ipc.createChannel(1);

    h.ipc.cleanupProcess(1);
    CHECK_FALSE(h.ipc.getChannel(1).has_value());
}

// ============================================================================
// Statistics Tests
// ============================================================================

TEST_CASE("T-IPC-015: Statistics tracking") {
    IPCHarness h;
    h.ipc.createChannel(2);

    h.ipc.sendMessage(1, 2, "MSG", "a");
    h.ipc.sendMessage(1, 2, "MSG", "b");
    h.ipc.receiveMessage(2);
    h.ipc.receiveMessage(2);

    auto stats = h.ipc.getStatistics();
    CHECK(stats.messagesSent == 2);
    CHECK(stats.messagesReceived == 2);
}

TEST_CASE("T-IPC-016: Per-process statistics") {
    IPCHarness h;
    h.ipc.createChannel(2);
    h.ipc.sendMessage(1, 2, "MSG", "test");
    h.ipc.sendMessage(2, 1, "MSG", "reply");

    auto ps1 = h.ipc.getProcessStatistics(1);
    CHECK(ps1.messagesSent == 1);
    CHECK(ps1.messagesReceived == 1);

    auto ps2 = h.ipc.getProcessStatistics(2);
    CHECK(ps2.messagesSent == 1);
    CHECK(ps2.messagesReceived == 1);
}

// ============================================================================
// Reset Tests
// ============================================================================

TEST_CASE("T-IPC-017: Reset clears all state") {
    IPCHarness h;
    h.ipc.createChannel(1);
    h.ipc.grantPermission(1, 2);
    h.ipc.sendMessage(1, 2, "MSG", "test");

    h.ipc.reset();

    CHECK_FALSE(h.ipc.getChannel(1).has_value());
    CHECK_FALSE(h.ipc.checkPermission(1, 2));
    auto stats = h.ipc.getStatistics();
    CHECK(stats.messagesSent == 0);
    CHECK(stats.messagesReceived == 0);
}

// ============================================================================
// Message Structure Tests
// ============================================================================

TEST_CASE("T-IPC-018: Message structure integrity") {
    IPCHarness h;
    h.ipc.createChannel(2);
    h.ipc.sendMessage(1, 2, "TASK_REQUEST", "process this");

    auto msg = h.ipc.receiveMessage(2);
    CHECK(msg.has_value());
    CHECK(msg->id > 0);
    CHECK(msg->senderPid == 1);
    CHECK(msg->receiverPid == 2);
    CHECK(msg->type == "TASK_REQUEST");
    CHECK(msg->payload == "process this");
    CHECK(msg->timestamp >= 0);
}

// ============================================================================
// Multiple Senders Tests
// ============================================================================

TEST_CASE("T-IPC-019: Multiple senders to same receiver") {
    IPCHarness h;
    h.ipc.createChannel(3);
    h.ipc.grantPermission(1, 3);
    h.ipc.grantPermission(2, 3);

    h.ipc.sendMessage(1, 3, "DATA", "from P1");
    h.ipc.sendMessage(2, 3, "DATA", "from P2");

    auto m1 = h.ipc.receiveMessage(3);
    auto m2 = h.ipc.receiveMessage(3);

    CHECK(m1.has_value());
    CHECK(m2.has_value());
    std::vector<std::string> senders = {std::to_string(m1->senderPid), std::to_string(m2->senderPid)};
    CHECK(std::find(senders.begin(), senders.end(), "1") != senders.end());
    CHECK(std::find(senders.begin(), senders.end(), "2") != senders.end());
}

// ============================================================================
// Message Types Tests
// ============================================================================

TEST_CASE("T-IPC-020: Different message types") {
    IPCHarness h;
    h.ipc.createChannel(2);
    h.ipc.grantPermission(1, 2);

    h.ipc.sendMessage(1, 2, "TASK_REQUEST", "do work");
    h.ipc.sendMessage(1, 2, "TASK_RESULT", "done");
    h.ipc.sendMessage(1, 2, "DATA", "raw data");
    h.ipc.sendMessage(1, 2, "STATUS", "running");
    h.ipc.sendMessage(1, 2, "NOTIFICATION", "alert");
    h.ipc.sendMessage(1, 2, "ERROR", "failed");
    h.ipc.sendMessage(1, 2, "CONTROL", "pause");

    auto types = {"TASK_REQUEST", "TASK_RESULT", "DATA", "STATUS", "NOTIFICATION", "ERROR", "CONTROL"};
    for (const auto& t : types) {
        auto msg = h.ipc.receiveMessage(2);
        CHECK(msg.has_value());
        CHECK(msg->type == t);
    }
}

// ============================================================================
// Dashboard/API Tests
// ============================================================================

TEST_CASE("T-IPC-021: Get messages for dashboard") {
    IPCHarness h;
    h.ipc.createChannel(2);
    h.ipc.sendMessage(1, 2, "MSG", "a");
    h.ipc.sendMessage(1, 2, "MSG", "b");

    auto msgs = h.ipc.getMessages(2);
    CHECK(msgs.size() == 2);
    CHECK(msgs[0].payload == "a");
    CHECK(msgs[1].payload == "b");
}

TEST_CASE("T-IPC-022: Channel listing") {
    IPCHarness h;
    h.ipc.createChannel(1);
    h.ipc.createChannel(2);

    // Note: IPCManager doesn't have listChannels() yet, but we can verify channels exist
    CHECK(h.ipc.getChannel(1).has_value());
    CHECK(h.ipc.getChannel(2).has_value());
}