#pragma once

#include <aios/core/types.hpp>

#include <functional>
#include <vector>

namespace aios {

// ---------------------------------------------------------------------------
// Central event log (docs/04 section 27).
// Every subsystem records Events here; a listener (e.g. future WebSocket
// bridge) can be attached for live streaming.
// ---------------------------------------------------------------------------
class EventLog {
public:
    using Listener = std::function<void(const Event&)>;

    void setListener(Listener listener) { listener_ = std::move(listener); }

    void record(const Event& event) {
        events_.push_back(event);
        if (listener_) {
            listener_(event);
        }
    }

    void record(EventType type, int pid, uint64_t cycle, std::string detail) {
        record(Event{type, pid, cycle, std::move(detail)});
    }

    const std::vector<Event>& events() const { return events_; }
    size_t size() const { return events_.size(); }
    void clear() { events_.clear(); }

private:
    std::vector<Event> events_;
    Listener listener_;
};

} // namespace aios