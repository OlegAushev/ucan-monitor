#include "server_watch_service.h"
#include <bsclog/bsclog.h>
#include "csv_logger/csv_logger.hpp"
#include <ucanopen/od/value.h>

namespace ucanopen {

ServerWatchService::ServerWatchService(impl::Server& server,
                                       ServerSdoService& sdo_service,
                                       const ServerHeartbeatService& heartbeat)
        : SdoSubscriber(sdo_service),
          _server(server),
          _sdo_service(sdo_service),
          _heartbeat(heartbeat),
          _logger(std::make_unique<CsvLogger>("logs", 5 * 1024 * 1024, std::chrono::days{7})) {
    _daq_timepoint = std::chrono::steady_clock::now();

    const auto& description = _server.description();
    if (!description) {
        return;
    }
    for (const auto& [key, object] : description->objects()) {
        // create watch object list and data map
        if (object.cls != od::Class::watch) {
            continue;
        }
        auto item = description->resolve(key);
        if (!item.has_value() || !item->available) {
            continue;
        }
        _objects.push_back(*item);
        _object_daq_enabled.push_back(true);
        _data.insert({key, {ExpeditedSdoData{}, "н/д"}});
    }
}

ServerWatchService::~ServerWatchService() = default;

void ServerWatchService::send() {
    // what the last connection lacked, the next one may have
    if (auto const epoch = _heartbeat.epoch(); epoch != _epoch) {
        _epoch = epoch;
        std::lock_guard<std::mutex> lock(_data_mtx);
        for (auto& [key, data] : _data) {
            if (data.state == State::absent) {
                data = {ExpeditedSdoData{}, "н/д"};
            }
        }
    }

    std::shared_lock lock(_objects_mtx);
    if (_enabled && !_objects.empty()) {
        auto now = std::chrono::steady_clock::now();
        if (now - _daq_timepoint >= _period) {
            auto const i = _daq_idx;
            auto const key = _objects[i].key;
            bool absent = false;
            {
                std::lock_guard<std::mutex> data_lock(_data_mtx);
                auto data = _data.find(key);
                absent = (data != _data.end()) && (data->second.state == State::absent);
            }
            if (_object_daq_enabled[i] && !absent) {
                _sdo_service.read(key);
                _daq_timepoint = now;
            }
            _daq_idx = (i + 1) % _objects.size();
        }
    }
}

FrameHandlingStatus ServerWatchService::handle_sdo(const SdoAnswer& answer) {
    if (!answer.item.has_value() || answer.item->cls != od::Class::watch) {
        return FrameHandlingStatus::irrelevant_frame;
    }

    std::lock_guard<std::mutex> lock(_data_mtx);
    auto data = _data.find(answer.key);
    if (data == _data.end()) {
        return FrameHandlingStatus::irrelevant_frame;
    }

    if (answer.type == SdoType::response_to_read) {
        data->second.raw = answer.data;
        data->second.str = od::format_value(*answer.item, answer.data, 2);
        data->second.state = State::value;
        _logger->write(answer.item->path, static_cast<float>(od::to_double(*answer.item, answer.data)));
        return FrameHandlingStatus::success;
    }

    if (answer.type == SdoType::abort) {
        auto const code = static_cast<SdoAbortCode>(answer.data.u32());
        auto const state = (code == SdoAbortCode::no_data_available) ? State::no_data : State::absent;
        if (state != data->second.state) {
            if (state == State::no_data) {
                bsclog::warning("{}: у {} нет данных.", _server.name(), answer.item->path);
            } else {
                bsclog::warning("{}: {} не отвечает (0x{:08X}), опрос его остановлен до нового подключения.",
                                _server.name(),
                                answer.item->path,
                                answer.data.u32());
            }
        }
        data->second.str = "н/д";
        data->second.state = state;
        return FrameHandlingStatus::success;
    }
    return FrameHandlingStatus::irrelevant_frame;
}

} // namespace ucanopen
