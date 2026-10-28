#include <aios/interrupt/SystemCallManager.hpp>

#include <aios/io/IOManager.hpp>
#include <aios/ipc/IPCManager.hpp>
#include <aios/process/ProcessManager.hpp>
#include <aios/sync/SynchronizationManager.hpp>

#include <string>

namespace aios {

void SystemCallManager::setSyscallPayload(int pid, const std::string& payload) {
    syscallPayloads_[pid] = payload;
}

SyscallResult SystemCallManager::handleSystemCall(const SyscallRequest& req) {
    // Consume payload for this PID if available
    SyscallRequest reqWithPayload = req;
    auto it = syscallPayloads_.find(req.pid);
    if (it != syscallPayloads_.end()) {
        reqWithPayload.payload = it->second;
        syscallPayloads_.erase(it);
    }

    switch (reqWithPayload.syscallNumber) {
        case SYSCALL_READ:
            return handleRead(reqWithPayload);
        case SYSCALL_WRITE:
            return handleWrite(reqWithPayload);
        case SYSCALL_ALLOCATE:
            return handleAllocate(reqWithPayload);
        case SYSCALL_EXIT:
            return handleExit(reqWithPayload);
        case SYSCALL_IPC_SEND:
            return handleIpcSend(reqWithPayload);
        case SYSCALL_IO_REQUEST:
            return handleIoRequest(reqWithPayload);
        case SYSCALL_LOCK:
            return handleLock(reqWithPayload);
        case SYSCALL_UNLOCK:
            return handleUnlock(reqWithPayload);
        case SYSCALL_WAIT:
            return handleWait(reqWithPayload);
        case SYSCALL_SIGNAL:
            return handleSignal(reqWithPayload);
        case SYSCALL_IPC_RECEIVE:
            return handleIpcReceive(reqWithPayload);
        default:
            return {SyscallStatus::INVALID, 0,
                    "unknown syscall " + std::to_string(reqWithPayload.syscallNumber)};
    }
}

SyscallResult SystemCallManager::handleRead(const SyscallRequest& req) {
    if (!input_) {
        return {SyscallStatus::ERR, 0, "no input attached"};
    }
    if (input_->empty()) {
        return {SyscallStatus::ERR, 0, "input queue empty"};
    }
    const int32_t value = input_->front();
    input_->pop_front();
    if (eventLog_ && clock_) {
        eventLog_->record(EventType::SYSCALL, req.pid, clock_->cycle(),
            "PID " + std::to_string(req.pid) + " READ -> " + std::to_string(value));
    }
    return {SyscallStatus::OK, value, "read"};
}

SyscallResult SystemCallManager::handleWrite(const SyscallRequest& req) {
    if (!output_) {
        return {SyscallStatus::ERR, 0, "no output attached"};
    }
    *output_ << "PID " << req.pid << " WRITE\n";
    if (eventLog_ && clock_) {
        eventLog_->record(EventType::SYSCALL, req.pid, clock_->cycle(),
            "PID " + std::to_string(req.pid) + " WRITE");
    }
    return {SyscallStatus::OK, 0, "write"};
}

SyscallResult SystemCallManager::handleAllocate(const SyscallRequest& req) {
    if (eventLog_ && clock_) {
        eventLog_->record(EventType::SYSCALL, req.pid, clock_->cycle(),
            "PID " + std::to_string(req.pid) + " ALLOCATE (stub)");
    }
    return {SyscallStatus::OK, 0, "allocate stub"};
}

SyscallResult SystemCallManager::handleExit(const SyscallRequest& req) {
    if (processes_) {
        processes_->terminate(req.pid);
    }
    if (eventLog_ && clock_) {
        eventLog_->record(EventType::SYSCALL, req.pid, clock_->cycle(),
            "PID " + std::to_string(req.pid) + " EXIT");
    }
    return {SyscallStatus::OK, 0, "exit"};
}

SyscallResult SystemCallManager::handleIpcSend(const SyscallRequest& req) {
    // Payload format: "receiverPid:type:payload"
    if (req.payload.empty()) {
        return {SyscallStatus::INVALID, 0, "IPC_SEND requires payload: receiverPid:type:payload"};
    }
    // For now, return UNSUPPORTED - full IPC send needs IPCManager integration
    if (eventLog_ && clock_) {
        eventLog_->record(EventType::SYSCALL, req.pid, clock_->cycle(),
            "PID " + std::to_string(req.pid) + " IPC_SEND (stub)");
    }
    return {SyscallStatus::UNSUPPORTED, 0, "IPC_SEND not fully implemented"};
}

SyscallResult SystemCallManager::handleIoRequest(const SyscallRequest& req) {
    if (!ioManager_) {
        return {SyscallStatus::ERR, 0, "no I/O manager attached"};
    }
    if (req.payload.empty()) {
        return {SyscallStatus::INVALID, 0, "IO_REQUEST requires payload: deviceId:operation:data"};
    }

    // Parse payload: "deviceId:operation:data"
    // operation: 0=READ, 1=WRITE
    size_t firstColon = req.payload.find(':');
    if (firstColon == std::string::npos) {
        return {SyscallStatus::INVALID, 0, "IO_REQUEST payload format: deviceId:operation:data"};
    }
    size_t secondColon = req.payload.find(':', firstColon + 1);
    if (secondColon == std::string::npos) {
        return {SyscallStatus::INVALID, 0, "IO_REQUEST payload format: deviceId:operation:data"};
    }

    std::string deviceId = req.payload.substr(0, firstColon);
    std::string opStr = req.payload.substr(firstColon + 1, secondColon - firstColon - 1);
    std::string data = req.payload.substr(secondColon + 1);

    IOOperation op = (opStr == "0" || opStr == "READ") ? IOOperation::READ : IOOperation::WRITE;

    int requestId = ioManager_->requestIO(req.pid, deviceId, op, data);
    if (requestId == INVALID_PID) {
        return {SyscallStatus::ERR, 0, "I/O request failed: device not found or in error state"};
    }

    // Block the process until I/O completes
    if (processes_) {
        processes_->blockProcess(req.pid);
    }

    if (eventLog_ && clock_) {
        eventLog_->record(EventType::SYSCALL, req.pid, clock_->cycle(),
            "PID " + std::to_string(req.pid) + " IO_REQUEST id=" + std::to_string(requestId)
            + " device=" + deviceId + " op=" + (op == IOOperation::READ ? "READ" : "WRITE"));
    }

    return {SyscallStatus::OK, requestId, "I/O request queued"};
}

SyscallResult SystemCallManager::handleLock(const SyscallRequest& req) {
    if (!sync_) {
        return {SyscallStatus::ERR, 0, "no synchronization manager"};
    }
    // Payload format: "mutexId"
    if (req.payload.empty()) {
        return {SyscallStatus::INVALID, 0, "LOCK requires mutex ID in payload"};
    }
    bool success = sync_->lockMutex(req.payload, req.pid);
    if (eventLog_ && clock_) {
        eventLog_->record(EventType::SYSCALL, req.pid, clock_->cycle(),
            "PID " + std::to_string(req.pid) + " LOCK " + req.payload + " " + (success ? "ok" : "failed"));
    }
    return {success ? SyscallStatus::OK : SyscallStatus::ERR, 0, success ? "locked" : "failed"};
}

SyscallResult SystemCallManager::handleUnlock(const SyscallRequest& req) {
    if (!sync_) {
        return {SyscallStatus::ERR, 0, "no synchronization manager"};
    }
    if (req.payload.empty()) {
        return {SyscallStatus::INVALID, 0, "UNLOCK requires mutex ID in payload"};
    }
    int nextWaiter = sync_->unlockMutex(req.payload, req.pid);
    bool success = (nextWaiter != INVALID_PID) || true; // unlockMutex returns next waiter PID or INVALID_PID on error
    if (eventLog_ && clock_) {
        eventLog_->record(EventType::SYSCALL, req.pid, clock_->cycle(),
            "PID " + std::to_string(req.pid) + " UNLOCK " + req.payload + " " + (success ? "ok" : "failed"));
    }
    return {success ? SyscallStatus::OK : SyscallStatus::ERR, nextWaiter, success ? "unlocked" : "failed"};
}

SyscallResult SystemCallManager::handleWait(const SyscallRequest& req) {
    if (!sync_) {
        return {SyscallStatus::ERR, 0, "no synchronization manager"};
    }
    if (req.payload.empty()) {
        return {SyscallStatus::INVALID, 0, "WAIT requires semaphore ID in payload"};
    }
    int nextWaiter = INVALID_PID;
    bool success = sync_->waitSemaphore(req.payload, req.pid);
    if (!success) {
        if (processes_) {
            processes_->blockProcess(req.pid);
        }
    }
    if (eventLog_ && clock_) {
        eventLog_->record(EventType::SYSCALL, req.pid, clock_->cycle(),
            "PID " + std::to_string(req.pid) + " WAIT " + req.payload + " " + (success ? "ok" : "blocked"));
    }
    return {success ? SyscallStatus::OK : SyscallStatus::ERR, nextWaiter, success ? "acquired" : "blocked"};
}

SyscallResult SystemCallManager::handleSignal(const SyscallRequest& req) {
    if (!sync_) {
        return {SyscallStatus::ERR, 0, "no synchronization manager"};
    }
    if (req.payload.empty()) {
        return {SyscallStatus::INVALID, 0, "SIGNAL requires semaphore ID in payload"};
    }
    int nextWaiter = sync_->signalSemaphore(req.payload);
    bool success = (nextWaiter != INVALID_PID) || true; // signalSemaphore returns next waiter PID or INVALID_PID if none
    if (eventLog_ && clock_) {
        eventLog_->record(EventType::SYSCALL, req.pid, clock_->cycle(),
            "PID " + std::to_string(req.pid) + " SIGNAL " + req.payload + " " + (success ? "ok" : "failed"));
    }
    return {success ? SyscallStatus::OK : SyscallStatus::ERR, nextWaiter, success ? "signaled" : "failed"};
}

SyscallResult SystemCallManager::handleIpcReceive(const SyscallRequest& req) {
    // Payload format: "blocking" (optional, default true)
    bool blocking = true;
    if (!req.payload.empty() && req.payload == "0") {
        blocking = false;
    }
    // For now, return UNSUPPORTED - full IPC receive needs IPCManager integration
    if (eventLog_ && clock_) {
        eventLog_->record(EventType::SYSCALL, req.pid, clock_->cycle(),
            "PID " + std::to_string(req.pid) + " IPC_RECEIVE blocking=" + (blocking ? "true" : "false") + " (stub)");
    }
    return {SyscallStatus::UNSUPPORTED, 0, "IPC_RECEIVE not fully implemented"};
}

} // namespace aios