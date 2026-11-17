#include <aios/core/EventLog.hpp>
#include <aios/core/SimulationClock.hpp>
#include <aios/memory/Memory.hpp>
#include <aios/process/ProcessManager.hpp>
#include <aios/server/ApiServer.hpp>

#include <doctest/doctest.h>

#include <chrono>
#include <mutex>
#include <thread>

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
