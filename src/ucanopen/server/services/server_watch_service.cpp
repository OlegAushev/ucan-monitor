#include "server_watch_service.h"
#include <bsclog/bsclog.h>


namespace ucanopen {


ServerWatchService::ServerWatchService(impl::Server& server, impl::SdoPublisher& sdo_publisher, impl::TpdoPublisher& tpdo_publisher)
        : SdoSubscriber(sdo_publisher)
        , TpdoSubscriber(tpdo_publisher)
        , _server(server) {
    _acq_timepoint = std::chrono::steady_clock::now();
    _history_start = std::chrono::steady_clock::now();

    for (const auto& [key, object] : _server.dictionary().entries) {
        // create watch entries list and data map
        if (object.category == _server.dictionary().config.watch_category) {
            _objects.push_back(&object);
            _object_acq_enabled.push_back(true);
            _current_data.insert({WatchKey{object.subcategory, object.name}, {ExpeditedSdoData{}, "..."}});
            _history.insert({WatchKey{object.subcategory, object.name}, WatchBuf(_history_size)});
        }
    }

    // create tpdo mapping
    for (int i = 0; i < 4; ++i) {
        uint16_t index = 0x1A00 + i;
        if (!_server.dictionary().entries.contains({index, 0x00})) {
            continue;
        }

        uint16_t subindex = 0x01;
        uint64_t offset = 0;
        while (_server.dictionary().entries.contains({index, subindex})) {
            const auto& object = _server.dictionary().entries.at({index, subindex});
            _tpdo_objects.push_back(&object);
            _current_data.insert({WatchKey{object.subcategory, object.name}, {ExpeditedSdoData{}, "..."}});
            _history.insert({WatchKey{object.subcategory, object.name}, WatchBuf(_history_size)});

            uint64_t mask = 0;
            for (size_t i = 0; i < od_object_data_type_sizes[object.data_type]; ++i) {
                mask |= 0xFF << i * 8;
            }

            _tpdo_mapping[CobTpdo(i)].push_back({object.subcategory, object.name, offset, mask});
            offset += od_object_data_type_sizes[object.data_type] * 8;

            ++subindex;
        }
    }
}


void ServerWatchService::send() {
    std::shared_lock lock(_objects_mtx);
    if (_enabled && !_objects.empty()) {
        auto now = std::chrono::steady_clock::now();
        if (now - _acq_timepoint >= _period) {
            static size_t i = 0;
            if (_object_acq_enabled[i]) {
                _server.read(_server.dictionary().config.watch_category,
                            _objects[i]->subcategory,
                            _objects[i]->name);
                _acq_timepoint = now;
            }
            i = (i + 1) % _objects.size();
        }
    }
}


FrameHandlingStatus ServerWatchService::handle_sdo(ODEntryIter entry, SdoType sdo_type, ExpeditedSdoData sdo_data) {
    std::lock_guard<std::mutex> lock(_current_data_mtx);
    auto now = std::chrono::steady_clock::now();
    const auto& [key, object] = *entry;

    if ((object.category == _server.dictionary().config.watch_category) && (sdo_type == SdoType::response_to_read)) {
        WatchKey watch_key{object.subcategory, object.name};
        _current_data[watch_key].raw = sdo_data;
        _current_data[watch_key].str = sdo_data.to_string(object.data_type, 2);
        
        std::lock_guard<std::mutex> history_lock(_history_mtx);
        auto history_item = _history.find(watch_key);
        float time = std::chrono::duration_cast<std::chrono::microseconds>(now - _history_start).count()/1000000.0f;
        float value = sdo_data.f32();
        history_item->second.push_back({time, value});

        return FrameHandlingStatus::success;
    }
    return FrameHandlingStatus::irrelevant_frame;
}


FrameHandlingStatus ServerWatchService::handle_tpdo(CobTpdo tpdo, const can_payload& payload) {
    auto now = std::chrono::steady_clock::now();
    auto tpdo_data = _unmap_tpdo(tpdo, payload);

    for (const auto& item : tpdo_data) {
        WatchKey watch_key{std::get<0>(item), std::get<1>(item)};
        
        std::lock_guard<std::mutex> history_lock(_history_mtx);
        auto history_item = _history.find(watch_key);
        float time = std::chrono::duration_cast<std::chrono::microseconds>(now - _history_start).count()/1000000.0f;
        history_item->second.push_back({time, std::get<2>(item)});
    }

    return FrameHandlingStatus::success;
}


std::vector<std::tuple<std::string_view, std::string_view, float>> ServerWatchService::_unmap_tpdo(CobTpdo tpdo, const can_payload& payload) const {
    if (!_tpdo_mapping.contains(tpdo)) {
        return {};
    }

    std::vector<std::tuple<std::string_view, std::string_view, float>> ret;
    uint64_t data = from_payload<uint64_t>(payload);

    for (const auto& mapping : _tpdo_mapping.at(tpdo)) {
        ret.push_back({mapping.subcategory, mapping.name, float((data >> mapping.offset) & mapping.mask)});
    }

    return ret;
}


} // namespace ucanopen
