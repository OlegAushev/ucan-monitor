#pragma once


#include <ucanopen/server/impl/impl_server.h>
#include <ucanopen/server/services/sdo/server_sdo_service.h>
#include <map>
#include <mutex>
#include <shared_mutex>
#include <tuple>
#include <vector>
#include <boost/circular_buffer.hpp>
#include <boost/geometry/geometries/point_xy.hpp>


namespace ucanopen {


class ServerWatchService : public SdoSubscriber {
private:
    impl::Server& _server;
    ServerSdoService& _sdo_service;
    bool _enabled{false};
    std::chrono::milliseconds _period{1000};
    std::chrono::time_point<std::chrono::steady_clock> _daq_timepoint;
    
    // sdo watch objects
    std::vector<const ODObject*> _objects;
    std::vector<bool> _object_daq_enabled;
    mutable std::shared_mutex _objects_mtx;

    using WatchKey = std::pair<std::string_view, std::string_view>;
    using WatchBuf = boost::circular_buffer<boost::geometry::model::d2::point_xy<float>>;

    struct WatchCurrentData {
        ExpeditedSdoData raw;
        std::string str;
    };

    // current data
    std::map<WatchKey, WatchCurrentData> _data;
    mutable std::mutex _data_mtx;

public:
    ServerWatchService(impl::Server& server, ServerSdoService& sdo_service);
    void send();
    virtual FrameHandlingStatus handle_sdo(ODEntryIter entry, SdoType sdo_type, ExpeditedSdoData sdo_data) override;

    void enable() { _enabled = true; }
    void disable() { _enabled = false; }
    void set_period(std::chrono::milliseconds period) { _period = period; }
    
    std::vector<const ODObject*> objects() const {
        std::shared_lock lock(_objects_mtx);
        return _objects;
    }

    bool daq_enabled(size_t idx) {
        std::shared_lock lock(_objects_mtx);
        if (idx >= _object_daq_enabled.size()) { return false; }
        return _object_daq_enabled[idx];
    }

    void toggle_daq(size_t idx, bool value) {
        std::unique_lock lock(_objects_mtx);
        if (idx >= _object_daq_enabled.size()) { return;}
        _object_daq_enabled[idx] = value;
    }

    std::string string_value(std::string_view watch_subcategory, std::string_view watch_name) const {
        std::lock_guard<std::mutex> lock(_data_mtx);
        auto data = _data.find({watch_subcategory, watch_name});
        if (data == _data.end()) {
            return "n/a";
        }
        return data->second.str;
    }

    ExpeditedSdoData value(std::string_view watch_subcategory, std::string_view watch_name) {
        std::lock_guard<std::mutex> lock(_data_mtx);
        auto data = _data.find({watch_subcategory, watch_name});
        if (data == _data.end()) {
            return 0;
        }
        return data->second.raw;
    }
};


} // namespace ucanopen
