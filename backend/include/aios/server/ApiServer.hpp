#pragma once

#include "aios/core/EventLog.hpp"
#include "aios/core/SimulationClock.hpp"
#include "aios/cpu/CPU.hpp"
#include "aios/interrupt/InterruptManager.hpp"
#include "aios/memory/Memory.hpp"
#include "aios/memory/MemoryManager.hpp"
#include "aios/process/ProcessManager.hpp"
#include "aios/scheduling/Scheduler.hpp"
#include "httplib.h"

#include <nlohmann/json.hpp>
#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace aios {

using json = nlohmann::json;

class ApiServer {
public:
    ApiServer(int port = 8080);
    ~ApiServer();

    void setProcessManager(ProcessManager* pm);
    void setMemory(Memory* mem);
    void setMemoryManager(MemoryManager* mm);
    void setCPU(CPU* cpu);
    void setEventLog(EventLog* log);
    void setClock(SimulationClock* clock);
    void setScheduler(Scheduler* sched);
    void setInterruptManager(InterruptManager* im);

    // Simulation run-state control: START/PAUSE/STOP write this flag and the
    // host simulation loop reads it each tick (docs/13 sections 8-11).
    void setRunFlag(std::atomic<bool>* flag) { runFlag_ = flag; }
    // RESET clears engine state; the hook re-creates the demo workload.
    void setResetHook(std::function<void()> hook) { resetHook_ = std::move(hook); }

    void start();
    void stop();

    // False after start() when the port could not be bound (already in use).
    bool ok() const { return boundOk_; }

private:
    void setupRoutes();
    void setupWebSocket();

    json processToJson(const ProcessControlBlock* pcb) const;
    json agentToJson(const ProcessControlBlock* pcb) const;
    json cpuToJson() const;
    json memoryToJson() const;
    json schedulerToJson() const;
    json statisticsToJson() const;
    json interruptsToJson() const;
    json eventsToJson(int limit = 100) const;

    httplib::Server server_;
    int port_;
    bool running_ = false;
    bool boundOk_ = false;
    std::thread serverThread_;

    // OS Engine components
    ProcessManager* pm_ = nullptr;
    Memory* mem_ = nullptr;
    MemoryManager* mm_ = nullptr;
    CPU* cpu_ = nullptr;
    EventLog* log_ = nullptr;
    SimulationClock* clock_ = nullptr;
    Scheduler* sched_ = nullptr;
    InterruptManager* im_ = nullptr;

    // Simulation control shared with the host loop
    std::atomic<bool>* runFlag_ = nullptr;
    std::function<void()> resetHook_;
};

} // namespace aios