#include <aios/interrupt/SystemCallManager.hpp>

#include <aios/process/ProcessManager.hpp>
#include <aios/sync/SynchronizationManager.hpp>

#include <string>

namespace aios {

SyscallResult SystemCallManager::handleSystemCall(const SyscallRequest& req) {
    switch (req.syscallNumber) {
        case SYSCALL_READ: {
            if (!input_) {
                return {SyscallStatus::ERR, 0, "no input attached"};
            }
            if (input_->empty()) {
                return {SyscallStatus::ERR, 0, "input queue empty"};
            }
            const int32_t value = input_->front();
            input_->pop_front();
            return {SyscallStatus::OK, value, "read"};
        }
        case SYSCALL_WRITE: {
            if (!output_) {
                return {SyscallStatus::ERR, 0, "no output attached"};
            }
            *output_ << "PID " << req.pid << " WRITE\n";
            return {SyscallStatus::OK, 0, "write"};
        }
        case SYSCALL_ALLOCATE: {
            return {SyscallStatus::OK, 0, "allocate stub"};
        }
        case SYSCALL_EXIT: {
            if (processes_) {
                processes_->terminate(req.pid);
            }
            return {SyscallStatus::OK, 0, "exit"};
        }
        // IPC syscalls (Week 6) - not yet fully implemented
        case SYSCALL_IPC_SEND:
        case SYSCALL_IPC_RECEIVE: {
            // Recognized but not yet implemented (docs/05 section 4, decision D5).
            return {SyscallStatus::UNSUPPORTED, 0, "not implemented yet"};
        }
        case SYSCALL_IO_REQUEST: {
            // I/O is Week 7 (docs/05 section 4, decision D5).
            return {SyscallStatus::UNSUPPORTED, 0, "not implemented yet"};
        }
        // Synchronization syscalls (Week 6)
        case SYSCALL_LOCK: {
            if (!sync_) {
                return {SyscallStatus::ERR, 0, "no synchronization manager"};
            }
            return {SyscallStatus::UNSUPPORTED, 0, "LOCK requires mutex ID"};
        }
        case SYSCALL_UNLOCK: {
            if (!sync_) {
                return {SyscallStatus::ERR, 0, "no synchronization manager"};
            }
            return {SyscallStatus::UNSUPPORTED, 0, "UNLOCK requires mutex ID"};
        }
        case SYSCALL_WAIT: {
            if (!sync_) {
                return {SyscallStatus::ERR, 0, "no synchronization manager"};
            }
            return {SyscallStatus::UNSUPPORTED, 0, "WAIT requires semaphore ID"};
        }
        case SYSCALL_SIGNAL: {
            if (!sync_) {
                return {SyscallStatus::ERR, 0, "no synchronization manager"};
            }
            return {SyscallStatus::UNSUPPORTED, 0, "SIGNAL requires semaphore ID"};
        }
        default: {
            return {SyscallStatus::INVALID, 0,
                    "unknown syscall " + std::to_string(req.syscallNumber)};
        }
    }
}

} // namespace aios