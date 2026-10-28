#pragma once

#include <aios/core/EventLog.hpp>
#include <aios/core/SimulationClock.hpp>
#include <aios/core/types.hpp>

#include <deque>
#include <map>
#include <ostream>
#include <string>

namespace aios {

class ProcessManager;
class SynchronizationManager;
class IOManager;
class SimulationClock;

// ---------------------------------------------------------------------------
// System call manager (docs/05 section 4).
// Stage I services: 1 READ, 2 WRITE, 3 ALLOCATE (stub), 4 EXIT.
// 5 IPC_SEND, 6 IO_REQUEST, 11 IPC_RECEIVE.
// 7 LOCK, 8 UNLOCK, 9 WAIT, 10 SIGNAL for Synchronization (Week 6).
// [decision D5]
// ---------------------------------------------------------------------------
class SystemCallManager {
public:
    SystemCallManager() = default;

    void setEventLog(EventLog* log) { eventLog_ = log; }
    void setProcessManager(ProcessManager* m) { processes_ = m; }
    void setSynchronizationManager(SynchronizationManager* m) { sync_ = m; }
    void setIOManager(IOManager* m) { ioManager_ = m; }
    void setClock(SimulationClock* clock) { clock_ = clock; }
    void setInputQueue(std::deque<int32_t>* input) { input_ = input; }
    void setOutputStream(std::ostream* output) { output_ = output; }

    // Store syscall payload for a PID (set before the process executes SYSCALL)
    void setSyscallPayload(int pid, const std::string& payload);

    SyscallResult handleSystemCall(const SyscallRequest& req);

private:
    EventLog* eventLog_ = nullptr;
    ProcessManager* processes_ = nullptr;
    SynchronizationManager* sync_ = nullptr;
    IOManager* ioManager_ = nullptr;
    SimulationClock* clock_ = nullptr;
    std::deque<int32_t>* input_ = nullptr;
    std::ostream* output_ = nullptr;

    // Per-PID syscall payloads (set via setSyscallPayload, consumed on handleSystemCall)
    std::map<int, std::string> syscallPayloads_;

    // Syscall handlers
    SyscallResult handleRead(const SyscallRequest& req);
    SyscallResult handleWrite(const SyscallRequest& req);
    SyscallResult handleAllocate(const SyscallRequest& req);
    SyscallResult handleExit(const SyscallRequest& req);
    SyscallResult handleIpcSend(const SyscallRequest& req);
    SyscallResult handleIoRequest(const SyscallRequest& req);
    SyscallResult handleLock(const SyscallRequest& req);
    SyscallResult handleUnlock(const SyscallRequest& req);
    SyscallResult handleWait(const SyscallRequest& req);
    SyscallResult handleSignal(const SyscallRequest& req);
    SyscallResult handleIpcReceive(const SyscallRequest& req);
};

} // namespace aios