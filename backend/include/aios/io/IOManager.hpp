#pragma once

#include <aios/core/EventLog.hpp>
#include <aios/core/SimulationClock.hpp>

#include <deque>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace aios {

class ProcessManager;
class SimulationClock;
class EventLog;

enum class EventType; // Forward declaration

// ---------------------------------------------------------------------------
// I/O Device Types (docs/11 sections 64-77).
// ---------------------------------------------------------------------------
enum class DeviceType : int32_t {
    DISK = 0,
    PRINTER = 1,
    KEYBOARD = 2,
    NETWORK = 3
};

enum class DeviceState : int32_t {
    AVAILABLE = 0,
    BUSY = 1,
    ERROR = 2
};

std::string deviceTypeToString(DeviceType type);
std::string deviceStateToString(DeviceState state);

std::string ioOperationToString(IOOperation op);

// ---------------------------------------------------------------------------
// I/O Request States (docs/11 section 32).
// ---------------------------------------------------------------------------
enum class IORequestState : int32_t {
    CREATED = 0,
    QUEUED = 1,
    PROCESSING = 2,
    COMPLETED = 3,
    ERROR = 4
};

std::string ioRequestStateToString(IORequestState state);

// ---------------------------------------------------------------------------
// I/O Request (docs/11 section 6).
// ---------------------------------------------------------------------------
struct IORequest {
    int id = 0;
    int pid = INVALID_PID;
    int deviceId = 0;
    IOOperation operation = IOOperation::READ;
    std::string data; // Buffer reference or data payload
    uint64_t requestedAt = 0;
    uint64_t timestamp = 0; // Simulation cycle when request was made
    IORequestState state = IORequestState::CREATED;
    int deviceIdRef = -1; // Reference to target device
    int64_t completionCycle = 0; // When I/O will complete

    std::string toString() const;
};

// ---------------------------------------------------------------------------
// Buffer States (docs/11 section 22).
// ---------------------------------------------------------------------------
enum class BufferState : int32_t {
    EMPTY = 0,
    PARTIAL = 1,
    FULL = 2
};

std::string bufferStateToString(BufferState state);

// ---------------------------------------------------------------------------
// Buffer (docs/11 sections 18-25).
// ---------------------------------------------------------------------------
class Buffer {
public:
    Buffer() = default;
    explicit Buffer(std::string id, size_t capacity);

    const std::string& id() const { return id_; }
    size_t capacity() const { return capacity_; }
    size_t size() const { return data_.size(); }
    BufferState state() const { return state_; }
    const std::string& data() const { return data_; }

    // Write data to buffer (output buffering)
    // Returns true on success, false if buffer full
    bool write(const std::string& input);

    // Read data from buffer (input buffering)
    // Returns empty string if buffer empty
    std::string read(size_t count);

    // Check if buffer has space
    bool hasSpace(size_t count) const;

    // Check if buffer has data
    bool hasData() const;

    void clear();

    std::string toString() const;

private:
    std::string id_;
    size_t capacity_ = 0;
    std::string data_;
    BufferState state_ = BufferState::EMPTY;

    void updateState();
};

// ---------------------------------------------------------------------------
// Spool Job (docs/11 section 28).
// ---------------------------------------------------------------------------
struct SpoolJob {
    int id = 0;
    int ownerPid = INVALID_PID;
    std::string deviceId;
    IOOperation operation;
    std::string data;
    std::string status; // QUEUED, PROCESSING, COMPLETED, ERROR
    uint64_t createdAt = 0;
    uint64_t completedAt = 0;

    std::string toString() const;
};

// ---------------------------------------------------------------------------
// Simulated Device (docs/11 sections 3-5, 85-98).
// ---------------------------------------------------------------------------
class Device {
public:
    Device() = default;
    Device(std::string id, DeviceType type, int processingTimeCycles = 10);

    const std::string& id() const { return id_; }
    DeviceType type() const { return type_; }
    DeviceState state() const { return state_; }
    const std::optional<IORequest>& currentRequest() const { return currentRequest_; }
    const std::deque<IORequest>& queue() const { return queue_; }
    std::deque<IORequest>& queue() { return queue_; }
    int processingTimeCycles() const { return processingTimeCycles_; }
    int currentProcessingCycle() const { return currentProcessingCycle_; }

    // Check if device can accept a request
    bool canAcceptRequest() const;

    // Add request to device queue
    void enqueueRequest(const IORequest& req);

    // Get next request from queue
    std::optional<IORequest> dequeueRequest();

    // Start processing current request
    bool startProcessing();

    // Advance processing by one cycle
    // Returns true if processing completed
    bool tick();

    // Get current request completion status
    std::optional<IORequest> checkCompletion();

