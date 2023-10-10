#include "server_watch_service.h"
#include <bsclog/bsclog.h>


namespace ucanopen {


ServerWatchService::ServerWatchService(impl::Server& server, impl::SdoPublisher& sdo_publisher)
        : SdoSubscriber(sdo_publisher)
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
}


void ServerWatchService::send() {
    if (_enabled && !_objects.empty()) {
        auto now = std::chrono::steady_clock::now();
        if (now - _acq_timepoint >= _period) {
            static int i = 0;
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
        WatchKey watch_key = std::make_pair(object.subcategory, object.name);
        _current_data[watch_key].raw = sdo_data;
        if (object.type != OD_ENUM16) {
            _current_data[watch_key].str = sdo_data.to_string(object.type, 2);
        }
        
        std::lock_guard<std::mutex> history_lock(_history_mtx);
        auto item = _history.find(watch_key);
        float time = std::chrono::duration_cast<std::chrono::microseconds>(now - _history_start).count()/1000000.0f;
        float value = sdo_data.f32();
        item->second.push_back({time, value});

        return FrameHandlingStatus::success;
    }
    return FrameHandlingStatus::irrelevant_frame;
}


} // namespace ucanopen
