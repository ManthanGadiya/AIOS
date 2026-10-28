#pragma once

#include <aios/core/EventLog.hpp>
#include <aios/core/SimulationClock.hpp>
#include <aios/core/types.hpp>
#include <aios/process/ProcessManager.hpp>

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace aios {

class MemoryManager;
class SynchronizationManager;
class IPCManager;
class ProcessManager;

// ---------------------------------------------------------------------------
// AI Agent Manager (docs/12 section 5, 50).
// Manages AI agent metadata, checkpoints, failure/recovery, tool permissions,
// and resource quotas. Integrates with Process Manager, Scheduler, IPC,
// Synchronization, and Memory Manager.
// ---------------------------------------------------------------------------
class AIAgentManager {
public:
    AIAgentManager() = default;

    void setEventLog(EventLog* log) { eventLog_ = log; }
    void setProcessManager(ProcessManager* pm) { processes_ = pm; }
    void setMemoryManager(MemoryManager* mm) { memoryManager_ = mm; }
    void setSynchronizationManager(SynchronizationManager* sync) { sync_ = sync; }
    void setIPCManager(IPCManager* ipc) { ipc_ = ipc; }
    void setClock(SimulationClock* clock) { clock_ = clock; }

    // ----- Agent Creation (docs/12 section 11) -----

    // Creates an AI agent as an OS process with AI metadata.
    // Returns the PID on success, INVALID_PID on failure.
    int createAgent(const std::string& name, AgentType type, int priority,
                    const std::vector<int32_t>& program, uint32_t base,
                    uint64_t tokenBudget = 1000,
                    const std::vector<ToolPermission>& permissions = {});

    // ----- Agent Query -----

    std::optional<AgentMetadata> getAgent(const std::string& agentId) const;
    std::optional<AgentMetadata> getAgentByPid(int pid) const;
    std::vector<AgentMetadata> getAllAgents() const;
    std::vector<AgentStatistics> getAllAgentStatistics() const;

    // ----- Agent State Management (docs/12 section 13) -----

    bool updateAgentState(const std::string& agentId, AgentState state);
    bool updateAgentStateByPid(int pid, AgentState state);

    // ----- Resource Quotas (docs/12 section 23-26) -----

    bool setResourceQuota(const std::string& agentId, const ResourceQuota& quota);
    bool checkResourceQuota(const std::string& agentId, ResourceType type, uint64_t amount = 1);
    void chargeResource(const std::string& agentId, ResourceType type, uint64_t amount = 1);

    // ----- Tool Permissions (docs/12 section 27-28) -----

    bool checkToolPermission(const std::string& agentId, ToolPermission permission);
    bool grantToolPermission(const std::string& agentId, ToolPermission permission);
    bool revokeToolPermission(const std::string& agentId, ToolPermission permission);

    // ----- Checkpointing (docs/12 section 31) -----

    // Creates a checkpoint of the agent's current state.
    // Returns checkpoint ID on success, empty string on failure.
    std::string createCheckpoint(const std::string& agentId, const std::string& progress);

    // Returns the checkpoint if it exists.
    std::optional<Checkpoint> getCheckpoint(const std::string& checkpointId) const;

    // Lists all checkpoints for an agent.
    std::vector<Checkpoint> getAgentCheckpoints(const std::string& agentId) const;

    // ----- Failure and Recovery (docs/12 section 29-33) -----

    // Simulates an agent failure (docs/12 section 43).
    // Transitions agent to FAILED, releases sync resources, records failure.
    bool failAgent(const std::string& agentId, FailureType type = FailureType::SIMULATED_CRASH);

    // Recovers an agent from its latest checkpoint (docs/12 section 33).
    // Restores CPU context, process state, and agent metadata.
    // Returns true on success.
    bool recoverAgent(const std::string& agentId, const std::string& checkpointId = "");

    // ----- Statistics and Monitoring (docs/12 section 39) -----

    std::optional<AgentStatistics> getAgentStatistics(const std::string& agentId) const;

    void reset();

private:
    // Internal storage
    std::map<std::string, AgentMetadata> agents_;        // agentId -> metadata
    std::map<int, std::string> pidToAgentId_;            // pid -> agentId
    std::map<std::string, Checkpoint> checkpoints_;      // checkpointId -> checkpoint
    std::map<std::string, std::vector<std::string>> agentCheckpoints_; // agentId -> checkpointIds

    // Helper methods
    std::string generateAgentId();
    std::string generateCheckpointId();
    void recordAgentEvent(EventType type, const std::string& agentId, const std::string& detail);
    bool restoreCpuContext(int pid, const CpuRegisters& context);
    void releaseAgentResources(const std::string& agentId);

    // Dependencies
    EventLog* eventLog_ = nullptr;
    ProcessManager* processes_ = nullptr;
    MemoryManager* memoryManager_ = nullptr;
    SynchronizationManager* sync_ = nullptr;
    IPCManager* ipc_ = nullptr;
    SimulationClock* clock_ = nullptr;

    int nextAgentId_ = 1;
    int nextCheckpointId_ = 1;
};

} // namespace aios