#include "server_log_service.h"


namespace ucanopen {


ServerLogService::ServerLogService(impl::Server& server, impl::SdoPublisher& sdo_publisher, impl::TpdoPublisher& tpdo_publisher)
        : SdoSubscriber(sdo_publisher)
        , TpdoSubscriber(tpdo_publisher)
        , _server(server) {
    _start = std::chrono::steady_clock::now();

    for (const auto& [key, object] : _server.dictionary().entries) {
        // add watch objects to log
        if (object.category == _server.dictionary().config.watch_category && object.data_type == OD_FLOAT32) {
            _objects.push_back(&object);
            _log.insert({LogKey{object.subcategory, object.name}, LogBuf(_log_capacity)});
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
            if (std::find(_objects.begin(), _objects.end(), &object) != _objects.end()) {
                // avoid duplicating objects
                _objects.push_back(&object);
            }
            _log.insert({LogKey{object.subcategory, object.name}, LogBuf(_log_capacity)});

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


FrameHandlingStatus ServerLogService::handle_sdo(ODEntryIter entry, SdoType sdo_type, ExpeditedSdoData sdo_data) {
    auto now = std::chrono::steady_clock::now();
    const auto& [key, object] = *entry;

    if ((object.category == _server.dictionary().config.watch_category)
            && (object.data_type == OD_FLOAT32)
            && (sdo_type == SdoType::response_to_read)) {
        LogKey log_key{object.subcategory, object.name};
        
        std::lock_guard<std::mutex> lock(_log_mtx);
        
        auto log = _log.find(log_key);
        if (log == _log.end()) {
            return FrameHandlingStatus::irrelevant_frame;
        }

        float time = std::chrono::duration_cast<std::chrono::microseconds>(now - _start).count()/1000000.0f;
        float value = sdo_data.f32();
        log->second.push_back({time, value});

        return FrameHandlingStatus::success;
    }
    return FrameHandlingStatus::irrelevant_frame;
}


FrameHandlingStatus ServerLogService::handle_tpdo(CobTpdo tpdo, const can_payload& payload) {
    auto now = std::chrono::steady_clock::now();
    auto tpdo_data = _unmap_tpdo(tpdo, payload);

    for (const auto& item : tpdo_data) {
        LogKey log_key{std::get<0>(item), std::get<1>(item)};
        
        std::lock_guard<std::mutex> lock(_log_mtx);
        
        auto log = _log.find(log_key);
        if (log == _log.end()) {
            return FrameHandlingStatus::irrelevant_frame;
        }

        float time = std::chrono::duration_cast<std::chrono::microseconds>(now - _start).count()/1000000.0f;
        log->second.push_back({time, std::get<2>(item)});
    }

    return FrameHandlingStatus::success;
}


std::vector<std::tuple<std::string_view, std::string_view, float>> ServerLogService::_unmap_tpdo(CobTpdo tpdo, const can_payload& payload) const {
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


} // namepsace ucanopen