    // Device error handling
    void setError(const std::string& reason);
    void recover();

    // Device statistics
    uint64_t completedRequests() const { return completedRequests_; }
    uint64_t failedRequests() const { return failedRequests_; }
    uint64_t totalProcessingCycles() const { return totalProcessingCycles_; }

    std::string toString() const;

private:
    std::string id_;
    DeviceType type_;
    DeviceState state_ = DeviceState::AVAILABLE;
    std::deque<IORequest> queue_;
    std::optional<IORequest> currentRequest_;
    int processingTimeCycles_ = 10; // Cycles to complete one I/O
    int currentProcessingCycle_ = 0;

    // Statistics
    uint64_t completedRequests_ = 0;
    uint64_t failedRequests_ = 0;
    uint64_t totalProcessingCycles_ = 0;

    void setState(DeviceState newState);
};

// ---------------------------------------------------------------------------
// I/O Manager (docs/11 sections 12, 31, 49).
// Central authority for I/O operations: devices, requests, buffers, spooling.
// ---------------------------------------------------------------------------
class IOManager {
public:
    IOManager() = default;

    void setEventLog(EventLog* log) { eventLog_ = log; }
    void setProcessManager(ProcessManager* pm) { processes_ = pm; }
    void setClock(SimulationClock* clock) { clock_ = clock; }

    // ----- Device Management (docs/11 section 3) -----
    bool createDevice(const std::string& id, DeviceType type, int processingTimeCycles = 10);
    std::optional<Device> getDevice(const std::string& id) const;
    std::vector<std::string> listDevices() const;
    size_t deviceCount() const { return devices_.size(); }

    // ----- I/O Request Operations (docs/11 sections 6, 8) -----
    // Returns request ID on success, INVALID_PID on failure
    int requestIO(int pid, const std::string& deviceId, IOOperation op, const std::string& data = "");

    // Blocking I/O: process waits for completion
    // Non-blocking: returns immediately, process polls or gets interrupt
    std::optional<IORequest> checkCompletion(int pid);

    // ----- Device Queue (docs/11 sections 11, 12) -----
    bool hasPendingRequests(const std::string& deviceId) const;
    std::vector<IORequest> getDeviceQueue(const std::string& deviceId) const;
    size_t queueSize(const std::string& deviceId) const;

    // ----- Buffer Management (docs/11 sections 18-25) -----
    bool createBuffer(const std::string& id, size_t capacity);
    std::optional<Buffer> getBuffer(const std::string& id) const;
    std::vector<std::string> listBuffers() const;
    bool writeBuffer(const std::string& id, const std::string& data);
    std::string readBuffer(const std::string& id, size_t count);
    bool hasBufferSpace(const std::string& id, size_t count) const;
    bool hasBufferData(const std::string& id) const;

    // ----- Spooling (docs/11 sections 26-29) -----
    int createSpoolJob(int pid, const std::string& deviceId, IOOperation op, const std::string& data);
    bool processSpoolQueue(const std::string& deviceId);
    std::optional<SpoolJob> getSpoolJob(int jobId) const;
    std::vector<SpoolJob> getSpoolQueue(const std::string& deviceId) const;

    // ----- Statistics (docs/11 section 41) -----
    struct IOStatistics {
        uint64_t totalRequests = 0;
        uint64_t completedRequests = 0;
        uint64_t failedRequests = 0;
        uint64_t ioInterrupts = 0;
        uint64_t bufferFullEvents = 0;
        uint64_t bufferEmptyEvents = 0;
        uint64_t spoolJobsCreated = 0;
        uint64_t spoolJobsCompleted = 0;
    };
    IOStatistics getStatistics() const { return stats_; }
    std::vector<Device> getAllDevices() const;

    // ----- Process Cleanup -----
    void cleanupProcess(int pid);

    // ----- Simulation Tick -----
    void tick(); // Advances all device processing

    // ----- Reset -----
    void reset();

private:
    void record(EventType type, int pid, const std::string& detail);
    int nextRequestId() { return nextRequestId_++; }
    int nextSpoolJobId() { return nextSpoolJobId_++; }
    void scheduleInterrupt(const IORequest& req);

    std::map<std::string, Device> devices_;
    std::map<std::string, Buffer> buffers_;
    std::map<int, SpoolJob> spoolJobs_;
    std::map<int, std::vector<int>> processRequests_; // pid -> request IDs

    EventLog* eventLog_ = nullptr;
    ProcessManager* processes_ = nullptr;
    SimulationClock* clock_ = nullptr;

    int nextRequestId_ = 1;
    int nextSpoolJobId_ = 1;
    IOStatistics stats_{};
};

} // namespace aios