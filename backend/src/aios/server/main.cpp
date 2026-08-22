#include "aios/core/EventLog.hpp"
#include "aios/core/SimulationClock.hpp"
#include "aios/core/types.hpp"
#include "aios/cpu/CPU.hpp"
#include "aios/interrupt/InterruptManager.hpp"
#include "aios/interrupt/SystemCallManager.hpp"
#include "aios/interrupt/Timer.hpp"
#include "aios/memory/Memory.hpp"
#include "aios/memory/MemoryManager.hpp"
#include "aios/process/ProcessManager.hpp"
#include "aios/scheduling/Scheduler.hpp"
#include "aios/server/ApiServer.hpp"

#include <atomic>
#include <chrono>
#include <csignal>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

using namespace aios;

static bool g_running = true;
static std::atomic<bool> g_simRunning{false};

void signalHandler(int) {
    g_running = false;
}

// Long-running demo program: N arithmetic instructions then HALT. At the
// server's 10 cycles/s tick rate a 200-instruction process runs ~20 s, so the
// dashboard shows live scheduling instead of an instant run-to-completion.
static std::vector<int32_t> makeProgram(int instructions) {
    std::vector<int32_t> prog;
    prog.reserve(static_cast<size_t>(instructions) + 1);
    for (int i = 0; i < instructions; ++i) {
        prog.push_back(Instruction{Opcode::ADD, 1}.toWord());
    }
    prog.push_back(Instruction{Opcode::HALT, 0}.toWord());
    return prog;
}

// Standard demonstration workload (PRD section 20):
// P1 TextEditor, P2 Compiler, P3 MusicPlayer, A1 Research Agent, A2 Coding Agent.
// AI agents are ordinary OS processes created via createAgentProcess
// (docs/06 section 34); they never bypass the scheduler.
static void createDemoWorkload(ProcessManager& pm) {
    const auto proc = makeProgram(200);
    const auto agent = makeProgram(150);

    int p1 = pm.createProcess("TextEditor", 1, proc, 0);
    int p2 = pm.createProcess("Compiler", 2, proc, 0);
    int p3 = pm.createProcess("MusicPlayer", 1, proc, 0);
    int a1 = pm.createAgentProcess("Research", 2, agent, 0);
    int a2 = pm.createAgentProcess("Coding", 1, agent, 0);

    if (p1 != INVALID_PID) pm.admit(p1);
    if (p2 != INVALID_PID) pm.admit(p2);
    if (p3 != INVALID_PID) pm.admit(p3);
    if (a1 != INVALID_PID) pm.admit(a1);
    if (a2 != INVALID_PID) pm.admit(a2);

    std::cout << "[Server] Workload admitted: P1 TextEditor, P2 Compiler, "
                 "P3 MusicPlayer, A1 Research, A2 Coding" << std::endl;
}

