#include "aios/server/ApiServer.hpp"

#include <algorithm>
#include <chrono>
#include <iostream>

namespace aios {

ApiServer::ApiServer(int port) : port_(port) {}

ApiServer::~ApiServer() {
    stop();
}

void ApiServer::setProcessManager(ProcessManager* pm) { pm_ = pm; }
void ApiServer::setMemory(Memory* mem) { mem_ = mem; }
void ApiServer::setMemoryManager(MemoryManager* mm) { mm_ = mm; }
void ApiServer::setCPU(CPU* cpu) { cpu_ = cpu; }
void ApiServer::setEventLog(EventLog* log) { log_ = log; }
void ApiServer::setClock(SimulationClock* clock) { clock_ = clock; }
void ApiServer::setScheduler(Scheduler* sched) { sched_ = sched; }
void ApiServer::setInterruptManager(InterruptManager* im) { im_ = im; }

void ApiServer::start() {
    if (running_) return;
    running_ = true;
    std::cout << "[ApiServer] Setting up routes..." << std::endl;
    setupRoutes();
    std::cout << "[ApiServer] Routes set up, starting server..." << std::endl;
    
    // Bind synchronously so a port conflict fails loudly here instead of
    // silently leaving a process that serves nothing (docs/13 section 65).
    if (!server_.bind_to_port("0.0.0.0", port_)) {
        std::cerr << "[ApiServer] ERROR: cannot bind port " << port_
                  << " - another instance is probably already running." << std::endl;
        running_ = false;
        boundOk_ = false;
        return;
    }
    boundOk_ = true;
    
    serverThread_ = std::thread([this]() {
        server_.listen_after_bind();
    });
}

void ApiServer::stop() {
    if (!running_) return;
    running_ = false;
    server_.stop();
    if (serverThread_.joinable()) {
        serverThread_.join();
    }
    std::cout << "[ApiServer] Stopped" << std::endl;
}

json ApiServer::processToJson(const ProcessControlBlock* pcb) const {
    if (!pcb) return nullptr;
    
    json j;
    j["pid"] = pcb->pid;
    j["type"] = (pcb->type == ProcessType::AI_AGENT) ? "AI_AGENT" : "NORMAL";
    j["name"] = pcb->name;
    
    std::string stateStr;
    switch (pcb->state) {
        case ProcessState::NEW: stateStr = "NEW"; break;
        case ProcessState::READY: stateStr = "READY"; break;
        case ProcessState::RUNNING: stateStr = "RUNNING"; break;
        case ProcessState::WAITING: stateStr = "WAITING"; break;
        case ProcessState::TERMINATED: stateStr = "TERMINATED"; break;
        case ProcessState::FAILED: stateStr = "FAILED"; break;
        default: stateStr = "UNKNOWN";
    }
    j["state"] = stateStr;
    j["priority"] = pcb->priority;
    j["memory"] = pcb->programSize / 1024; // KB approximation
    j["cpuTime"] = pcb->cpuTime;
    j["readySinceCycle"] = pcb->readySinceCycle;
    j["waitingTime"] = pcb->waitingTime;
    j["contextSwitchCount"] = pcb->contextSwitchCount;
    j["pageFaultCount"] = 0; // Not directly tracked in PCB
    
    if (pcb->type == ProcessType::AI_AGENT) {
        j["agentId"] = "A" + std::to_string(pcb->pid);
        j["task"] = "Simulated Task";
        j["tokenUsage"] = json{{"used", 0}, {"budget", 1000}};
        j["permissions"] = json{{"READ_DATA", true}, {"SEARCH", true}, {"WRITE_CODE", false}};
    }
    
    return j;
}

json ApiServer::agentToJson(const ProcessControlBlock* pcb) const {
    if (!pcb || pcb->type != ProcessType::AI_AGENT) return nullptr;
    return processToJson(pcb);
}

json ApiServer::cpuToJson() const {
    json j;
    if (!cpu_) return j;
    
    const auto& regs = cpu_->registers();
    j["pc"] = regs.pc;
    j["ir"] = regs.ir.toWord(); // or regs.ir.opcode etc.
    j["acc"] = regs.acc;
    j["mar"] = regs.mar;
    j["mbr"] = regs.mbr;
    j["flags"] = json::object();
    j["flags"]["zero"] = regs.flags.zero;
    j["flags"]["negative"] = regs.flags.negative;
    j["flags"]["carry"] = regs.flags.carry;
    j["flags"]["overflow"] = regs.flags.overflow;
    j["flags"]["error"] = regs.flags.error;
    return j;
}

json ApiServer::memoryToJson() const {
    json j;
    j["frames"] = json::array();
    j["pageTables"] = json::array();
    
    if (mem_ && mm_) {
        // Get physical frames from MemoryManager
        const auto& frames = mm_->frames();
        for (size_t i = 0; i < frames.size(); ++i) {
            json frame;
            frame["frameId"] = static_cast<int>(i);
            if (frames[i].occupied) {
                frame["owner"] = "P" + std::to_string(frames[i].pid);
                frame["processId"] = frames[i].pid;
                frame["pageId"] = frames[i].page;
            } else {
                frame["owner"] = "Free";
                frame["processId"] = nullptr;
                frame["pageId"] = nullptr;
            }
            j["frames"].push_back(frame);
        }
    } else if (mem_) {
        // Fallback: just show memory size as frames
        for (uint32_t i = 0; i < mem_->size() / 4; ++i) {
            json frame;
            frame["frameId"] = static_cast<int>(i);
            frame["owner"] = "Free";
            frame["processId"] = nullptr;
            frame["pageId"] = nullptr;
            j["frames"].push_back(frame);
        }
    }
    
    if (mm_) {
        // Get page tables for all processes
        if (pm_) {
            for (int pid = 0; pid < 256; ++pid) { // Max PID range
                auto* pcb = pm_->getProcess(pid);
                if (pcb) {
                    const auto* pageTable = mm_->getPageTable(pid);
                    if (pageTable) {
                        json table;
                        table["processId"] = pid;
                        table["entries"] = json::array();
                        
                        // PageTable has entries vector
                        for (size_t page = 0; page < pageTable->entries.size(); ++page) {
                            const auto& entry = pageTable->entries[page];
                            json e;
                            e["page"] = static_cast<int>(page);
                            e["frame"] = entry.valid ? static_cast<int>(entry.frame) : -1;
                            e["valid"] = entry.valid;
                            e["modified"] = entry.dirty; // dirty = modified
                            e["referenced"] = entry.referenced;
                            table["entries"].push_back(e);
                        }
                        j["pageTables"].push_back(table);
                    }
                }
            }
        }
    }
    
    return j;
}

json ApiServer::schedulerToJson() const {
    json j;
    if (!sched_) return j;
    
    j["currentAlgorithm"] = schedulingPolicyToString(sched_->policy());
    j["currentProcess"] = sched_->currentRunning();
    j["readyQueue"] = sched_->readyQueue(); // Need to expose this
    j["timeQuantum"] = sched_->timeQuantum();
    j["contextSwitchCount"] = sched_->contextSwitchCount();
    j["decisionCount"] = sched_->decisionCount();
    
    // History
    j["schedulingHistory"] = json::array();
    for (const auto& decision : sched_->history()) {
        json histEntry;
        histEntry["cycle"] = decision.cycle;
        histEntry["policy"] = schedulingPolicyToString(decision.policy);
        histEntry["selectedPid"] = decision.selectedPid;
        histEntry["preemptedPid"] = decision.preemptedPid;
        histEntry["reason"] = decision.reason;
        j["schedulingHistory"].push_back(std::move(histEntry));
    }
    
    // Metrics (docs/08 section 25), averaged over all processes from the
    // Process Manager statistics; throughput = completed / elapsed cycles.
    j["metrics"] = json::object();
    double avgWait = 0.0, avgTurnaround = 0.0, avgResponse = 0.0;
    int completed = 0;
    if (pm_) {
        const auto stats = pm_->getAllProcessStatistics();
        if (!stats.empty()) {
            for (const auto& s : stats) {
                avgWait += static_cast<double>(s.waitingTime);
                avgTurnaround += static_cast<double>(s.turnaroundTime);
                avgResponse += static_cast<double>(s.responseTime);
                if (s.state != ProcessState::TERMINATED && s.state != ProcessState::FAILED) {
                    ++completed;
                }
            }
            avgWait /= static_cast<double>(stats.size());
            avgTurnaround /= static_cast<double>(stats.size());
            avgResponse /= static_cast<double>(stats.size());
        }
    }
    j["metrics"]["avgWaitingTime"] = avgWait;
    j["metrics"]["avgTurnaroundTime"] = avgTurnaround;
    j["metrics"]["avgResponseTime"] = avgResponse;
    j["metrics"]["cpuUtilization"] = pm_ ? pm_->cpuUtilization() : 0.0;
    j["metrics"]["contextSwitchCount"] = sched_->contextSwitchCount();
    const uint64_t cycles = clock_ ? clock_->cycle() : 0;
    j["metrics"]["throughput"] =
        cycles > 0 ? static_cast<double>(completed) / static_cast<double>(cycles) : 0.0;
    
    return j;
}

json ApiServer::statisticsToJson() const {
    json j;
    if (!pm_) return j;
    
    int totalProcesses = 0;
    int aiAgents = 0;
    
    for (int pid = 0; pid < 256; ++pid) {
        auto* pcb = pm_->getProcess(pid);
        if (pcb) {
            ++totalProcesses;
            if (pcb->type == ProcessType::AI_AGENT) ++aiAgents;
        }
    }
    
    // Get interrupt count from log
    int interruptCount = 0;
    if (log_) {
        for (const auto& e : log_->events()) {
            if (e.type == EventType::INTERRUPT || 
                e.type == EventType::PAGE_FAULT ||
                e.type == EventType::SYSCALL ||
                e.type == EventType::IO_COMPLETE ||
                e.type == EventType::IPC_MESSAGE ||
                e.type == EventType::CPU_ERROR) {
                ++interruptCount;
            }
        }
    }
    
    j["totalProcesses"] = totalProcesses;
    j["aiAgents"] = aiAgents;

    // CPU utilization from Process Manager accounting (docs/06 section 29):
    // busy CPU cycles / elapsed cycles.
    const double util = pm_->cpuUtilization();
    j["cpuUsage"] = static_cast<int>(util * 100.0 + 0.5);

    // Memory usage and page-fault totals from the Memory Manager counters
    // (docs/07 sections 34-35); zero when running on the flat-memory path.
    uint64_t faults = 0;
    int memPercent = 0;
    if (mm_) {
        const auto ms = mm_->getStatistics();
        if (ms.totalWords > 0) {
            memPercent = static_cast<int>(
                (static_cast<double>(ms.usedWords) / ms.totalWords) * 100.0 + 0.5);
        }
        faults = ms.pageFaultCount;
    }
    j["memoryUsage"] = memPercent;
    j["pageFaults"] = faults;
    j["interrupts"] = interruptCount;
    j["cycle"] = clock_ ? clock_->cycle() : 0;
    
    return j;
}

json ApiServer::interruptsToJson() const {
    json j = json::array();
    if (!log_) return j;
    
    for (size_t idx = 0; idx < log_->events().size(); ++idx) {
        const auto& e = log_->events()[idx];
        if (e.type == EventType::INTERRUPT || 
            e.type == EventType::PAGE_FAULT ||
            e.type == EventType::SYSCALL ||
            e.type == EventType::IO_COMPLETE ||
            e.type == EventType::IPC_MESSAGE ||
            e.type == EventType::CPU_ERROR) {
            
            std::string typeStr;
            switch (e.type) {
                case EventType::INTERRUPT: typeStr = "INTERRUPT"; break;
                case EventType::PAGE_FAULT: typeStr = "PAGE_FAULT"; break;
                case EventType::SYSCALL: typeStr = "SYSCALL"; break;
                case EventType::IO_COMPLETE: typeStr = "IO_COMPLETE"; break;
                case EventType::IPC_MESSAGE: typeStr = "IPC_MESSAGE"; break;
                case EventType::CPU_ERROR: typeStr = "CPU_ERROR"; break;
                default: typeStr = "UNKNOWN";
            }
            
            std::string statusStr = "COMPLETED";
            
            json intEntry;
            intEntry["id"] = static_cast<int>(idx);
            intEntry["timestamp"] = std::to_string(e.cycle);
            intEntry["type"] = typeStr;
            intEntry["description"] = e.detail;
            intEntry["status"] = statusStr;
            intEntry["pid"] = e.pid;
            j.push_back(std::move(intEntry));
        }
    }
    
    return j;
}

json ApiServer::eventsToJson(int limit) const {
    json j = json::array();
    if (!log_) return j;
    
    const auto& events = log_->events();
    int start = std::max(0, static_cast<int>(events.size()) - limit);
    
    for (int i = start; i < static_cast<int>(events.size()); ++i) {
        const auto& e = events[i];
        
        std::string category;
        switch (e.type) {
            case EventType::PROCESS_CREATED: category = "PROCESS"; break;
            case EventType::PROCESS_TERMINATED: category = "PROCESS"; break;
            case EventType::PROCESS_STATE_CHANGED: category = "PROCESS"; break;
            case EventType::CONTEXT_SWITCH: category = "SCHEDULER"; break;
            case EventType::PAGE_FAULT: category = "MEMORY"; break;
            case EventType::PAGE_LOADED: category = "MEMORY"; break;
            case EventType::PAGE_REPLACED: category = "MEMORY"; break;
            case EventType::INTERRUPT: category = "INTERRUPT"; break;
            case EventType::SYSCALL: category = "INTERRUPT"; break;
            case EventType::IO_COMPLETE: category = "INTERRUPT"; break;
            case EventType::IPC_MESSAGE: category = "IPC"; break;
            case EventType::IO_REQUEST: category = "IO"; break;
            case EventType::SCHEDULER_STARTED: category = "SCHEDULER"; break;
            case EventType::PROCESS_SELECTED: category = "SCHEDULER"; break;
            case EventType::PROCESS_PREEMPTED: category = "SCHEDULER"; break;
            case EventType::TIME_QUANTUM_EXPIRED: category = "SCHEDULER"; break;
            case EventType::CPU_ERROR: category = "ERROR"; break;
            default: category = "PROCESS";
        }
        
        json evtEntry;
        evtEntry["id"] = i;
        evtEntry["timestamp"] = std::to_string(e.cycle);
        evtEntry["category"] = category;
        evtEntry["message"] = e.detail;
        evtEntry["pid"] = e.pid;
        j.push_back(std::move(evtEntry));
    }
    
    return j;
}

void ApiServer::setupRoutes() {
    // CORS - using simpler approach
    httplib::Headers corsHeaders;
    corsHeaders.emplace("Access-Control-Allow-Origin", "*");
    corsHeaders.emplace("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    corsHeaders.emplace("Access-Control-Allow-Headers", "Content-Type");
    server_.set_default_headers(corsHeaders);
    
    server_.Options(".*", [](const httplib::Request&, httplib::Response& res) {
        res.status = 200;
    });
    
    // Health check
    server_.Get("/health", [](const httplib::Request&, httplib::Response& res) {
        std::cout << "[ApiServer] GET /health called" << std::endl;
        res.set_content(R"({"status":"ok"})", "application/json");
    });
    
    // Simulation commands
    server_.Post("/api/command", [this](const httplib::Request& req, httplib::Response& res) {
        try {
            auto body = json::parse(req.body);
            std::string command = body.value("command", "");
            auto payload = body.value("payload", json::object());
            
            bool success = false;
            std::string error;
            
            if (command == "START_SIMULATION") {
                if (runFlag_) { runFlag_->store(true); success = true; }
                else error = "Run flag not initialized";
            } else if (command == "PAUSE_SIMULATION") {
                // Pause preserves CPU/process/memory state (docs/13 section 9).
                if (runFlag_) { runFlag_->store(false); success = true; }
                else error = "Run flag not initialized";
            } else if (command == "STOP_SIMULATION") {
                if (runFlag_) { runFlag_->store(false); success = true; }
                else error = "Run flag not initialized";
            } else if (command == "RESET_SIMULATION") {
                if (runFlag_) runFlag_->store(false);
                if (pm_) pm_->reset();
                if (mem_) mem_->reset();
                if (mm_) mm_->reset();
                if (cpu_) cpu_->reset();
                if (clock_) clock_->reset();
                if (log_) log_->clear();
                if (sched_) sched_->reset();
                if (resetHook_) resetHook_();
                success = true;
            } else if (command == "CHANGE_SCHEDULER") {
                std::string policy = payload.value("policy", "FCFS");
                if (sched_) {
                    if (policy == "FCFS") sched_->setPolicy(SchedulingPolicy::FCFS);
                    else if (policy == "ROUND_ROBIN") sched_->setPolicy(SchedulingPolicy::ROUND_ROBIN);
                    else if (policy == "PRIORITY") sched_->setPolicy(SchedulingPolicy::PRIORITY);
                    else if (policy == "AI_ADAPTIVE") sched_->setPolicy(SchedulingPolicy::ROUND_ROBIN); // AI scheduler is a later milestone (docs/08 section 44)
                    success = true;
                } else error = "Scheduler not initialized";
            } else if (command == "RUN_SCENARIO") {
                std::cout << "[ApiServer] RUN_SCENARIO: " << payload.value("scenario", std::string("basic_multiprogramming")) << " (workload preloaded)" << std::endl;
                success = true;
            } else {
                error = "Unknown command: " + command;
            }
            
            json response;
            response["success"] = success;
            if (!error.empty()) response["error"] = error;
            res.set_content(response.dump(), "application/json");
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content(json{{"success", false}, {"error", e.what()}}.dump(), "application/json");
        }
    });
    
    // State endpoints
    server_.Get("/api/statistics", [this](const httplib::Request&, httplib::Response& res) {
        res.set_content(statisticsToJson().dump(), "application/json");
    });
    
    server_.Get("/api/processes", [this](const httplib::Request&, httplib::Response& res) {
        json arr = json::array();
        if (pm_) {
            for (int pid = 0; pid < 256; ++pid) {
                auto* pcb = pm_->getProcess(pid);
                if (pcb && pcb->type == ProcessType::NORMAL) {
                    arr.push_back(processToJson(pcb));
                }
            }
        }
        res.set_content(arr.dump(), "application/json");
    });
    
    server_.Get("/api/agents", [this](const httplib::Request&, httplib::Response& res) {
        json arr = json::array();
        if (pm_) {
            for (int pid = 0; pid < 256; ++pid) {
                auto* pcb = pm_->getProcess(pid);
                if (pcb && pcb->type == ProcessType::AI_AGENT) {
                    arr.push_back(agentToJson(pcb));
                }
            }
        }
        res.set_content(arr.dump(), "application/json");
    });
    
    server_.Get("/api/cpu", [this](const httplib::Request&, httplib::Response& res) {
        res.set_content(cpuToJson().dump(), "application/json");
    });
    
    server_.Get("/api/memory", [this](const httplib::Request&, httplib::Response& res) {
        res.set_content(memoryToJson().dump(), "application/json");
    });
    
    server_.Get("/api/scheduler", [this](const httplib::Request&, httplib::Response& res) {
        res.set_content(schedulerToJson().dump(), "application/json");
    });
    
    server_.Get("/api/interrupts", [this](const httplib::Request&, httplib::Response& res) {
        res.set_content(interruptsToJson().dump(), "application/json");
    });
    
    server_.Get("/api/events", [this](const httplib::Request&, httplib::Response& res) {
        res.set_content(eventsToJson(100).dump(), "application/json");
    });
    
    server_.Get("/api/ipc", [this](const httplib::Request&, httplib::Response& res) {
        json arr = json::array(); // Placeholder
        res.set_content(arr.dump(), "application/json");
    });
    
    server_.Get("/api/io", [this](const httplib::Request&, httplib::Response& res) {
        json arr = json::array(); // Placeholder
        res.set_content(arr.dump(), "application/json");
    });
    
    server_.Get("/api/mutexes", [this](const httplib::Request&, httplib::Response& res) {
        json arr = json::array(); // Placeholder
        res.set_content(arr.dump(), "application/json");
    });
}

void ApiServer::setupWebSocket() {
    server_.Get("/ws", [this](const httplib::Request& req, httplib::Response& res) {
        if (req.has_header("Upgrade") && req.get_header_value("Upgrade") == "websocket") {
            // Handle WebSocket upgrade manually
            res.status = 101;
            res.set_header("Upgrade", "websocket");
            res.set_header("Connection", "Upgrade");
            // Note: cpp-httplib WebSocket support is limited
            // For production, consider using a dedicated WebSocket library
        } else {
            res.status = 400;
            res.set_content("WebSocket upgrade required", "text/plain");
        }
    });
    
    // Simple polling endpoint as alternative to WebSocket
    server_.Get("/api/events/poll", [this](const httplib::Request& req, httplib::Response& res) {
        std::string sinceStr = req.get_param_value("since");
        int since = sinceStr.empty() ? 0 : std::stoi(sinceStr);
        // Return events after 'since' index (using event vector index as id)
        json j = json::array();
        if (log_) {
            const auto& events = log_->events();
            for (size_t i = since; i < events.size(); ++i) {
                const auto& e = events[i];
                std::string category = "PROCESS";
                // ... same category logic as eventsToJson
                json evtEntry;
                evtEntry["id"] = static_cast<int>(i);
                evtEntry["timestamp"] = std::to_string(e.cycle);
                evtEntry["category"] = category;
                evtEntry["message"] = e.detail;
                evtEntry["pid"] = e.pid;
                j.push_back(std::move(evtEntry));
            }
        }
        res.set_content(j.dump(), "application/json");
    });
}

} // namespace aios