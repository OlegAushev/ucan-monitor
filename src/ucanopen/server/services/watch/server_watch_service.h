#pragma once

#include <map>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <ucanopen/od/description.h>
#include <ucanopen/server/impl/impl_server.h>
#include <ucanopen/server/services/heartbeat/server_heartbeat_service.h>
#include <ucanopen/server/services/sdo/server_sdo_service.h>
#include <vector>

class CsvLogger;

namespace ucanopen {

// The objects of the class watch, polled one a period in turn. An object that
// answers that it has no data, 08000024h, shows «н/д» and keeps being polled;
// one the device does not have, 06020000h or 06090011h, is no longer asked
// for until the next connection.
class ServerWatchService : public SdoSubscriber {
private:
    impl::Server& _server;
    ServerSdoService& _sdo_service;
    const ServerHeartbeatService& _heartbeat;
    bool _enabled{true};
    std::chrono::milliseconds _period{1000};
    std::chrono::time_point<std::chrono::steady_clock> _daq_timepoint;
    size_t _daq_idx{0}; // the object polled next, one cursor per server
    unsigned _epoch{0};

    std::vector<od::Item> _objects;
    std::vector<bool> _object_daq_enabled;
    mutable std::shared_mutex _objects_mtx;

    enum class State { unread, value, no_data, absent };

    struct WatchCurrentData {
        ExpeditedSdoData raw;
        std::string str;
        State state{State::unread};
    };

    std::map<od::Key, WatchCurrentData> _data;
    mutable std::mutex _data_mtx;

    std::unique_ptr<CsvLogger> _logger;
public:
    ServerWatchService(impl::Server& server,
                       ServerSdoService& sdo_service,
                       const ServerHeartbeatService& heartbeat);
    ~ServerWatchService();
    void send();
    virtual FrameHandlingStatus handle_sdo(const SdoAnswer& answer) override;

    bool enabled() const { return _enabled; }
    void enable() { _enabled = true; }
    void disable() { _enabled = false; }
    std::chrono::milliseconds period() const { return _period; }
    void set_period(std::chrono::milliseconds period) { _period = period; }

    std::vector<od::Item> objects() const {
        std::shared_lock lock(_objects_mtx);
        return _objects;
    }

    bool daq_enabled(size_t idx) {
        std::shared_lock lock(_objects_mtx);
        if (idx >= _object_daq_enabled.size()) {
            return false;
        }
        return _object_daq_enabled[idx];
    }

    void toggle_daq(size_t idx, bool value) {
        std::unique_lock lock(_objects_mtx);
        if (idx >= _object_daq_enabled.size()) {
            return;
        }
        _object_daq_enabled[idx] = value;
    }

    std::string string_value(od::Key key) const {
        std::lock_guard<std::mutex> lock(_data_mtx);
        auto data = _data.find(key);
        if (data == _data.end()) {
            return "н/д";
        }
        return data->second.str;
    }

    std::string string_value(std::string_view path) const {
        const auto& description = _server.description();
        auto key = description ? description->find(path) : std::nullopt;
        return key.has_value() ? string_value(*key) : "н/д";
    }

    ExpeditedSdoData value(od::Key key) const {
        std::lock_guard<std::mutex> lock(_data_mtx);
        auto data = _data.find(key);
        if (data == _data.end()) {
            return 0;
        }
        return data->second.raw;
    }
};

} // namespace ucanopen
