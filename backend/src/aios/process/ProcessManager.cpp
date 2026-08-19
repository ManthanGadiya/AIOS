#include <aios/process/ProcessManager.hpp>

#include <aios/cpu/CPU.hpp>
#include <aios/memory/Memory.hpp>

#include <utility>

namespace aios {

int ProcessManager::createProcess(const std::string& name, int priority,
                                  const std::vector<int32_t>& program, uint32_t base) {
    if (!memory_ || !memory_->loadProgram(program, base)) {
        return INVALID_PID;
    }
    const int pid = nextPid_++;
    const uint64_t cycle = clock_ ? clock_->cycle() : 0;

    ProcessControlBlock pcb;
    pcb.pid = pid;
    pcb.name = name;
    pcb.priority = priority;
    pcb.state = ProcessState::NEW;
    pcb.baseAddress = base;
    pcb.programSize = static_cast<uint32_t>(program.size());
    pcb.context.pc = static_cast<int32_t>(base);
    pcb.createdCycle = cycle;
    pcbs_.emplace(pid, std::move(pcb));

    if (eventLog_) {
        eventLog_->record(EventType::PROCESS_CREATED, pid, cycle, name);
    }
    return pid;
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
            transition(current, ProcessState::READY, "preempted");
        }
    }
    cpu_->loadContext(pcb->context, pid);
    transition(pid, ProcessState::RUNNING, "dispatch");
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
    if (to == ProcessState::TERMINATED || to == ProcessState::FAILED) {
        pcb->terminatedCycle = clock_ ? clock_->cycle() : 0;
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
        cpu_->reset();
    }
}

} // namespace aios