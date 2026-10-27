#include <aios/sync/SynchronizationManager.hpp>

#include <aios/core/SimulationClock.hpp>

#include <algorithm>
#include <sstream>

namespace aios {

bool SynchronizationManager::createMutex(const std::string& id) {
    if (mutexes_.find(id) != mutexes_.end()) {
        return false;
    }
    mutexes_.emplace(id, Mutex(id));
    recordMutexEvent(EventType::MUTEX_CREATED, id, INVALID_PID);
    return true;
}

bool SynchronizationManager::lockMutex(const std::string& id, int pid) {
    auto it = mutexes_.find(id);
    if (it == mutexes_.end()) {
        recordMutexEvent(EventType::SYNCHRONIZATION_ERROR, id, pid);
        return false;
    }

    if (it->second.tryLock(pid)) {
        recordMutexEvent(EventType::MUTEX_LOCKED, id, pid);
        return true;
    }

    // Lock not available - enqueue as waiter
    it->second.enqueueWaiter(pid);
    recordMutexEvent(EventType::PROCESS_BLOCKED_ON_MUTEX, id, pid);
    return false;
}

int SynchronizationManager::unlockMutex(const std::string& id, int pid) {
    auto it = mutexes_.find(id);
    if (it == mutexes_.end()) {
        recordMutexEvent(EventType::SYNCHRONIZATION_ERROR, id, pid);
        return INVALID_PID;
    }

    int nextWaiter = it->second.unlock(pid);
    if (nextWaiter == INVALID_PID && it->second.ownerPid() != INVALID_PID) {
        // Not the owner or already unlocked
        recordMutexEvent(EventType::SYNCHRONIZATION_ERROR, id, pid);
        return INVALID_PID;
    }

    recordMutexEvent(EventType::MUTEX_UNLOCKED, id, pid);

    if (nextWaiter != INVALID_PID) {
        // The next waiter will be granted the lock when woken
        // We record the wakeup event here; the actual lock grant happens in ProcessManager
        recordMutexEvent(EventType::PROCESS_WAKEUP, id, nextWaiter);
    }
    return nextWaiter;
}

std::optional<Mutex> SynchronizationManager::getMutex(const std::string& id) const {
    auto it = mutexes_.find(id);
    if (it != mutexes_.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::vector<std::string> SynchronizationManager::listMutexes() const {
    std::vector<std::string> ids;
    for (const auto& [id, m] : mutexes_) {
        ids.push_back(id);
    }
    return ids;
}

bool SynchronizationManager::createSemaphore(const std::string& id, int initialCount) {
    if (semaphores_.find(id) != semaphores_.end()) {
        return false;
    }
    semaphores_.emplace(id, Semaphore(id, initialCount));
    recordSemaphoreEvent(EventType::SEMAPHORE_CREATED, id, INVALID_PID);
    return true;
}

bool SynchronizationManager::waitSemaphore(const std::string& id, int pid) {
    auto it = semaphores_.find(id);
    if (it == semaphores_.end()) {
        recordSemaphoreEvent(EventType::SYNCHRONIZATION_ERROR, id, pid);
        return false;
    }

    if (it->second.wait(pid)) {
        recordSemaphoreEvent(EventType::SEMAPHORE_WAIT, id, pid);
        return true;
    }

    // Count < 0, process blocked
    recordSemaphoreEvent(EventType::SEMAPHORE_WAIT, id, pid);
    return false;
}

int SynchronizationManager::signalSemaphore(const std::string& id) {
    auto it = semaphores_.find(id);
    if (it == semaphores_.end()) {
        return INVALID_PID;
    }

    int nextWaiter = it->second.signal();
    recordSemaphoreEvent(EventType::SEMAPHORE_SIGNAL, id, INVALID_PID);

    if (nextWaiter != INVALID_PID) {
        recordSemaphoreEvent(EventType::PROCESS_WAKEUP, id, nextWaiter);
    }
    return nextWaiter;
}

std::optional<Semaphore> SynchronizationManager::getSemaphore(const std::string& id) const {
    auto it = semaphores_.find(id);
    if (it != semaphores_.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::vector<std::string> SynchronizationManager::listSemaphores() const {
    std::vector<std::string> ids;
    for (const auto& [id, s] : semaphores_) {
        ids.push_back(id);
    }
    return ids;
}

bool SynchronizationManager::detectDeadlock() {
    // Build wait-for graph: nodes are process PIDs and mutex/semaphore IDs
    // Edges: PID -> mutex/sem ID (process waits for resource)
    //        mutex/sem ID -> PID (resource held by process)
    // A cycle indicates deadlock.

    std::map<std::string, std::vector<std::string>> graph;

    // Add mutex wait edges
    for (const auto& [id, m] : mutexes_) {
        if (m.isLocked()) {
            int owner = m.ownerPid();
            if (owner != INVALID_PID) {
                std::string ownerNode = "P" + std::to_string(owner);
                std::string mutexNode = "M" + id; // "M" prefix for mutex
                graph[mutexNode].push_back(ownerNode); // mutex held by owner
            }
            for (int waiter : m.waitingQueue()) {
                std::string waiterNode = "P" + std::to_string(waiter);
                std::string mutexNode = "M" + id;
                graph[waiterNode].push_back(mutexNode); // waiter waits for mutex
            }
        }
    }

    // Add semaphore wait edges
    for (const auto& [id, s] : semaphores_) {
        if (s.count() < 0) { // has waiters
            for (int waiter : s.waitingQueue()) {
                std::string waiterNode = "P" + std::to_string(waiter);
                std::string semNode = "S" + id; // "S" prefix for semaphore
                graph[waiterNode].push_back(semNode); // waiter waits for semaphore
            }
        }
        // Note: semaphores don't have strict ownership, so no "holds" edge
    }

    // Cycle detection via DFS
    std::map<std::string, int> visited; // 0=unvisited, 1=visiting, 2=visited
    std::vector<std::string> path;
    lastDeadlockCycle_.clear();

    for (const auto& [node, _] : graph) {
        if (visited[node] == 0) {
            if (hasCycleDFS(node, graph, visited, path)) {
                record(EventType::DEADLOCK_DETECTED, INVALID_PID, "Deadlock detected: " + 
                       joinCycle(lastDeadlockCycle_));
                return true;
            }
        }
    }
    return false;
}

std::vector<std::string> SynchronizationManager::getDeadlockCycle() const {
    return lastDeadlockCycle_;
}

bool SynchronizationManager::hasCycleDFS(const std::string& node,
                                         const std::map<std::string, std::vector<std::string>>& graph,
                                         std::map<std::string, int>& visited,
                                         std::vector<std::string>& path) {
    visited[node] = 1; // visiting
    path.push_back(node);

    auto it = graph.find(node);
    if (it != graph.end()) {
        for (const std::string& neighbor : it->second) {
            if (visited[neighbor] == 0) {
                if (hasCycleDFS(neighbor, graph, visited, path)) {
                    return true;
                }
            } else if (visited[neighbor] == 1) {
                // Found a cycle - extract it
                auto cycleStart = std::find(path.begin(), path.end(), neighbor);
                if (cycleStart != path.end()) {
                    lastDeadlockCycle_.assign(cycleStart, path.end());
                    lastDeadlockCycle_.push_back(neighbor); // close the cycle
                }
                return true;
            }
        }
    }

    visited[node] = 2; // visited
    path.pop_back();
    return false;
}

void SynchronizationManager::cleanupProcess(int pid) {
    // Release all mutexes owned by this process
    for (auto& [id, m] : mutexes_) {
        if (m.isLocked() && m.ownerPid() == pid) {
            int nextWaiter = m.forceRelease();
            recordMutexEvent(EventType::MUTEX_UNLOCKED, id, pid);
            if (nextWaiter != INVALID_PID) {
                recordMutexEvent(EventType::PROCESS_WAKEUP, id, nextWaiter);
            }
        }
        m.removeWaiter(pid);
    }

    // Remove from all semaphore waiting queues
    for (auto& [id, s] : semaphores_) {
        s.removeWaiter(pid);
    }
}

void SynchronizationManager::reset() {
    mutexes_.clear();
    semaphores_.clear();
    lastDeadlockCycle_.clear();
}

void SynchronizationManager::record(EventType type, int pid, const std::string& detail) {
    if (eventLog_ && clock_) {
        eventLog_->record(type, pid, clock_->cycle(), detail);
    }
}

void SynchronizationManager::recordMutexEvent(EventType type, const std::string& mutexId, int pid) {
    std::ostringstream oss;
    oss << "Mutex " << mutexId;
    if (pid != INVALID_PID) {
        oss << " by PID " << pid;
    }
    record(type, pid, oss.str());
}

void SynchronizationManager::recordSemaphoreEvent(EventType type, const std::string& semId, int pid) {
    std::ostringstream oss;
    oss << "Semaphore " << semId;
    if (pid != INVALID_PID) {
        oss << " by PID " << pid;
    }
    record(type, pid, oss.str());
}

std::string SynchronizationManager::joinCycle(const std::vector<std::string>& cycle) {
    std::ostringstream oss;
    for (size_t i = 0; i < cycle.size(); ++i) {
        if (i > 0) oss << " -> ";
        oss << cycle[i];
    }
    return oss.str();
}

} // namespace aios