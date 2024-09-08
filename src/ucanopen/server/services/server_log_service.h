#pragma once


#include "../impl/impl_server.h"
#include "server_sdo_service.h"
#include <map>
#include <mutex>
#include <shared_mutex>
#include <tuple>
#include <vector>
#include <boost/circular_buffer.hpp>
#include <boost/geometry/geometries/point_xy.hpp>


namespace ucanopen {


class ServerLogService : public SdoSubscriber,
                         public TpdoSubscriber {
public:
    using LogKey = std::pair<std::string_view, std::string_view>;
    using LogBuf = boost::circular_buffer<boost::geometry::model::d2::point_xy<float>>;
private:
    impl::Server& _server;
    std::vector<const ODObject*> _objects;


    std::chrono::time_point<std::chrono::steady_clock> _start;
    std::map<LogKey, LogBuf> _log;
    mutable std::mutex _log_mtx;
    static inline size_t _log_capacity = 1000;
public:
    ServerLogService(impl::Server& server, SdoPublisher& sdo_publisher, impl::TpdoPublisher& tpdo_publisher);
    std::vector<const ODObject*> objects() const { return _objects; }
    virtual FrameHandlingStatus handle_sdo(ODEntryIter entry, SdoType sdo_type, ExpeditedSdoData sdo_data) override;
    virtual FrameHandlingStatus handle_tpdo(CobTpdo tpdo, const can_payload& payload) override;

    const boost::circular_buffer<boost::geometry::model::d2::point_xy<float>>* get_log(std::string_view watch_subcategory, std::string_view watch_name) const {
        std::lock_guard<std::mutex> lock(_log_mtx);
        auto log = _log.find({watch_subcategory, watch_name});
        if (log == _log.end()) {
            return nullptr;
        }
        return &log->second;
    }

    std::map<LogKey, LogBuf> get_log() const {
        return _log;
    }

    auto log_start() const { return _start; }
    size_t log_capacity() const { return _log_capacity; }
    void set_log_capacity(size_t capacity) {
        std::lock_guard<std::mutex> lock(_log_mtx);
        _log_capacity = capacity;
        for (auto& log : _log) {
            if (capacity < log.second.capacity()) {
                log.second.clear();
            }
            log.second.set_capacity(capacity);
        }
    }

    std::mutex& log_mtx() { return _log_mtx; }
private:
    // tpdo watch objects
    struct TpdoMapping {
        std::string subcategory;
        std::string name;
        uint64_t offset;
        uint64_t mask;
    };
    std::map<CobTpdo, std::vector<TpdoMapping>> _tpdo_mapping;
    std::vector<std::tuple<std::string_view, std::string_view, float>> _unmap_tpdo(CobTpdo tpdo, const can_payload& payload) const;
};


} // namespace ucanopen
