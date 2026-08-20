#include <aios/process/ProcessManager.hpp>

#include <aios/cpu/CPU.hpp>
#include <aios/memory/Memory.hpp>
#include <aios/memory/MemoryManager.hpp>

#include <utility>

namespace aios {

int ProcessManager::createProcess(const std::string& name, int priority,
                                  const std::vector<int32_t>& program, uint32_t base,
                                  ProcessType type) {
    if (!memory_ || program.empty()) {
        return INVALID_PID;
    }
    const int pid = nextPid_++;
    if (memoryManager_) {
        // Paged path: image goes to swap; logical space starts at 0.
        if (!memoryManager_->allocateProcessMemory(pid, program)) {
            return INVALID_PID;
        }
    } else if (!memory_->loadProgram(program, base)) {
        return INVALID_PID;
    }
    const uint64_t cycle = clock_ ? clock_->cycle() : 0;

    ProcessControlBlock pcb;
    pcb.pid = pid;
    pcb.name = name;
    pcb.priority = priority;
    pcb.type = type;
    pcb.state = ProcessState::NEW;
    pcb.baseAddress = memoryManager_ ? 0 : base;
    pcb.programSize = static_cast<uint32_t>(program.size());
    pcb.context.pc = memoryManager_ ? 0 : static_cast<int32_t>(base);
    pcb.createdCycle = cycle;
    pcbs_.emplace(pid, std::move(pcb));

    if (eventLog_) {
        eventLog_->record(EventType::PROCESS_CREATED, pid, cycle, name);
    }
    return pid;
}

int ProcessManager::createAgentProcess(const std::string& name, int priority,
                                       const std::vector<int32_t>& program, uint32_t base) {
    return createProcess(name, priority, program, base, ProcessType::AI_AGENT);
}

int ProcessManager::runningPid() const {
    return cpu_ ? cpu_->currentProcess() : INVALID_PID;
}

bool ProcessManager::admit(int pid) {
    return transition(pid, ProcessState::READY, "admit");
}

bool ProcessManager::dispatch(int pid) {
    ProcessControlBlock* pcb = getProcess(pid);
    if (!pcb || pcb->state != ProcessState::READY || !cpu_) {
        return false;
    }
    // Preempt the current process if any (RUNNING -> READY, context saved).
    const int current = cpu_->currentProcess();
    if (current != INVALID_PID && current != pid) {
        ProcessControlBlock* cur = getProcess(current);
        if (cur && cur->state == ProcessState::RUNNING) {
            saveRunningContext();
            chargeCpuTime(*cur);
            transition(current, ProcessState::READY, "preempted");
        }
    }
    // Leaving the ready queue is charged centrally in transition().
    cpu_->loadContext(pcb->context, pid);
    transition(pid, ProcessState::RUNNING, "dispatch");
    // CPU-time accounting: the run is charged when the process leaves the CPU.
    pcb->onCpu = true;
    pcb->lastCpuCycle = clock_ ? clock_->cycle() : 0;
    ++pcb->contextSwitchCount;
    if (pcb->firstRunCycle == 0) {
        pcb->firstRunCycle = clock_ ? clock_->cycle() : 0;
    }
    if (eventLog_) {
        eventLog_->record(EventType::CONTEXT_SWITCH, pid, clock_ ? clock_->cycle() : 0,
                          "dispatch");
    }
    return true;
}

bool ProcessManager::terminate(int pid) {
    ProcessControlBlock* pcb = getProcess(pid);
    if (!pcb || !transition(pid, ProcessState::TERMINATED, "terminate")) {
        return false;
    }
    releaseCpuIfRunning(pid);
    if (memoryManager_) {
        memoryManager_->releaseProcessMemory(pid);
    }
    if (eventLog_) {
        eventLog_->record(EventType::PROCESS_TERMINATED, pid,
                          clock_ ? clock_->cycle() : 0, pcb->name);
    }
    return true;
}

bool ProcessManager::blockProcess(int pid) {
    if (!transition(pid, ProcessState::WAITING, "block")) {
        return false;
    }
    releaseCpuIfRunning(pid);
    return true;
}

bool ProcessManager::wakeProcess(int pid) {
    return transition(pid, ProcessState::READY, "wake");
}

bool ProcessManager::markFailed(int pid) {
    if (!transition(pid, ProcessState::FAILED, "unrecoverable error")) {
        return false;
    }
    releaseCpuIfRunning(pid);
    if (memoryManager_) {
        memoryManager_->releaseProcessMemory(pid);
    }
    return true;
}

ProcessControlBlock* ProcessManager::getProcess(int pid) {
    const auto it = pcbs_.find(pid);
    return it == pcbs_.end() ? nullptr : &it->second;
}

const ProcessControlBlock* ProcessManager::getProcess(int pid) const {
    const auto it = pcbs_.find(pid);
    return it == pcbs_.end() ? nullptr : &it->second;
}

std::vector<int> ProcessManager::readyQueue() const {
    std::vector<int> pids;
    for (const auto& [pid, pcb] : pcbs_) {
        if (pcb.state == ProcessState::READY) {
            pids.push_back(pid);
        }
    }
    return pids;
}

std::vector<int> ProcessManager::processIds() const {
    std::vector<int> pids;
    pids.reserve(pcbs_.size());
    for (const auto& [pid, pcb] : pcbs_) {
        (void)pcb;
        pids.push_back(pid);
    }
    return pids;
}

void ProcessManager::reset() {
    if (memoryManager_) {
        for (const auto& [pid, pcb] : pcbs_) {
            (void)pcb;
            memoryManager_->releaseProcessMemory(pid);
        }
    }
    for (auto& [pid, pcb] : pcbs_) {
        (void)pid;
        chargeCpuTime(pcb);
        chargeWaitingTime(pcb);
    }
    pcbs_.clear();
    nextPid_ = 1;
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

bool ProcessManager::transition(int pid, ProcessState to, const std::string& reason) {
    ProcessControlBlock* pcb = getProcess(pid);
    if (!pcb) {
        return false;
    }
    const ProcessState from = pcb->state;

    // Enforced state machine (docs/06 section 15):
    //   NEW -> READY | FAILED
    //   READY -> RUNNING | FAILED
    //   RUNNING -> READY | WAITING | TERMINATED | FAILED
    //   WAITING -> READY | TERMINATED | FAILED
    //   TERMINATED / FAILED: terminal (no outgoing edges)
    bool allowed = false;
    switch (to) {
        case ProcessState::READY:
            allowed = (from == ProcessState::NEW || from == ProcessState::RUNNING ||
                       from == ProcessState::WAITING);
            break;
        case ProcessState::RUNNING:
            allowed = (from == ProcessState::READY);
            break;
        case ProcessState::WAITING:
            allowed = (from == ProcessState::RUNNING);
            break;
        case ProcessState::TERMINATED:
            allowed = (from == ProcessState::RUNNING || from == ProcessState::WAITING);
            break;
        case ProcessState::FAILED:
            allowed = pcb->isAlive();
            break;
        case ProcessState::NEW:
            allowed = false;
            break;
    }
    if (!allowed) {
        return false;
    }

    pcb->state = to;
    ++pcb->stateChangeCount;
    // Leave the ready queue: charge the time spent waiting (docs/06 §29).
    // RUNNING -> READY is charged here too when the preempted process is
    // re-dispatched later.
    if (from == ProcessState::READY && to != ProcessState::READY) {
        chargeWaitingTime(*pcb);
    }
    if (to == ProcessState::TERMINATED || to == ProcessState::FAILED) {
        pcb->terminatedCycle = clock_ ? clock_->cycle() : 0;
    }
    if (to == ProcessState::READY) {
        pcb->readySinceCycle = clock_ ? clock_->cycle() : 0;
        pcb->inReadyQueue = true;
    }
    if (eventLog_) {
        eventLog_->record(EventType::PROCESS_STATE_CHANGED, pid,
                          clock_ ? clock_->cycle() : 0,
                          processStateToString(from) + " -> " + processStateToString(to) +
                              " (" + reason + ")");
    }
    return true;
}

void ProcessManager::saveRunningContext() {
    if (!cpu_ || cpu_->currentProcess() == INVALID_PID) {
        return;
    }
    ProcessControlBlock* pcb = getProcess(cpu_->currentProcess());
    if (!pcb) {
        return;
    }
    pcb->context = cpu_->saveContext();
}

void ProcessManager::releaseCpuIfRunning(int pid) {
    if (cpu_ && cpu_->currentProcess() == pid) {
        if (ProcessControlBlock* pcb = getProcess(pid)) {
            chargeCpuTime(*pcb);
        }
        cpu_->reset();
    }
}

void ProcessManager::chargeCpuTime(ProcessControlBlock& pcb) {
    if (!pcb.onCpu) {
        return;
    }
    pcb.cpuTime += (clock_ ? clock_->cycle() : 0) - pcb.lastCpuCycle;
    pcb.onCpu = false;
    pcb.lastCpuCycle = 0;
}

void ProcessManager::chargeWaitingTime(ProcessControlBlock& pcb) {
    if (!pcb.inReadyQueue) {
        return;
    }
    pcb.waitingTime += (clock_ ? clock_->cycle() : 0) - pcb.readySinceCycle;
    pcb.readySinceCycle = 0;
    pcb.inReadyQueue = false;
}

// ---------------------------------------------------------------------------
// Process statistics (docs/06 sections 28-29)
// ---------------------------------------------------------------------------

std::optional<ProcessStatistics> ProcessManager::getProcessStatistics(int pid) const {
    const ProcessControlBlock* pcb = getProcess(pid);
    if (!pcb) {
        return std::nullopt;
    }
    ProcessStatistics s;
    s.pid = pcb->pid;
    s.name = pcb->name;
    s.state = pcb->state;
    s.type = pcb->type;
    s.priority = pcb->priority;
    s.baseAddress = pcb->baseAddress;
    s.programSize = pcb->programSize;
    s.arrivalTime = pcb->createdCycle;
    s.cpuTime = pcb->cpuTime;
    s.waitingTime = pcb->waitingTime;
    if (pcb->terminatedCycle >= pcb->createdCycle) {
        s.turnaroundTime = pcb->terminatedCycle - pcb->createdCycle;
    }
    if (pcb->firstRunCycle != 0) {
        s.responseTime = pcb->firstRunCycle - pcb->createdCycle;
    }
    s.stateChanges = pcb->stateChangeCount;
    s.contextSwitches = pcb->contextSwitchCount;
    if (memoryManager_) {
        const auto& faults = memoryManager_->getStatistics().perProcessFaults;
        const auto it = faults.find(pid);
        if (it != faults.end()) {
            s.pageFaults = it->second;
        }
    }
    s.ioRequests = pcb->ioRequests;
    s.ipcOperations = pcb->ipcOperations;
    return s;
}

std::vector<ProcessStatistics> ProcessManager::getAllProcessStatistics() const {
    std::vector<ProcessStatistics> stats;
    stats.reserve(pcbs_.size());
    for (const auto& [pid, pcb] : pcbs_) {
        (void)pcb;
        if (auto s = getProcessStatistics(pid)) {
            stats.push_back(*s);
        }
    }
    return stats;
}

double ProcessManager::cpuUtilization() const {
    if (!clock_) {
        return 0.0;
    }
    const uint64_t total = clock_->cycle();
    if (total == 0) {
        return 0.0;
    }
    uint64_t busy = 0;
    for (const auto& [pid, pcb] : pcbs_) {
        (void)pid;
        busy += pcb.cpuTime;
        if (pcb.onCpu) {
            // Include the still-uncharged current run in the snapshot.
            busy += total - pcb.lastCpuCycle;
        }
    }
    const double util = static_cast<double>(busy) / static_cast<double>(total);
    return util > 1.0 ? 1.0 : util;
}

} // namespace aios