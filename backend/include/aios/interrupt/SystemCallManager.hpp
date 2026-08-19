#pragma once

#include <aios/core/EventLog.hpp>
#include <aios/core/types.hpp>

#include <deque>
#include <ostream>

namespace aios {

class ProcessManager;

// ---------------------------------------------------------------------------
// System call manager (docs/05 section 4).
// Stage I services: 1 READ, 2 WRITE, 3 ALLOCATE (stub), 4 EXIT.
// 5 IPC_SEND and 6 IO_REQUEST are recognized but return UNSUPPORTED
// (IPC is Week 6, I/O is Week 7).  [decision D5]
// ---------------------------------------------------------------------------
class SystemCallManager {
public:
    SystemCallManager() = default;

    void setEventLog(EventLog* log) { eventLog_ = log; }
    void setProcessManager(ProcessManager* m) { processes_ = m; }
    void setInputQueue(std::deque<int32_t>* input) { input_ = input; }
    void setOutputStream(std::ostream* output) { output_ = output; }

    SyscallResult handleSystemCall(const SyscallRequest& req);

private:
    EventLog* eventLog_ = nullptr;
    ProcessManager* processes_ = nullptr;
    std::deque<int32_t>* input_ = nullptr;
    std::ostream* output_ = nullptr;
};

} // namespace aios