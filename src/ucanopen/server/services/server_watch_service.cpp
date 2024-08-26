#include "server_watch_service.h"
#include <bsclog/bsclog.h>


namespace ucanopen {


ServerWatchService::ServerWatchService(impl::Server& server, impl::SdoPublisher& sdo_publisher)
        : SdoSubscriber(sdo_publisher)
        , _server(server) {
    _daq_timepoint = std::chrono::steady_clock::now();

    for (const auto& [key, object] : _server.dictionary().entries) {
        // create watch object list and data map
        if (object.category == _server.dictionary().config.watch_category) {
            _objects.push_back(&object);
            _object_daq_enabled.push_back(true);
            _data.insert({WatchKey{object.subcategory, object.name}, {ExpeditedSdoData{}, "н/д"}});
        }
    }
}


void ServerWatchService::send() {
    std::shared_lock lock(_objects_mtx);
    if (_enabled && !_objects.empty()) {
        auto now = std::chrono::steady_clock::now();
        if (now - _daq_timepoint >= _period) {
            static size_t i = 0;
            if (_object_daq_enabled[i]) {
                _server.read(_server.dictionary().config.watch_category,
                            _objects[i]->subcategory,
                            _objects[i]->name);
                _daq_timepoint = now;
            }
            i = (i + 1) % _objects.size();
        }
    }
}


FrameHandlingStatus ServerWatchService::handle_sdo(ODEntryIter entry, SdoType sdo_type, ExpeditedSdoData sdo_data) {
    const auto& [key, object] = *entry;

    if ((object.category == _server.dictionary().config.watch_category) && (sdo_type == SdoType::response_to_read)) {
        WatchKey watch_key{object.subcategory, object.name};
        std::lock_guard<std::mutex> lock(_data_mtx);
        
        auto data = _data.find(watch_key);
        if (data == _data.end()) {
            return FrameHandlingStatus::irrelevant_frame;
        }

        data->second.raw = sdo_data;
        data->second.str = sdo_data.to_string(object.data_type, 2);
        return FrameHandlingStatus::success;
    }
    return FrameHandlingStatus::irrelevant_frame;
}


} // namespace ucanopen
