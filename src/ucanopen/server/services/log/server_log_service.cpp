#include "server_log_service.h"

#include <ucanopen/od/value.h>


namespace ucanopen {


ServerLogService::ServerLogService(impl::Server& server, SdoPublisher& sdo_publisher)
        : SdoSubscriber(sdo_publisher)
        , _server(server) {
    _start = std::chrono::steady_clock::now();

    const auto& description = _server.description();
    if (!description) {
        return;
    }
    for (const auto& [key, object] : description->objects()) {
        // add watch objects to log
        if (object.cls != od::Class::watch) {
            continue;
        }
        auto item = description->resolve(key);
        if (!item.has_value() || !item->available) {
            continue;
        }
        _log.insert({item->path, LogBuf(_log_capacity)});
        _objects.push_back(std::move(*item));
    }
}


FrameHandlingStatus ServerLogService::handle_sdo(const SdoAnswer& answer) {
    auto now = std::chrono::steady_clock::now();
    if (!answer.item.has_value() || answer.item->cls != od::Class::watch ||
        answer.type != SdoType::response_to_read) {
        return FrameHandlingStatus::irrelevant_frame;
    }

    std::lock_guard<std::mutex> lock(_log_mtx);

    auto log = _log.find(answer.item->path);
    if (log == _log.end()) {
        if (answer.item->kind != od::Item::Kind::element) {
            return FrameHandlingStatus::irrelevant_frame;
        }
        log = _log.emplace(answer.item->path, LogBuf(_log_capacity)).first;
        _objects.push_back(*answer.item);
        ++_version;
    }

    float time = std::chrono::duration_cast<std::chrono::microseconds>(now - _start).count()/1000000.0f;
    float value = static_cast<float>(od::to_double(*answer.item, answer.data));
    log->second.push_back({time, value});

    return FrameHandlingStatus::success;
}


} // namespace ucanopen
