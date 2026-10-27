#include <aios/io/IOManager.hpp>

#include <aios/core/SimulationClock.hpp>
#include <aios/interrupt/InterruptManager.hpp>
#include <aios/process/ProcessManager.hpp>

#include <algorithm>
#include <sstream>

namespace aios {

// ---------------------------------------------------------------------------
// DeviceType/DeviceState/IOOperation/IORequestState/BufferState toString
// ---------------------------------------------------------------------------
std::string deviceTypeToString(DeviceType type) {
    switch (type) {
        case DeviceType::DISK: return "DISK";
        case DeviceType::PRINTER: return "PRINTER";
        case DeviceType::KEYBOARD: return "KEYBOARD";
        case DeviceType::NETWORK: return "NETWORK";
    }
    return "UNKNOWN";
}

std::string deviceStateToString(DeviceState state) {
    switch (state) {
        case DeviceState::AVAILABLE: return "AVAILABLE";
        case DeviceState::BUSY: return "BUSY";
        case DeviceState::ERROR: return "ERROR";
    }
    return "UNKNOWN";
}

std::string ioOperationToString(IOOperation op) {
    switch (op) {
        case IOOperation::READ: return "READ";
        case IOOperation::WRITE: return "WRITE";
    }
    return "UNKNOWN";
}

std::string ioRequestStateToString(IORequestState state) {
    switch (state) {
        case IORequestState::CREATED: return "CREATED";
        case IORequestState::QUEUED: return "QUEUED";
        case IORequestState::PROCESSING: return "PROCESSING";
        case IORequestState::COMPLETED: return "COMPLETED";
        case IORequestState::ERROR: return "ERROR";
    }
    return "UNKNOWN";
}

std::string bufferStateToString(BufferState state) {
    switch (state) {
        case BufferState::EMPTY: return "EMPTY";
        case BufferState::PARTIAL: return "PARTIAL";
        case BufferState::FULL: return "FULL";
    }
    return "UNKNOWN";
}

std::string IORequest::toString() const {
    std::ostringstream oss;
    oss << "IORequest[id=" << id << " pid=" << pid << " device=" << deviceId
        << " op=" << ioOperationToString(operation) << " state=" << ioRequestStateToString(state)
        << " requestedAt=" << requestedAt << "]";
    return oss.str();
}

std::string SpoolJob::toString() const {
    std::ostringstream oss;
    oss << "SpoolJob[id=" << id << " pid=" << ownerPid << " device=" << deviceId
        << " op=" << ioOperationToString(operation) << " status=" << status
        << " created=" << createdAt << "]";
    return oss.str();
}

// ---------------------------------------------------------------------------
// Buffer Implementation
// ---------------------------------------------------------------------------
Buffer::Buffer(std::string id, size_t capacity)
    : id_(std::move(id)), capacity_(capacity), state_(BufferState::EMPTY) {}

void Buffer::updateState() {
    if (data_.empty()) {
        state_ = BufferState::EMPTY;
    } else if (data_.size() >= capacity_) {
        state_ = BufferState::FULL;
    } else {
        state_ = BufferState::PARTIAL;
    }
}

bool Buffer::write(const std::string& input) {
    if (data_.size() + input.size() > capacity_) {
        return false;
    }
    data_ += input;
    updateState();
    return true;
}

std::string Buffer::read(size_t count) {
    if (data_.empty()) {
        return "";
    }
    size_t actualCount = std::min(count, data_.size());
    std::string result = data_.substr(0, actualCount);
    data_.erase(0, actualCount);
    updateState();
    return result;
}

bool Buffer::hasSpace(size_t count) const {
    return data_.size() + count <= capacity_;
}

bool Buffer::hasData() const {
    return !data_.empty();
}

void Buffer::clear() {
    data_.clear();
    updateState();
}

std::string Buffer::toString() const {
    std::ostringstream oss;
    oss << "Buffer[id=" << id_ << " capacity=" << capacity_ << " size=" << data_.size()
        << " state=" << bufferStateToString(state_) << "]";
    return oss.str();
}

// ---------------------------------------------------------------------------
// Device Implementation
// ---------------------------------------------------------------------------
Device::Device(std::string id, DeviceType type, int processingTimeCycles)
    : id_(std::move(id)), type_(type), processingTimeCycles_(processingTimeCycles) {}

bool Device::canAcceptRequest() const {
    return state_ == DeviceState::AVAILABLE && !currentRequest_.has_value();
}

void Device::enqueueRequest(const IORequest& req) {
    queue_.push_back(req);
}

std::optional<IORequest> Device::dequeueRequest() {
    if (queue_.empty()) {
        return std::nullopt;
    }
    IORequest req = queue_.front();
    queue_.pop_front();
    return req;
}

bool Device::startProcessing() {
    if (!canAcceptRequest()) {
        return false;
    }
    auto req = dequeueRequest();
    if (!req.has_value()) {
        return false;
    }
    currentRequest_ = req;
    currentProcessingCycle_ = 0;
    setState(DeviceState::BUSY);
    return true;
}

bool Device::tick() {
    if (!currentRequest_.has_value()) {
        return false;
    }
    currentProcessingCycle_++;
    if (currentProcessingCycle_ >= processingTimeCycles_) {
        return true; // Processing complete
    }
    return false;
}

std::optional<IORequest> Device::checkCompletion() {
    if (!currentRequest_.has_value()) {
        return std::nullopt;
    }
    if (currentProcessingCycle_ >= processingTimeCycles_) {
        IORequest completed = currentRequest_.value();
        completed.state = IORequestState::COMPLETED;
        completed.completionCycle = currentProcessingCycle_; // Using this field
        currentRequest_.reset();
        currentProcessingCycle_ = 0;
        completedRequests_++;
        totalProcessingCycles_ += processingTimeCycles_;
        setState(DeviceState::AVAILABLE);
        return completed;
    }
    return std::nullopt;
}

void Device::setError(const std::string& reason) {
    setState(DeviceState::ERROR);
    failedRequests_++;
    // Current request fails
    if (currentRequest_.has_value()) {
        currentRequest_.reset();
        currentProcessingCycle_ = 0;
    }
}

void Device::recover() {
    setState(DeviceState::AVAILABLE);
}

void Device::setState(DeviceState newState) {
    state_ = newState;
}

std::string Device::toString() const {
    std::ostringstream oss;
    oss << "Device[id=" << id_ << " type=" << deviceTypeToString(type_)
        << " state=" << deviceStateToString(state_) << " queue=" << queue_.size();
    if (currentRequest_.has_value()) {
        oss << " current=" << currentRequest_->id;
    }
    oss << "]";
    return oss.str();
}

// ---------------------------------------------------------------------------
// IOManager Implementation
// ---------------------------------------------------------------------------

bool IOManager::createDevice(const std::string& id, DeviceType type, int processingTimeCycles) {
    if (devices_.find(id) != devices_.end()) {
        return false;
    }
    devices_.emplace(id, Device(id, type, processingTimeCycles));
    record(EventType::IO_REQUEST, INVALID_PID, "Device created: " + id + " type=" + deviceTypeToString(type));
    return true;
}

std::optional<Device> IOManager::getDevice(const std::string& id) const {
    auto it = devices_.find(id);
    if (it != devices_.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::vector<std::string> IOManager::listDevices() const {
    std::vector<std::string> ids;
    for (const auto& [id, dev] : devices_) {
        ids.push_back(id);
    }
    return ids;
}

int IOManager::requestIO(int pid, const std::string& deviceId, IOOperation op, const std::string& data) {
    auto it = devices_.find(deviceId);
    if (it == devices_.end()) {
        record(EventType::IO_ERROR, pid, "Device not found: " + deviceId);
        return INVALID_PID;
    }

    Device& device = it->second;
    if (device.state() == DeviceState::ERROR) {
        record(EventType::IO_ERROR, pid, "Device in error state: " + deviceId);
        return INVALID_PID;
    }

    int reqId = nextRequestId();
    IORequest req;
    req.id = reqId;
    req.pid = pid;
    req.deviceId = 0; // We'll map device ID string to index if needed
    req.operation = op;
    req.data = data;
    req.requestedAt = clock_ ? clock_->cycle() : 0;
    req.state = IORequestState::CREATED;

    // Check if we should use buffer for this operation
    // For now, directly queue to device
    if (device.canAcceptRequest()) {
        // Start immediately
        IORequest reqCopy = req;
        reqCopy.state = IORequestState::PROCESSING;
        device.enqueueRequest(reqCopy);
        device.startProcessing();
        record(EventType::IO_REQUEST, pid, "I/O started on " + deviceId + " op=" + ioOperationToString(op));
    } else {
        req.state = IORequestState::QUEUED;
        device.enqueueRequest(req);
        record(EventType::IO_REQUEST, pid, "I/O queued on " + deviceId + " op=" + ioOperationToString(op));
    }

    // Track request for process
    processRequests_[pid].push_back(reqId);
    stats_.totalRequests++;
    record(EventType::IO_REQUEST, pid, "I/O request " + std::to_string(reqId) + " on " + deviceId);

    return reqId;
}

std::optional<IORequest> IOManager::checkCompletion(int pid) {
    auto it = processRequests_.find(pid);
    if (it == processRequests_.end()) {
        return std::nullopt;
    }

    for (int reqId : it->second) {
        // Check all devices for this request
        for (auto& [devId, device] : devices_) {
            auto completed = device.checkCompletion();
            if (completed.has_value() && completed->id == it->second.front()) {
                IORequest req = completed.value();
                req.state = IORequestState::COMPLETED;
                req.completionCycle = clock_ ? clock_->cycle() : 0;
                stats_.completedRequests++;
                record(EventType::IO_COMPLETE, pid, "I/O request " + std::to_string(req.id) + " completed");
                return req;
            }
        }
    }
    return std::nullopt;
}

bool IOManager::hasPendingRequests(const std::string& deviceId) const {
    auto it = devices_.find(deviceId);
    if (it == devices_.end()) return false;
    return !it->second.queue().empty() || it->second.currentRequest().has_value();
}

std::vector<IORequest> IOManager::getDeviceQueue(const std::string& deviceId) const {
    auto it = devices_.find(deviceId);
    if (it == devices_.end()) return {};
    std::vector<IORequest> result;
    if (it->second.currentRequest().has_value()) {
        result.push_back(it->second.currentRequest().value());
    }
    for (const auto& req : it->second.queue()) {
        result.push_back(req);
    }
    return result;
}

size_t IOManager::queueSize(const std::string& deviceId) const {
    auto it = devices_.find(deviceId);
    if (it == devices_.end()) return 0;
    size_t count = it->second.queue().size();
    if (it->second.currentRequest().has_value()) count++;
    return count;
}

// Buffer Management
bool IOManager::createBuffer(const std::string& id, size_t capacity) {
    if (buffers_.find(id) != buffers_.end()) {
        return false;
    }
    buffers_.emplace(id, Buffer(id, capacity));
    record(EventType::IO_REQUEST, INVALID_PID, "Buffer created: " + id + " capacity=" + std::to_string(capacity));
    return true;
}

std::optional<Buffer> IOManager::getBuffer(const std::string& id) const {
    auto it = buffers_.find(id);
    if (it != buffers_.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::vector<std::string> IOManager::listBuffers() const {
    std::vector<std::string> ids;
    for (const auto& [id, buf] : buffers_) {
        ids.push_back(id);
    }
    return ids;
}

bool IOManager::writeBuffer(const std::string& id, const std::string& data) {
    auto it = buffers_.find(id);
    if (it == buffers_.end()) {
        return false;
    }
    bool success = it->second.write(data);
    if (!success) {
        stats_.bufferFullEvents++;
        record(EventType::IO_ERROR, INVALID_PID, "Buffer full: " + id);
    }
    return success;
}

std::string IOManager::readBuffer(const std::string& id, size_t count) {
    auto it = buffers_.find(id);
    if (it == buffers_.end()) {
        stats_.bufferEmptyEvents++;
        record(EventType::IO_ERROR, INVALID_PID, "Buffer not found: " + id);
        return "";
    }
    std::string data = it->second.read(count);
    if (data.empty() && it->second.state() == BufferState::EMPTY) {
        stats_.bufferEmptyEvents++;
        record(EventType::IO_ERROR, INVALID_PID, "Buffer empty: " + id);
    }
    return data;
}

bool IOManager::hasBufferSpace(const std::string& id, size_t count) const {
    auto it = buffers_.find(id);
    if (it == buffers_.end()) return false;
    return it->second.hasSpace(count);
}

bool IOManager::hasBufferData(const std::string& id) const {
    auto it = buffers_.find(id);
    if (it == buffers_.end()) return false;
    return it->second.hasData();
}

// Spooling
int IOManager::createSpoolJob(int pid, const std::string& deviceId, IOOperation op, const std::string& data) {
    auto it = devices_.find(deviceId);
    if (it == devices_.end()) {
        return INVALID_PID;
    }

    int jobId = nextSpoolJobId();
    SpoolJob job;
    job.id = jobId;
    job.ownerPid = pid;
    job.deviceId = deviceId;
    job.operation = op;
    job.data = data;
    job.status = "QUEUED";
    job.createdAt = clock_ ? clock_->cycle() : 0;

    spoolJobs_[jobId] = job;
    stats_.spoolJobsCreated++;
    record(EventType::IO_REQUEST, pid, "Spool job " + std::to_string(jobId) + " created for " + deviceId);

    // Try to process immediately
    processSpoolQueue(deviceId);

    return jobId;
}

bool IOManager::processSpoolQueue(const std::string& deviceId) {
    auto it = devices_.find(deviceId);
    if (it == devices_.end()) return false;

    Device& device = it->second;

    // Find first queued spool job for this device
    for (auto& [jobId, job] : spoolJobs_) {
        if (job.deviceId == deviceId && job.status == "QUEUED" && device.canAcceptRequest()) {
            job.status = "PROCESSING";
            IORequest req;
            req.id = nextRequestId();
            req.pid = job.ownerPid;
            req.deviceId = 0;
            req.operation = job.operation;
            req.data = job.data;
            req.state = IORequestState::PROCESSING;
            req.requestedAt = clock_ ? clock_->cycle() : 0;

            device.enqueueRequest(req);
            device.startProcessing();

            job.status = "PROCESSING";
            record(EventType::IO_REQUEST, job.ownerPid, "Spool job " + std::to_string(job.id) + " started on " + deviceId);
            return true;
        }
    }
    return false;
}

std::optional<SpoolJob> IOManager::getSpoolJob(int jobId) const {
    auto it = spoolJobs_.find(jobId);
    if (it != spoolJobs_.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::vector<SpoolJob> IOManager::getSpoolQueue(const std::string& deviceId) const {
    std::vector<SpoolJob> result;
    for (const auto& [id, job] : spoolJobs_) {
        if (job.deviceId == deviceId) {
            result.push_back(job);
        }
    }
    return result;
}

// Process cleanup
void IOManager::cleanupProcess(int pid) {
    // Remove process requests
    processRequests_.erase(pid);

    // Remove spool jobs owned by this process
    for (auto it = spoolJobs_.begin(); it != spoolJobs_.end();) {
        if (it->second.ownerPid == pid) {
            it = spoolJobs_.erase(it);
        } else {
            ++it;
        }
    }

    // Remove from device queues
    for (auto& [devId, device] : devices_) {
        device.queue().erase(
            std::remove_if(device.queue().begin(), device.queue().end(),
                [pid](const IORequest& req) { return req.pid == pid; }),
            device.queue().end()
        );
    }
}

// Tick - advance all device processing
void IOManager::tick() {
    for (auto& [devId, device] : devices_) {
        if (device.tick()) {
            // Processing complete
            auto completed = device.checkCompletion();
            if (completed.has_value()) {
                IORequest req = completed.value();
                req.state = IORequestState::COMPLETED;
                req.completionCycle = clock_ ? clock_->cycle() : 0;
                stats_.completedRequests++;
                stats_.ioInterrupts++;
                record(EventType::IO_COMPLETE, req.pid, "I/O request " + std::to_string(req.id) + " completed on " + devId);
                // Wake the process via interrupt
                if (processes_) {
                    processes_->wakeProcess(req.pid);
                }
            }
        }
    }

    // Process spool queues
    for (const auto& [devId, device] : devices_) {
        if (device.canAcceptRequest()) {
            processSpoolQueue(devId);
        }
    }
}

void IOManager::reset() {
    devices_.clear();
    buffers_.clear();
    spoolJobs_.clear();
    processRequests_.clear();
    nextRequestId_ = 1;
    nextSpoolJobId_ = 1;
    stats_ = IOStatistics{};
}

void IOManager::record(EventType type, int pid, const std::string& detail) {
    if (eventLog_ && clock_) {
        eventLog_->record(type, pid, clock_->cycle(), detail);
    }
}

} // namespace aios