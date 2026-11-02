#include "server_watch_service.h"
#include <bsclog/bsclog.h>
#include "csv_logger/csv_logger.hpp"
#include <ucanopen/od/value.h>

#include <algorithm>

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
    for (const auto& [index, array] : description->arrays()) {
        if (array.cls == od::Class::watch && !array.text) {
            _arrays.push_back({index, 0x00});
        }
    }
    _pending_counts = _arrays;
}

ServerWatchService::~ServerWatchService() = default;

void ServerWatchService::send() {
    if (auto const epoch = _heartbeat.epoch(); epoch != _epoch) {
        _epoch = epoch;
        _new_connection();
    }

    if (!_enabled) {
        return;
    }
    auto now = std::chrono::steady_clock::now();
    if (now - _daq_timepoint < _period) {
        return;
    }

    // the counts first: they say which elements there are to poll
    if (!_pending_counts.empty()) {
        _count_idx %= _pending_counts.size();
        _sdo_service.read(_pending_counts[_count_idx++], true);
        _daq_timepoint = now;
        return;
    }

    std::shared_lock lock(_objects_mtx);
    if (_objects.empty()) {
        return;
    }
    _daq_idx %= _objects.size();
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

FrameHandlingStatus ServerWatchService::handle_sdo(const SdoAnswer& answer) {
    if (!answer.item.has_value() || answer.item->cls != od::Class::watch) {
        return FrameHandlingStatus::irrelevant_frame;
    }

    if (answer.item->kind == od::Item::Kind::count) {
        auto pending = std::find(_pending_counts.begin(), _pending_counts.end(), answer.key);
        if (pending == _pending_counts.end()) {
            return FrameHandlingStatus::irrelevant_frame;
        }
        _pending_counts.erase(pending);
        if (answer.type == SdoType::response_to_read) {
            _list_elements(*answer.item, answer.data.u8());
        }
        return FrameHandlingStatus::success;
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

// What the last connection lacked, the next one may have, and its arrays may
// be of other lengths: another board answers at the same node-ID.
void ServerWatchService::_new_connection() {
    std::unique_lock lock(_objects_mtx);
    std::lock_guard<std::mutex> data_lock(_data_mtx);

    std::vector<od::Item> objects;
    std::vector<bool> enabled;
    for (size_t i = 0; i < _objects.size(); ++i) {
        if (_objects[i].kind == od::Item::Kind::element) {
            _data.erase(_objects[i].key);
            continue;
        }
        objects.push_back(std::move(_objects[i]));
        enabled.push_back(_object_daq_enabled[i]);
    }
    _objects = std::move(objects);
    _object_daq_enabled = std::move(enabled);
    for (auto& [key, data] : _data) {
        if (data.state == State::absent) {
            data = {ExpeditedSdoData{}, "н/д"};
        }
    }
    _pending_counts = _arrays;
    ++_version;
}

void ServerWatchService::_list_elements(const od::Item& count, unsigned elements) {
    const auto& description = _server.description();
    std::unique_lock lock(_objects_mtx);
    std::lock_guard<std::mutex> data_lock(_data_mtx);
    for (unsigned i = 1; i <= elements; ++i) {
        auto item = description->resolve({count.key.index, static_cast<uint8_t>(i)});
        if (!item.has_value() || !item->available) {
            continue;
        }
        // in the order of keys, as the description lists the objects
        auto const at = std::find_if(_objects.begin(), _objects.end(), [&item](const od::Item& object) {
            return item->key < object.key;
        });
        auto const offset = at - _objects.begin();
        _object_daq_enabled.insert(_object_daq_enabled.begin() + offset, true);
        _data.insert({item->key, {ExpeditedSdoData{}, "н/д"}});
        _objects.insert(at, std::move(*item));
    }
    ++_version;
}

} // namespace ucanopen
