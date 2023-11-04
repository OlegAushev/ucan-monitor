#pragma once


#include "../impl/impl_server.h"
#include <map>
#include <mutex>
#include <shared_mutex>
#include <tuple>
#include <vector>
#include <boost/circular_buffer.hpp>
#include <boost/geometry/geometries/point_xy.hpp>


namespace ucanopen {


class ServerWatchService : public SdoSubscriber, public TpdoSubscriber {
private:
    impl::Server& _server;
    bool _enabled{false};
    std::chrono::milliseconds _period{1000};
    std::chrono::time_point<std::chrono::steady_clock> _acq_timepoint;
    
    // sdo watch objects
    std::vector<const ODObject*> _objects;
    std::vector<bool> _object_acq_enabled;
    mutable std::shared_mutex _objects_mtx;

    using WatchKey = std::pair<std::string_view, std::string_view>;
    using WatchBuf = boost::circular_buffer<boost::geometry::model::d2::point_xy<float>>;

    struct WatchCurrentData {
        ExpeditedSdoData raw;
        std::string str;
    };

    // current data
    std::map<WatchKey, WatchCurrentData> _current_data;
    mutable std::mutex _current_data_mtx;

    // history
    std::chrono::time_point<std::chrono::steady_clock> _history_start;
    std::map<WatchKey, WatchBuf> _history;
    mutable std::mutex _history_mtx;
    static constexpr size_t _history_size = 1000;
public:
    ServerWatchService(impl::Server& server, impl::SdoPublisher& sdo_publisher, impl::TpdoPublisher& tpdo_publisher);
    void send();
    virtual FrameHandlingStatus handle_sdo(ODEntryIter entry, SdoType sdo_type, ExpeditedSdoData sdo_data) override;
    virtual FrameHandlingStatus handle_tpdo(CobTpdo tpdo, const can_payload& payload) override;

    void enable() { _enabled = true; }
    void disable() { _enabled = false; }
    void set_period(std::chrono::milliseconds period) { _period = period; }
    
    std::vector<const ODObject*> objects() const {
        std::shared_lock lock(_objects_mtx);
        return _objects;
    }

    bool acq_enabled(size_t idx) {
        std::shared_lock lock(_objects_mtx);
        if (idx >= _object_acq_enabled.size()) { return false; }
        return _object_acq_enabled[idx];
    }

    void enable_acq(size_t idx, bool value) {
        std::unique_lock lock(_objects_mtx);
        if (idx >= _object_acq_enabled.size()) { return;}
        _object_acq_enabled[idx] = value;
    }

    std::string string_value(std::string_view watch_subcategory, std::string_view watch_name) const {
        std::lock_guard<std::mutex> lock(_current_data_mtx);
        auto it = _current_data.find({watch_subcategory, watch_name});
        if (it == _current_data.end()) {
            return "n/a";
        }
        return it->second.str;
    }

    ExpeditedSdoData value(std::string_view watch_subcategory, std::string_view watch_name) {
        std::lock_guard<std::mutex> lock(_current_data_mtx);
        auto it = _current_data.find({watch_subcategory, watch_name});
        if (it == _current_data.end()) {
            return 0;
        }
        return it->second.raw;
    }

    boost::circular_buffer<boost::geometry::model::d2::point_xy<float>>* history(std::string_view watch_subcategory, std::string_view watch_name) {
        std::lock_guard<std::mutex> lock(_history_mtx);
        auto iter = _history.find({watch_subcategory, watch_name});
        if (iter == _history.end()) {
            return nullptr;
        }
        return &iter->second;
    }

    auto history_start() const { return _history_start; }
    void set_history_size(size_t size) {
        std::lock_guard<std::mutex> lock(_history_mtx);
        for (auto& item : _history) {
            item.second.set_capacity(size);
            item.second.clear();
        }
    }

    std::mutex& history_mtx() { return _history_mtx; }

private:
    // tpdo watch objects
    std::vector<const ODObject*> _tpdo_objects;
    struct TpdoMapping {
        std::string subcategory;
        std::string name;
        uint64_t offset;
        uint64_t mask;
    };
    std::map<CobTpdo, std::vector<TpdoMapping>> _tpdo_mapping;
    std::vector<std::tuple<std::string_view, std::string_view, float>> _unmap_tpdo(CobTpdo tpdo, const can_payload& payload) const;
public:
    std::vector<const ODObject*> tpdo_objects() const {
        return _tpdo_objects;
    }
};


} // namespace ucanopen
