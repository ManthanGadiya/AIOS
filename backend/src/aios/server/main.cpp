#include "aios/agent/AIAgentManager.hpp"
#include "aios/core/EventLog.hpp"
#include "aios/core/SimulationClock.hpp"
#include "aios/core/types.hpp"
#include "aios/cpu/CPU.hpp"
#include "aios/interrupt/InterruptManager.hpp"
#include "aios/interrupt/SystemCallManager.hpp"
#include "aios/interrupt/Timer.hpp"
#include "aios/io/IOManager.hpp"
#include "aios/ipc/IPCManager.hpp"
#include "aios/memory/Memory.hpp"
#include "aios/memory/MemoryManager.hpp"
#include "aios/process/ProcessManager.hpp"
#include "aios/scheduling/Scheduler.hpp"
#include "aios/sync/SynchronizationManager.hpp"
#include "aios/server/ApiServer.hpp"

#include <atomic>
#include <chrono>
#include <csignal>
#include <iostream>
#include <memory>
#include <mutex>
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

// Demonstration program that issues one blocking I/O syscall mid-run
// (docs/05 section 6, docs/11): compute, IO_REQUEST, block until the device
// completes, compute, HALT. The payload is supplied per-PID via
// setSyscallPayload and consumed on first use, so exactly one SYSCALL
// instruction may appear in the program - a second one would reach
// handleIoRequest with an empty payload and is fatal (docs/05 section 4).
static std::vector<int32_t> makeIoProgram(int instructions, int ioAfter) {
    std::vector<int32_t> prog;
    prog.reserve(static_cast<size_t>(instructions) + 2);
    for (int i = 0; i < ioAfter; ++i) {
        prog.push_back(Instruction{Opcode::ADD, 1}.toWord());
    }
    prog.push_back(Instruction{Opcode::SYSCALL, SYSCALL_IO_REQUEST}.toWord());
    for (int i = ioAfter; i < instructions; ++i) {
        prog.push_back(Instruction{Opcode::ADD, 1}.toWord());
    }
    prog.push_back(Instruction{Opcode::HALT, 0}.toWord());
    return prog;
}

