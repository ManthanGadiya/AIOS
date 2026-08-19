#include <aios/interrupt/SystemCallManager.hpp>

#include <aios/process/ProcessManager.hpp>

#include <string>

namespace aios {

SyscallResult SystemCallManager::handleSystemCall(const SyscallRequest& req) {
    switch (req.syscallNumber) {
        case SYSCALL_READ: {
            if (!input_) {
                return {SyscallStatus::ERROR, 0, "no input attached"};
            }
            if (input_->empty()) {
                return {SyscallStatus::ERROR, 0, "input queue empty"};
            }
            const int32_t value = input_->front();
            input_->pop_front();
            return {SyscallStatus::OK, value, "read"};
        }
        case SYSCALL_WRITE: {
            if (!output_) {
                return {SyscallStatus::ERROR, 0, "no output attached"};
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
        case SYSCALL_IPC_SEND:
        case SYSCALL_IO_REQUEST: {
            // IPC is Week 6, I/O is Week 7 (docs/05 section 4, decision D5).
            return {SyscallStatus::UNSUPPORTED, 0, "not implemented yet"};
        }
        default: {
            return {SyscallStatus::INVALID, 0,
                    "unknown syscall " + std::to_string(req.syscallNumber)};
        }
    }
}

} // namespace aios