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


class ServerLogService : public SdoSubscriber, public TpdoSubscriber {
private:
    impl::Server& _server;
    std::vector<const ODObject*> _objects;

    using LogKey = std::pair<std::string_view, std::string_view>;
    using LogBuf = boost::circular_buffer<boost::geometry::model::d2::point_xy<float>>;

    std::chrono::time_point<std::chrono::steady_clock> _start;
    std::map<LogKey, LogBuf> _log;
    mutable std::mutex _log_mtx;
    static constexpr size_t _log_size = 1000;
public:
    ServerLogService(impl::Server& server, impl::SdoPublisher& sdo_publisher, impl::TpdoPublisher& tpdo_publisher);
    std::vector<const ODObject*> objects() const { return _objects; }
    virtual FrameHandlingStatus handle_sdo(ODEntryIter entry, SdoType sdo_type, ExpeditedSdoData sdo_data) override;
    virtual FrameHandlingStatus handle_tpdo(CobTpdo tpdo, const can_payload& payload) override;

    boost::circular_buffer<boost::geometry::model::d2::point_xy<float>>* get_log(std::string_view watch_subcategory, std::string_view watch_name) {
        std::lock_guard<std::mutex> lock(_log_mtx);
        auto iter = _log.find({watch_subcategory, watch_name});
        if (iter == _log.end()) {
            return nullptr;
        }
        return &iter->second;
    }

    auto log_start() const { return _start; }
    void set_log_size(size_t size) {
        std::lock_guard<std::mutex> lock(_log_mtx);
        for (auto& item : _log) {
            item.second.set_capacity(size);
            item.second.clear();
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
