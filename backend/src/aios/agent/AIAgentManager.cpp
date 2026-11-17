#include <aios/agent/AIAgentManager.hpp>

#include <aios/process/ProcessControlBlock.hpp>
#include <aios/memory/MemoryManager.hpp>
#include <aios/sync/SynchronizationManager.hpp>
#include <aios/ipc/IPCManager.hpp>

#include <algorithm>
#include <sstream>

namespace aios {

std::string AIAgentManager::generateAgentId() {
    return "AGENT-" + std::to_string(nextAgentId_++);
}

std::string AIAgentManager::generateCheckpointId() {
    return "CP-" + std::to_string(nextCheckpointId_++);
}

void AIAgentManager::recordAgentEvent(EventType type, const std::string& agentId, const std::string& detail) {
    if (eventLog_) {
        int pid = INVALID_PID;
        auto it = agents_.find(agentId);
        if (it != agents_.end()) {
            pid = it->second.pid;
        }
        eventLog_->record(type, pid, clock_ ? clock_->cycle() : 0, agentId + ": " + detail);
    }
}

int AIAgentManager::createAgent(const std::string& name, AgentType type, int priority,
                                const std::vector<int32_t>& program, uint32_t base,
                                uint64_t tokenBudget,
                                const std::vector<ToolPermission>& permissions) {
    if (!processes_) {
        return INVALID_PID;
    }

    // Default permissions based on agent type (docs/12 section 27)
    std::vector<ToolPermission> defaultPerms = permissions;
    if (defaultPerms.empty()) {
        if (type == AgentType::RESEARCH) {
            defaultPerms = {ToolPermission::READ_DATA, ToolPermission::SEARCH};
        } else { // CODING
            defaultPerms = {ToolPermission::READ_DATA, ToolPermission::WRITE_CODE, ToolPermission::EXECUTE};
        }
    }

    // Create the process via Process Manager
    int pid = processes_->createAgentProcess(name, priority, program, base);
    if (pid == INVALID_PID) {
        return INVALID_PID;
    }

    // Create agent metadata
    std::string agentId = generateAgentId();
    AgentMetadata meta;
    meta.agentId = agentId;
    meta.name = name;
    meta.type = type;
    meta.pid = pid;
    meta.priority = priority;
    meta.tokenBudget = tokenBudget;
    meta.tokenUsed = 0;
    meta.permissions = defaultPerms;
    meta.agentState = AgentState::ACTIVE;
    meta.checkpointId = "";
    meta.checkpointCount = 0;
    meta.failureCount = 0;
    meta.recoveryCount = 0;
    meta.cpuCyclesUsed = 0;
    meta.ipcSent = 0;
    meta.ipcReceived = 0;
    meta.toolRequests = 0;
    meta.toolDenials = 0;

    // Default quotas
    meta.quota.cpuQuota = 1000;
    meta.quota.cpuUsed = 0;
    meta.quota.memoryQuota = 16;
    meta.quota.memoryUsed = 0;
    meta.quota.tokenBudget = tokenBudget;
    meta.quota.tokenUsed = 0;
    meta.quota.ioLimit = 100;
    meta.quota.ioUsed = 0;

    // Store agent
    agents_.emplace(agentId, meta);
    pidToAgentId_.emplace(pid, agentId);

    // Grant IPC permission to communicate with other agents
    if (ipc_) {
        // Grant permission to send to all other agents (will be updated as they're created)
        // For now, we'll grant per-agent in the IPC calls
    }

    recordAgentEvent(EventType::AGENT_CREATED, agentId,
                     "type=" + agentTypeToString(type) + " pid=" + std::to_string(pid) +
                     " priority=" + std::to_string(priority) +
                     " tokenBudget=" + std::to_string(tokenBudget));

    return pid;
}

std::optional<AgentMetadata> AIAgentManager::getAgent(const std::string& agentId) const {
    auto it = agents_.find(agentId);
    if (it == agents_.end()) {
        return std::nullopt;
    }
    return it->second;
}

std::optional<AgentMetadata> AIAgentManager::getAgentByPid(int pid) const {
    auto it = pidToAgentId_.find(pid);
    if (it == pidToAgentId_.end()) {
        return std::nullopt;
    }
    return getAgent(it->second);
}

std::vector<AgentMetadata> AIAgentManager::getAllAgents() const {
    std::vector<AgentMetadata> result;
    result.reserve(agents_.size());
    for (const auto& [id, meta] : agents_) {
        result.push_back(meta);
    }
    return result;
}

std::vector<AgentStatistics> AIAgentManager::getAllAgentStatistics() const {
    std::vector<AgentStatistics> result;
    result.reserve(agents_.size());

    for (const auto& [agentId, meta] : agents_) {
        AgentStatistics stats;
        stats.pid = meta.pid;
        stats.agentId = meta.agentId;
        stats.name = meta.name;
        stats.type = meta.type;
        stats.processState = processes_ ? processes_->getProcess(meta.pid)->state : ProcessState::NEW;
        stats.agentState = meta.agentState;
        stats.priority = meta.priority;
        stats.task = (meta.type == AgentType::RESEARCH) ? "Research Task" : "Coding Task";
        stats.tokenUsed = meta.tokenUsed;
        stats.tokenBudget = meta.tokenBudget;

        // Convert permissions to map
        for (const auto& perm : meta.permissions) {
            stats.permissions[toolPermissionToString(perm)] = true;
        }

        // Get CPU time from process
        if (processes_) {
            auto* pcb = processes_->getProcess(meta.pid);
            if (pcb) {
                stats.cpuTime = pcb->cpuTime;
            }
        }

        // Get memory usage from memory manager
        if (memoryManager_) {
            stats.memoryUsage = memoryManager_->getProcessMemoryUsage(meta.pid);
        }

        stats.ipcSent = meta.ipcSent;
        stats.ipcReceived = meta.ipcReceived;
        stats.checkpointCount = meta.checkpointCount;
        stats.failureCount = meta.failureCount;
        stats.recoveryCount = meta.recoveryCount;
        stats.lastCheckpointId = meta.checkpointId;

        result.push_back(stats);
    }
    return result;
}

bool AIAgentManager::updateAgentState(const std::string& agentId, AgentState state) {
    auto it = agents_.find(agentId);
    if (it == agents_.end()) {
        return false;
    }

    AgentState oldState = it->second.agentState;
    it->second.agentState = state;

    recordAgentEvent(EventType::AGENT_STARTED, agentId,
                     agentStateToString(oldState) + " -> " + agentStateToString(state));

    return true;
}

bool AIAgentManager::updateAgentStateByPid(int pid, AgentState state) {
    auto it = pidToAgentId_.find(pid);
    if (it == pidToAgentId_.end()) {
        return false;
    }
    return updateAgentState(it->second, state);
}

bool AIAgentManager::setResourceQuota(const std::string& agentId, const ResourceQuota& quota) {
    auto it = agents_.find(agentId);
    if (it == agents_.end()) {
        return false;
    }
    it->second.quota = quota;
    return true;
}

bool AIAgentManager::checkResourceQuota(const std::string& agentId, ResourceType type, uint64_t amount) {
    auto it = agents_.find(agentId);
    if (it == agents_.end()) {
        return false;
    }

    const auto& quota = it->second.quota;
    switch (type) {
        case ResourceType::CPU:
            return (quota.cpuUsed + amount) <= quota.cpuQuota;
        case ResourceType::MEMORY:
            return (quota.memoryUsed + amount) <= quota.memoryQuota;
        case ResourceType::TOKEN:
            return (quota.tokenUsed + amount) <= quota.tokenBudget;
        case ResourceType::IO:
            return (quota.ioUsed + amount) <= quota.ioLimit;
    }
    return false;
}

void AIAgentManager::chargeResource(const std::string& agentId, ResourceType type, uint64_t amount) {
    auto it = agents_.find(agentId);
    if (it == agents_.end()) {
        return;
    }

    switch (type) {
        case ResourceType::CPU:
            it->second.quota.cpuUsed += amount;
            it->second.cpuCyclesUsed += amount;
            break;
        case ResourceType::MEMORY:
            it->second.quota.memoryUsed += amount;
            break;
        case ResourceType::TOKEN:
            it->second.quota.tokenUsed += amount;
            it->second.tokenUsed += amount;
            break;
        case ResourceType::IO:
            it->second.quota.ioUsed += amount;
            break;
    }
}

bool AIAgentManager::checkToolPermission(const std::string& agentId, ToolPermission permission) {
    auto it = agents_.find(agentId);
    if (it == agents_.end()) {
        return false;
    }

    it->second.toolRequests++;

    const auto& perms = it->second.permissions;
    bool allowed = std::find(perms.begin(), perms.end(), permission) != perms.end();

    if (!allowed) {
        it->second.toolDenials++;
        recordAgentEvent(EventType::AGENT_TOOL_DENIED, agentId,
                         "permission=" + toolPermissionToString(permission));
    } else {
        recordAgentEvent(EventType::AGENT_TOOL_REQUEST, agentId,
                         "permission=" + toolPermissionToString(permission) + " granted");
    }

    return allowed;
}

bool AIAgentManager::grantToolPermission(const std::string& agentId, ToolPermission permission) {
    auto it = agents_.find(agentId);
    if (it == agents_.end()) {
        return false;
    }

    auto& perms = it->second.permissions;
    if (std::find(perms.begin(), perms.end(), permission) == perms.end()) {
        perms.push_back(permission);
    }
    return true;
}

bool AIAgentManager::revokeToolPermission(const std::string& agentId, ToolPermission permission) {
    auto it = agents_.find(agentId);
    if (it == agents_.end()) {
        return false;
    }

    auto& perms = it->second.permissions;
    perms.erase(std::remove(perms.begin(), perms.end(), permission), perms.end());
    return true;
}

std::string AIAgentManager::createCheckpoint(const std::string& agentId, const std::string& progress) {
    auto it = agents_.find(agentId);
    if (it == agents_.end()) {
        return "";
    }

    auto* pcb = processes_ ? processes_->getProcess(it->second.pid) : nullptr;
    if (!pcb) {
        return "";
    }

    std::string checkpointId = generateCheckpointId();
    Checkpoint cp;
    cp.checkpointId = checkpointId;
    cp.createdCycle = clock_ ? clock_->cycle() : 0;
    cp.agentId = agentId;
    cp.pid = it->second.pid;
    cp.progress = progress;
    cp.cpuContext = pcb->context;
    cp.processState = pcb->state;

    // Get page count from memory manager
    if (memoryManager_) {
        cp.pageCount = memoryManager_->getProcessPageCount(it->second.pid);
    }

    // Store checkpoint
    checkpoints_.emplace(checkpointId, cp);
    agentCheckpoints_[agentId].push_back(checkpointId);

    // Update agent metadata
    it->second.checkpointId = checkpointId;
    it->second.checkpointCount++;
    it->second.agentState = AgentState::CHECKPOINTED;

    recordAgentEvent(EventType::AGENT_CHECKPOINTED, agentId,
                     "checkpoint=" + checkpointId + " progress=" + progress +
                     " pc=" + std::to_string(cp.cpuContext.pc));

    return checkpointId;
}

std::optional<Checkpoint> AIAgentManager::getCheckpoint(const std::string& checkpointId) const {
    auto it = checkpoints_.find(checkpointId);
    if (it == checkpoints_.end()) {
        return std::nullopt;
    }
    return it->second;
}

std::vector<Checkpoint> AIAgentManager::getAgentCheckpoints(const std::string& agentId) const {
    std::vector<Checkpoint> result;
    auto it = agentCheckpoints_.find(agentId);
    if (it == agentCheckpoints_.end()) {
        return result;
    }

    for (const auto& cpId : it->second) {
        auto cpIt = checkpoints_.find(cpId);
        if (cpIt != checkpoints_.end()) {
            result.push_back(cpIt->second);
        }
    }
    return result;
}

bool AIAgentManager::failAgent(const std::string& agentId, FailureType type) {
    auto it = agents_.find(agentId);
    if (it == agents_.end()) {
        return false;
    }

    int pid = it->second.pid;

    // Release synchronization resources held by this agent (docs/12 section 37)
    releaseAgentResources(agentId);

    // Mark process as FAILED
    if (processes_) {
        processes_->markFailed(pid);
    }

    // Update agent metadata
    it->second.agentState = AgentState::FAILED;
    it->second.lastFailure = type;
    it->second.failureCount++;

    // IPC cleanup - close channel and remove pending messages (docs/12 section 36)
    if (ipc_) {
        ipc_->cleanupProcess(pid);
    }

    recordAgentEvent(EventType::AGENT_FAILURE, agentId,
                     "type=" + failureTypeToString(type) + " pid=" + std::to_string(pid));

    return true;
}

void AIAgentManager::releaseAgentResources(const std::string& agentId) {
    auto it = agents_.find(agentId);
    if (it == agents_.end() || !sync_) {
        return;
    }

    int pid = it->second.pid;

    // Release any mutexes held by this agent
    auto mutexIds = sync_->listMutexes();
    for (const auto& mutexId : mutexIds) {
        auto mutexOpt = sync_->getMutex(mutexId);
        if (mutexOpt && mutexOpt->ownerPid() == pid) {
            sync_->unlockMutex(mutexId, pid);
        }
    }

    // Note: Semaphores don't have ownership in the same way, but we could
    // track and signal if the agent was waiting on one.
}

bool AIAgentManager::recoverAgent(const std::string& agentId, const std::string& checkpointId) {
    auto it = agents_.find(agentId);
    if (it == agents_.end()) {
        return false;
    }

    // Use latest checkpoint if none specified
    std::string cpId = checkpointId;
    if (cpId.empty()) {
        cpId = it->second.checkpointId;
    }
    if (cpId.empty()) {
        return false; // No checkpoint available
    }

    auto cpOpt = getCheckpoint(cpId);
    if (!cpOpt) {
        return false;
    }
    const Checkpoint& cp = *cpOpt;

    int pid = it->second.pid;

    recordAgentEvent(EventType::AGENT_RECOVERY_STARTED, agentId,
                     "checkpoint=" + cpId);

    // Restore CPU context
    if (!restoreCpuContext(pid, cp.cpuContext)) {
        return false;
    }

    // Restore process state via Process Manager
    if (processes_) {
        // The process is currently FAILED, we need to transition it back
        // First, restore the PCB context and state
        auto* pcb = processes_->getProcess(pid);
        if (pcb) {
            pcb->context = cp.cpuContext;
            // Transition FAILED -> NEW -> READY (docs/12 section 33)
            // We use admit to go NEW -> READY
            // But first we need to reset the state to NEW
            // ProcessManager doesn't have a direct setter, so we transition through
            // the state machine carefully
        }
    }

    // Memory validation - check page count matches (docs/12 section 35)
    if (memoryManager_) {
        uint32_t currentPages = memoryManager_->getProcessPageCount(pid);
        if (currentPages != cp.pageCount) {
            // Memory mapping has changed - log warning but continue
            recordAgentEvent(EventType::AGENT_RECOVERY_STARTED, agentId,
                             "WARNING: page count mismatch (checkpoint=" + std::to_string(cp.pageCount) +
                             " current=" + std::to_string(currentPages) + ")");
        }
    }

    // IPC state: pending messages discarded per policy (docs/12 section 36)
    if (ipc_) {
        // Channel remains, but pending messages are not restored
        // This is the deterministic policy
    }

    // Update agent metadata
    it->second.agentState = AgentState::RECOVERED;
    it->second.recoveryCount++;
    it->second.checkpointId = cpId;

    // If process is FAILED, we need to recreate it or reset its state
    // For now, we'll use the ProcessManager to transition FAILED -> NEW -> READY
    // This requires adding a resetProcess method to ProcessManager

    recordAgentEvent(EventType::AGENT_RECOVERED, agentId,
                     "checkpoint=" + cpId + " progress=" + cp.progress);

    return true;
}

bool AIAgentManager::restoreCpuContext(int pid, const CpuRegisters& context) {
    if (!processes_) {
        return false;
    }

    auto* pcb = processes_->getProcess(pid);
    if (!pcb) {
        return false;
    }

    pcb->context = context;
    return true;
}

std::optional<AgentStatistics> AIAgentManager::getAgentStatistics(const std::string& agentId) const {
    auto it = agents_.find(agentId);
    if (it == agents_.end()) {
        return std::nullopt;
    }

    const auto& meta = it->second;
    AgentStatistics stats;
    stats.pid = meta.pid;
    stats.agentId = meta.agentId;
    stats.name = meta.name;
    stats.type = meta.type;
    stats.processState = processes_ ? processes_->getProcess(meta.pid)->state : ProcessState::NEW;
    stats.agentState = meta.agentState;
    stats.priority = meta.priority;
    stats.task = (meta.type == AgentType::RESEARCH) ? "Research Task" : "Coding Task";
    stats.tokenUsed = meta.tokenUsed;
    stats.tokenBudget = meta.tokenBudget;

    for (const auto& perm : meta.permissions) {
        stats.permissions[toolPermissionToString(perm)] = true;
    }

    if (processes_) {
        auto* pcb = processes_->getProcess(meta.pid);
        if (pcb) {
            stats.cpuTime = pcb->cpuTime;
        }
    }

    if (memoryManager_) {
        stats.memoryUsage = memoryManager_->getProcessMemoryUsage(meta.pid);
    }

    stats.ipcSent = meta.ipcSent;
    stats.ipcReceived = meta.ipcReceived;
    stats.checkpointCount = meta.checkpointCount;
    stats.failureCount = meta.failureCount;
    stats.recoveryCount = meta.recoveryCount;
    stats.lastCheckpointId = meta.checkpointId;

    return stats;
}

void AIAgentManager::reset() {
    agents_.clear();
    pidToAgentId_.clear();
    checkpoints_.clear();
    agentCheckpoints_.clear();
    nextAgentId_ = 1;
    nextCheckpointId_ = 1;
}

} // namespace aios