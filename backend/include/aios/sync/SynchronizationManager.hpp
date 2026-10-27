#pragma once

#include <aios/core/EventLog.hpp>
#include <aios/core/types.hpp>
#include <aios/sync/Mutex.hpp>
#include <aios/sync/Semaphore.hpp>

#include <deque>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace aios {

class ProcessManager;
class SimulationClock;

// ---------------------------------------------------------------------------
// Synchronization Manager (docs/09 sections 17, 37).
// Central authority for mutexes and semaphores. Handles creation, lock/unlock,
// wait/signal, deadlock detection, and event generation.
// ---------------------------------------------------------------------------
class SynchronizationManager {
public:
    SynchronizationManager() = default;

    void setEventLog(EventLog* log) { eventLog_ = log; }
    void setProcessManager(ProcessManager* pm) { processes_ = pm; }
    void setClock(SimulationClock* clock) { clock_ = clock; }

    // ----- Mutex operations (docs/09 sections 8-11, 23) -----

    // Creates a new mutex with the given ID. Returns false if ID already exists.
    bool createMutex(const std::string& id);

    // Attempts to lock mutex `id` for process `pid`.
    // Returns:
    //   true  -> lock acquired, process continues
    //   false -> lock not available, process should be blocked (caller handles state change)
    bool lockMutex(const std::string& id, int pid);

    // Unlocks mutex `id` for process `pid` (must be owner).
    // Returns the PID of the next waiter to wake, or INVALID_PID if none/error.
    int unlockMutex(const std::string& id, int pid);

    // Returns mutex state for dashboard/debug.
    std::optional<Mutex> getMutex(const std::string& id) const;

    std::vector<std::string> listMutexes() const;

    // ----- Semaphore operations (docs/09 sections 12-14, 23) -----

    // Creates a new semaphore with the given ID and initial count.
    // Returns false if ID already exists.
    bool createSemaphore(const std::string& id, int initialCount);

    // WAIT on semaphore `id` for process `pid`.
    // Returns:
    //   true  -> acquired (count >= 0)
    //   false -> blocked (count < 0), process should be blocked
    bool waitSemaphore(const std::string& id, int pid);

    // SIGNAL on semaphore `id`.
    // Returns the PID of the next waiter to wake, or INVALID_PID if none.
    int signalSemaphore(const std::string& id);

    // Returns semaphore state for dashboard/debug.
    std::optional<Semaphore> getSemaphore(const std::string& id) const;

    std::vector<std::string> listSemaphores() const;

    // ----- Deadlock detection (docs/09 sections 29-31) -----

    // Builds a wait-for graph and checks for cycles.
    // Returns true if a deadlock is detected.
    bool detectDeadlock();

    // Returns the detected cycle as a vector of PIDs/mutex IDs, empty if none.
    std::vector<std::string> getDeadlockCycle() const;

    // ----- Cleanup on process termination -----

    // Releases all mutexes owned by `pid` and removes `pid` from all waiting queues.
    void cleanupProcess(int pid);

    // ----- Statistics -----

    size_t mutexCount() const { return mutexes_.size(); }
    size_t semaphoreCount() const { return semaphores_.size(); }

    void reset();

private:
    void record(EventType type, int pid, const std::string& detail);
    void recordMutexEvent(EventType type, const std::string& mutexId, int pid);
    void recordSemaphoreEvent(EventType type, const std::string& semId, int pid);
    std::string joinCycle(const std::vector<std::string>& cycle);

    // Deadlock detection uses a simple resource-wait graph:
    // nodes = processes + mutexes/semaphores, edges = holds/waits.
    // Cycle detection via DFS.
    bool hasCycleDFS(const std::string& node,
                     const std::map<std::string, std::vector<std::string>>& graph,
                     std::map<std::string, int>& visited,
                     std::vector<std::string>& path);

    std::map<std::string, Mutex> mutexes_;
    std::map<std::string, Semaphore> semaphores_;

    EventLog* eventLog_ = nullptr;
    ProcessManager* processes_ = nullptr;
    SimulationClock* clock_ = nullptr;

    std::vector<std::string> lastDeadlockCycle_;
};

} // namespace aios