// Standard demonstration workload (PRD section 20):
// P1 TextEditor, P2 Compiler, P3 MusicPlayer, A1 Research Agent, A2 Coding Agent.
// AI agents are ordinary OS processes created via createAgentProcess
// (docs/06 section 34); they never bypass the scheduler.
// Each process performs one blocking I/O request so the dashboard's I/O
// section shows real device activity (docs/11, docs/14 scenario events).
static void createDemoWorkload(ProcessManager& pm, AIAgentManager& aiAgents,
                               SynchronizationManager* sync, IPCManager* ipc,
                               IOManager* io, SystemCallManager* syscalls) {
    const auto proc = makeIoProgram(200, 8);
    const auto agent = makeIoProgram(150, 12);

    int p1 = pm.createProcess("TextEditor", 1, proc, 0);
    int p2 = pm.createProcess("Compiler", 2, proc, 0);
    int p3 = pm.createProcess("MusicPlayer", 1, proc, 0);

    // Create AI agents through the AI Agent Manager (docs/12 section 11)
    int a1 = aiAgents.createAgent("Research", AgentType::RESEARCH, 2, agent, 0, 1000);
    int a2 = aiAgents.createAgent("Coding", AgentType::CODING, 1, agent, 0, 1000);

    // I/O payloads: "deviceId:operation:data", 0=READ 1=WRITE (docs/05).
    // TextEditor and Compiler both write to DISK so the device shows a real
    // queue; the others spread across the remaining devices.
    if (syscalls) {
        if (p1 != INVALID_PID) syscalls->setSyscallPayload(p1, "DISK:1:document-save");
        if (p2 != INVALID_PID) syscalls->setSyscallPayload(p2, "DISK:1:object-file");
        if (p3 != INVALID_PID) syscalls->setSyscallPayload(p3, "KEYBOARD:0:playlist");
        if (a1 != INVALID_PID) syscalls->setSyscallPayload(a1, "NETWORK:0:research-sources");
        if (a2 != INVALID_PID) syscalls->setSyscallPayload(a2, "PRINTER:1:code-report");
    }

    if (p1 != INVALID_PID) pm.admit(p1);
    if (p2 != INVALID_PID) pm.admit(p2);
    if (p3 != INVALID_PID) pm.admit(p3);
    if (a1 != INVALID_PID) pm.admit(a1);
    if (a2 != INVALID_PID) pm.admit(a2);

    // Create demo synchronization objects (docs/09 sections 38, 42)
    if (sync) {
        sync->createMutex("M1");           // Protects shared counter
        sync->createMutex("knowledge_bus"); // Protects AI knowledge bus
        sync->createSemaphore("S1", 3);    // 3 I/O slots
        std::cout << "[Server] Sync objects created: M1, knowledge_bus, S1(3)" << std::endl;
    }

    // Create demo IPC channels and permissions (docs/10 sections 27, 42)
    if (ipc) {
        ipc->createChannel(a1); // Research agent channel
        ipc->createChannel(a2); // Coding agent channel
        ipc->grantPermission(a1, a2); // Research -> Coding
        ipc->grantPermission(a2, a1); // Coding -> Research
        std::cout << "[Server] IPC channels created: Research->Coding, Coding->Research" << std::endl;
    }

    std::cout << "[Server] Workload admitted: P1 TextEditor, P2 Compiler, "
                 "P3 MusicPlayer, A1 Research, A2 Coding" << std::endl;

    // Create demo I/O devices (docs/11 sections 64-77)
    if (io) {
        io->createDevice("DISK", DeviceType::DISK, 5);      // 5 cycles per I/O
        io->createDevice("PRINTER", DeviceType::PRINTER, 8); // 8 cycles per I/O
        io->createDevice("KEYBOARD", DeviceType::KEYBOARD, 3); // 3 cycles per I/O
        io->createDevice("NETWORK", DeviceType::NETWORK, 6);  // 6 cycles per I/O
        std::cout << "[Server] I/O devices created: DISK, PRINTER, KEYBOARD, NETWORK" << std::endl;

        // Create demo buffers
        io->createBuffer("DISK_BUFFER", 64);
        io->createBuffer("PRINTER_BUFFER", 128);
        std::cout << "[Server] Buffers created: DISK_BUFFER(64), PRINTER_BUFFER(128)" << std::endl;
    }
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
    auto syncManager = std::make_unique<SynchronizationManager>();
    auto ipcManager = std::make_unique<IPCManager>();
    auto ioManager = std::make_unique<IOManager>();
    auto aiAgentManager = std::make_unique<AIAgentManager>();

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
    systemCallManager->setSynchronizationManager(syncManager.get());
    systemCallManager->setIOManager(ioManager.get());
    systemCallManager->setClock(clock.get());

    syncManager->setEventLog(eventLog.get());
    syncManager->setProcessManager(processManager.get());
    syncManager->setClock(clock.get());

    ipcManager->setEventLog(eventLog.get());
    ipcManager->setProcessManager(processManager.get());
    ipcManager->setClock(clock.get());

    ioManager->setEventLog(eventLog.get());
    ioManager->setProcessManager(processManager.get());
    ioManager->setClock(clock.get());

    interruptManager->setEventLog(eventLog.get());
    interruptManager->setSystemCallManager(systemCallManager.get());
    interruptManager->setProcessManager(processManager.get());
    interruptManager->setMemoryManager(memManager.get());

    // AI Agent Manager wiring
    aiAgentManager->setEventLog(eventLog.get());
    aiAgentManager->setProcessManager(processManager.get());
    aiAgentManager->setMemoryManager(memManager.get());
    aiAgentManager->setSynchronizationManager(syncManager.get());
    aiAgentManager->setIPCManager(ipcManager.get());
    aiAgentManager->setClock(clock.get());

    timer->setInterruptManager(interruptManager.get());
    timer->setQuantum(4);

    // Round Robin by default: the quantum preemption gives the mid-review
    // demo visible context switches (docs/08 sections 8-10).
    scheduler->setProcessManager(processManager.get());
    scheduler->setAIAgentManager(aiAgentManager.get());
    scheduler->setEventLog(eventLog.get());
    scheduler->setClock(clock.get());
    scheduler->setPolicy(SchedulingPolicy::ROUND_ROBIN);
    scheduler->setTimeQuantum(timer->quantum());

    interruptManager->setScheduler(scheduler.get());

    // Create API Server
    ApiServer apiServer(port);
    // One engine mutex serializes the simulation loop against every HTTP
    // handler: without it a mid-run scheduler change (or any dashboard read)
    // races the sim thread inside the managers and crashes (docs/13).
    std::mutex engineMutex;
    apiServer.setEngineMutex(&engineMutex);
    apiServer.setProcessManager(processManager.get());
    apiServer.setMemory(memory.get());
    apiServer.setMemoryManager(memManager.get());
    apiServer.setCPU(cpu.get());
    apiServer.setEventLog(eventLog.get());
    apiServer.setClock(clock.get());
    apiServer.setScheduler(scheduler.get());
    apiServer.setInterruptManager(interruptManager.get());
    apiServer.setSynchronizationManager(syncManager.get());
    apiServer.setIPCManager(ipcManager.get());
    apiServer.setIOManager(ioManager.get());
    apiServer.setSystemCallManager(systemCallManager.get());
    apiServer.setAIAgentManager(aiAgentManager.get());
    apiServer.setRunFlag(&g_simRunning);
    apiServer.setResetHook([&]() {
        // Devices still hold queues from the previous run; recreate them so the
        // workload below starts from a clean I/O state.
        if (ioManager) ioManager->reset();
        createDemoWorkload(*processManager, *aiAgentManager, syncManager.get(), ipcManager.get(), ioManager.get(), systemCallManager.get());
    });

    createDemoWorkload(*processManager, *aiAgentManager, syncManager.get(), ipcManager.get(), ioManager.get(), systemCallManager.get());

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

            std::lock_guard<std::mutex> lock(engineMutex);

            clock->tick();
            timer->tick();
            ioManager->tick();

            // Service interrupts (timer -> RR preemption via the scheduler hook)
            interruptManager->serviceNextInterrupt();

            // A serviced PAGE_FAULT/SYSCALL leaves the CPU INTERRUPTED: the OS
            // resumes it so the faulting instruction is retried (docs/05,
            // docs/07 section 16). A handler that blocked or terminated the
            // process already released the CPU, so resume() is a no-op then.
            if (cpu->state() == CpuState::INTERRUPTED) {
                cpu->resume();
            }

            // Schedule every cycle, not only when idle: the preemptive
            // policies (PRIORITY, AI_ADAPTIVE) only see newly-ready processes
            // here (docs/08 section 11).
            scheduler->schedule();

            if (cpu->currentProcess() != INVALID_PID) {
                if (!cpu->isHalted()) {
                    cpu->executeCycle();
                } else {
                    // HALT finished the process normally (TERMINATED); a CPU
                    // error halt is unrecoverable and maps to FAILED instead
                    // (docs/06).
                    const int donePid = cpu->currentProcess();
                    if (cpu->registers().flags.error) {
                        processManager->markFailed(donePid);
                    } else {
                        processManager->terminate(donePid);
                    }
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
