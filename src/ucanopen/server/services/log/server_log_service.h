#pragma once


#include <ucanopen/od/description.h>
#include <ucanopen/server/impl/impl_server.h>
#include <ucanopen/server/services/sdo/server_sdo_service.h>
#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <string_view>
#include <vector>
#include <boost/circular_buffer.hpp>
#include <boost/geometry/geometries/point_xy.hpp>


namespace ucanopen {


// The history of the watched objects for the plots, each by its path: the
// values that the watch reads, as numbers whatever their type.
class ServerLogService : public SdoSubscriber {
public:
    using LogKey = std::string;
    using LogBuf = boost::circular_buffer<boost::geometry::model::d2::point_xy<float>>;
private:
    impl::Server& _server;
    std::vector<od::Item> _objects;

    std::chrono::time_point<std::chrono::steady_clock> _start;
    std::map<LogKey, LogBuf, std::less<>> _log;
    mutable std::mutex _log_mtx;
    static inline size_t _log_capacity = 1000000;
public:
    ServerLogService(impl::Server& server, SdoPublisher& sdo_publisher);
    std::vector<od::Item> objects() const { return _objects; }
    virtual FrameHandlingStatus handle_sdo(const SdoAnswer& answer) override;

    const LogBuf* get_log(std::string_view path) const {
        std::lock_guard<std::mutex> lock(_log_mtx);
        auto log = _log.find(path);
        if (log == _log.end()) {
            return nullptr;
        }
        return &log->second;
    }

    std::map<LogKey, LogBuf, std::less<>> get_log() const {
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
};


} // namespace ucanopen