int main(int argc, char* argv[]) {
    int port = 8080;
    if (argc > 1) {
        port = std::stoi(argv[1]);
    }

    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    std::cout << "=== AIOS Backend Server ===" << std::endl;
    std::cout << "Starting on port " << port << std::endl;

    // Initialize OS Engine components
    auto memory = std::make_unique<Memory>();
    auto memManager = std::make_unique<MemoryManager>();
    memManager->setMemory(memory.get());
    memManager->initialize(16, 64);   // page size 16 words x 64 frames = Memory::MEMORY_SIZE

    auto eventLog = std::make_unique<EventLog>();
    auto clock = std::make_unique<SimulationClock>();
    auto cpu = std::make_unique<CPU>();
    auto processManager = std::make_unique<ProcessManager>();
    auto scheduler = std::make_unique<Scheduler>();
    auto systemCallManager = std::make_unique<SystemCallManager>();
    auto timer = std::make_unique<Timer>();
    auto interruptManager = std::make_unique<InterruptManager>();

    // Wire components together
    processManager->setMemory(memory.get());
    processManager->setMemoryManager(memManager.get());
    processManager->setEventLog(eventLog.get());
    processManager->setCPU(cpu.get());
    processManager->setClock(clock.get());

    cpu->setMemory(memory.get());
    cpu->setMemoryManager(memManager.get());
    cpu->setEventLog(eventLog.get());
    cpu->setClock(clock.get());
    cpu->setInterruptManager(interruptManager.get());

    memManager->setEventLog(eventLog.get());
    memManager->setClock(clock.get());

    systemCallManager->setEventLog(eventLog.get());
    systemCallManager->setProcessManager(processManager.get());

    interruptManager->setEventLog(eventLog.get());
    interruptManager->setSystemCallManager(systemCallManager.get());
    interruptManager->setProcessManager(processManager.get());
    interruptManager->setMemoryManager(memManager.get());

    timer->setInterruptManager(interruptManager.get());
    timer->setQuantum(4);

    // Round Robin by default: the quantum preemption gives the mid-review
    // demo visible context switches (docs/08 sections 8-10).
    scheduler->setProcessManager(processManager.get());
    scheduler->setEventLog(eventLog.get());
    scheduler->setClock(clock.get());
    scheduler->setPolicy(SchedulingPolicy::ROUND_ROBIN);
    scheduler->setTimeQuantum(timer->quantum());

    interruptManager->setScheduler(scheduler.get());

    // Create API Server
    ApiServer apiServer(port);
    apiServer.setProcessManager(processManager.get());
    apiServer.setMemory(memory.get());
    apiServer.setMemoryManager(memManager.get());
    apiServer.setCPU(cpu.get());
    apiServer.setEventLog(eventLog.get());
    apiServer.setClock(clock.get());
    apiServer.setScheduler(scheduler.get());
    apiServer.setInterruptManager(interruptManager.get());
    apiServer.setRunFlag(&g_simRunning);
    apiServer.setResetHook([&]() {
        createDemoWorkload(*processManager);
    });

    createDemoWorkload(*processManager);

    // Refuse to start when something already answers on this port. Windows
    // socket semantics make bind() alone unreliable for detecting this, which
    // previously left a second silent process while the dashboard talked to
    // the old one (docs/13 section 65: never pretend to be live).
    {
        httplib::Client probe("127.0.0.1", port);
        probe.set_connection_timeout(1, 0);
        if (auto res = probe.Get("/health")) {
            std::cerr << "Port " << port << " already has a running AIOS server."
                      << std::endl
                      << "Stop it first or start on another port:"
                      << std::endl
                      << "  aios_server.exe <port>" << std::endl;
            return 1;
        }
    }

    apiServer.start();
    if (!apiServer.ok()) {
        std::cerr << "Failed to bind port " << port
                  << ". Stop the other AIOS server instance or pick another port:"
                  << std::endl
                  << "  aios_server.exe <port>" << std::endl;
        return 1;
    }

    std::cout << "Server running on port " << port << ". Press Ctrl+C to stop." << std::endl;
    std::cout << "Endpoints:" << std::endl;
    std::cout << "  GET  /health" << std::endl;
    std::cout << "  GET  /api/statistics" << std::endl;
    std::cout << "  GET  /api/processes" << std::endl;
    std::cout << "  GET  /api/agents" << std::endl;
    std::cout << "  GET  /api/cpu" << std::endl;
    std::cout << "  GET  /api/memory" << std::endl;
    std::cout << "  GET  /api/scheduler" << std::endl;
    std::cout << "  GET  /api/interrupts" << std::endl;
    std::cout << "  GET  /api/events" << std::endl;
    std::cout << "  POST /api/command" << std::endl;

    // Simulation loop: one simulated cycle per 100 ms wall-clock while the
    // frontend has pressed Start (docs/13 sections 8-12). Pause/Stop freeze
    // the loop without destroying state.
    std::thread simLoop([&]() {
        while (g_running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            if (!g_simRunning.load()) {
                continue;
            }

            clock->tick();
            timer->tick();

            // Service interrupts (timer -> RR preemption via the scheduler hook)
            interruptManager->serviceNextInterrupt();

            // Schedule when the CPU is free; execute one instruction otherwise.
            if (cpu->currentProcess() == INVALID_PID) {
                const int nextPid = scheduler->schedule();
                if (nextPid != INVALID_PID && nextPid != cpu->currentProcess()) {
                    processManager->dispatch(nextPid);
                }
            } else if (!cpu->isHalted()) {
                cpu->executeCycle();
            } else {
                // HALT finished the process normally (TERMINATED); a CPU error
                // halt is unrecoverable and maps to FAILED instead (docs/06).
                const int donePid = cpu->currentProcess();
                if (cpu->registers().flags.error) {
                    processManager->markFailed(donePid);
                } else {
                    processManager->terminate(donePid);
                }
            }
        }
    });

    // Wait for shutdown
    while (g_running) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::cout << "\nShutting down..." << std::endl;
    apiServer.stop();
    simLoop.join();

    std::cout << "Server stopped." << std::endl;
    return 0;
}
