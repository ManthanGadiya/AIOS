#include <aios/core/EventLog.hpp>
#include <aios/core/SimulationClock.hpp>
#include <aios/memory/Memory.hpp>
#include <aios/process/ProcessManager.hpp>
#include <aios/server/ApiServer.hpp>

#include <doctest/doctest.h>

#include <algorithm>
#include <chrono>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

using namespace aios;

// Regression for the "engine wedges on page load" bug: the dashboard opens
// ws://host/ws on every load. cpp-httplib invokes pre_routing_handler_ once
// before WebSocket upgrade negotiation and again inside routing() when no
// native WS handler matches, so the engine-mutex lock() in that hook
// self-deadlocked the connection thread and the mutex stayed held forever:
// every later REST call blocked (dashboard showed Disconnected) and Ctrl+C
// hung in ApiServer::stop() -> join(). The hooks now skip upgrades entirely
// and guard re-entry per thread.
TEST_CASE("T-SRV-001: websocket upgrade attempt does not wedge REST or shutdown") {
    Memory memory;
    EventLog log;
    SimulationClock clock;
    ProcessManager pm;
    pm.setMemory(&memory);
    pm.setEventLog(&log);
    pm.setClock(&clock);

    std::mutex engineMutex;
    ApiServer server(18917);
    server.setEngineMutex(&engineMutex);
    server.setProcessManager(&pm);
    server.setEventLog(&log);
    server.setClock(&clock);
    server.start();
    REQUIRE(server.ok());

    httplib::Client cli("127.0.0.1", 18917);
    cli.set_connection_timeout(2, 0);
    cli.set_read_timeout(2, 0);

    bool ready = false;
    for (int i = 0; i < 50 && !ready; ++i) {
        if (auto r = cli.Get("/health"); r && r->status == 200) {
            ready = true;
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    REQUIRE(ready);

    // The dashboard's WS handshake on page load. No native WS handler is
    // registered, so this request traverses the routing hooks twice.
    httplib::Headers wsHeaders{
        {"Upgrade", "websocket"},
        {"Connection", "Upgrade"},
        {"Sec-WebSocket-Key", "dGhlIHNhbXBsZSBub25jZQ=="},
        {"Sec-WebSocket-Version", "13"}};
    auto wsRes = cli.Get("/ws", wsHeaders);
    REQUIRE(wsRes);
    CHECK(wsRes->status == 426); // Upgrade Required; polling fallback documented

    // REST must still answer afterwards. Before the fix this blocked on the
    // engine mutex the self-deadlocked WS thread kept holding.
    auto stats = cli.Get("/api/statistics");
    REQUIRE(stats);
    CHECK(stats->status == 200);
    CHECK(stats->body.find("totalProcesses") != std::string::npos);

    // Shutdown must complete: before the fix server_.stop() + join() hung on
    // the wedged thread. A regression here hangs the whole test binary.
    server.stop();
}

// Regression for "extra processes loop among themselves and allocate more
// memory": pids are monotonic and never reused, but statisticsToJson,
// memoryToJson page tables, ipcToJson, /api/processes, /api/agents and the
// SET_PROCESS_COUNT alive-count all scanned a hardcoded pid range 0..255.
// Once the server had created more than 255 PCBs (slider drags fire one
// command per intermediate value), new processes became invisible to every
// endpoint: the count command could no longer see or terminate them, so each
// subsequent request piled on more invisible processes that kept consuming
// frames until memory saturates and the FIFO replacer thrashes.
TEST_CASE("T-SRV-002: process listing and count command stay correct past pid 255") {
    Memory memory;
    EventLog log;
    SimulationClock clock;
MemoryManager memManager;
    memManager.setMemory(&memory);
    memManager.initialize(16, 1024);  // plenty of frames for 260 processes
    memManager.setEventLog(&log);
    memManager.setClock(&clock);

    CPU cpu;
    cpu.setMemory(&memory);
    cpu.setMemoryManager(&memManager);
    cpu.setEventLog(&log);
    cpu.setClock(&clock);

    ProcessManager pm;
    pm.setMemory(&memory);
    pm.setMemoryManager(&memManager);
    pm.setEventLog(&log);
    pm.setCPU(&cpu);
    pm.setClock(&clock);

    Scheduler sched;
    sched.setProcessManager(&pm);
    sched.setEventLog(&log);
    sched.setClock(&clock);
    sched.setPolicy(SchedulingPolicy::ROUND_ROBIN);
    sched.setTimeQuantum(4);

    std::mutex engineMutex;
    ApiServer server(18918);
    server.setEngineMutex(&engineMutex);
    server.setProcessManager(&pm);
    server.setMemory(&memory);
    server.setMemoryManager(&memManager);
    server.setCPU(&cpu);
    server.setEventLog(&log);
    server.setClock(&clock);
    server.setScheduler(&sched);
    server.start();
    REQUIRE(server.ok());

    httplib::Client cli("127.0.0.1", 18918);
    cli.set_connection_timeout(2, 0);
    cli.set_read_timeout(2, 0);
    bool ready = false;
    for (int i = 0; i < 50 && !ready; ++i) {
        if (auto r = cli.Get("/health"); r && r->status == 200) {
            ready = true;
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    REQUIRE(ready);

    // Fill the pid space past the old 0..255 scanning window.
    const std::vector<int32_t> haltProgram{11}; // HALT
    int created = 0;
    int lastPid = INVALID_PID;
    for (int i = 0; i < 260; ++i) {
        lastPid = pm.createProcess("Bulk" + std::to_string(i), 1, haltProgram, 0,
                                   ProcessType::NORMAL);
        if (lastPid != INVALID_PID) ++created;
    }
    REQUIRE(created == 260);
    REQUIRE(lastPid >= 260);

    // Admit all so they can be terminated later (NEW -> TERMINATED is
    // disallowed; only RUNNING/WAITING/READY -> TERMINATED is valid).
    for (const int pid : pm.processIds()) {
        CHECK(pm.admit(pid));
    }

    // /api/processes must list every process, not just pids below 256.
    auto procsRes = cli.Get("/api/processes");
    REQUIRE(procsRes);
    REQUIRE(procsRes->status == 200);
    auto procs = nlohmann::json::parse(procsRes->body);
    CHECK(procs.size() == 260);
    int maxPid = 0;
    for (const auto& p : procs) maxPid = std::max(maxPid, p.value("pid", 0));
    CHECK(maxPid == 260);

    // /api/statistics must count the same set.
    auto statsRes = cli.Get("/api/statistics");
    REQUIRE(statsRes);
    auto stats = nlohmann::json::parse(statsRes->body);
    CHECK(stats.value("totalProcesses", 0) == 260);

    // The count command must see and terminate the whole set: target 3 keeps
    // only three alive no matter where their pids sit.
    auto post = [&](const std::string& body) {
        auto r = cli.Post("/api/command", body, "application/json");
        REQUIRE(r);
        return nlohmann::json::parse(r->body);
    };
    auto aliveNormals = [&]() {
        int n = 0;
        for (const int pid : pm.processIds()) {
            auto* pcb = pm.getProcess(pid);
            if (pcb && pcb->type == ProcessType::NORMAL &&
                pcb->state != ProcessState::TERMINATED &&
                pcb->state != ProcessState::FAILED) {
                ++n;
            }
        }
        return n;
    };

    auto shrink = post(R"({"command":"SET_PROCESS_COUNT","payload":{"count":3}})");
    CHECK(shrink.value("success", false) == true);
    CHECK(aliveNormals() == 3);

    // Growing again creates fresh processes past pid 255 while still counting
    // correctly; before the fix these were invisible and unkillable.
    auto grow = post(R"({"command":"SET_PROCESS_COUNT","payload":{"count":5}})");
    CHECK(grow.value("success", false) == true);
    CHECK(aliveNormals() == 5);
    int maxAlivePid = 0;
    for (const int pid : pm.processIds()) {
        auto* pcb = pm.getProcess(pid);
        if (pcb && pcb->state != ProcessState::TERMINATED &&
            pcb->state != ProcessState::FAILED) {
            maxAlivePid = std::max(maxAlivePid, pid);
        }
    }
    CHECK(maxAlivePid > 255);

    // /api/agents must also see agents whose pid exceeds 255.
    int agentPid = pm.createProcess("AgentLate", 1, haltProgram, 0, ProcessType::AI_AGENT);
    REQUIRE(agentPid != INVALID_PID);
    CHECK(agentPid > 255);
    auto agentsRes = cli.Get("/api/agents");
    REQUIRE(agentsRes);
    auto agents = nlohmann::json::parse(agentsRes->body);
    bool found = false;
    for (const auto& a : agents) {
        if (a.value("pid", -1) == agentPid) found = true;
    }
    CHECK(found);

    server.stop();
}
