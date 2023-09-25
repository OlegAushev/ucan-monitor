#include "server_watch_service.h"


namespace ucanopen {


ServerWatchService::ServerWatchService(impl::Server& server, impl::SdoPublisher& sdo_publisher)
        : SdoSubscriber(sdo_publisher)
        , _server(server) {
    _timepoint = std::chrono::steady_clock::now();

    for (const auto& [key, object] : _server.dictionary().entries) {
        // create watch entries list and data map
        if (object.category == _server.dictionary().config.watch_category) {
            _object_list.push_back(std::make_pair(object.subcategory, object.name));
            _data.insert({{object.subcategory, object.name}, {ExpeditedSdoData{}, "..."}});
        }
    }
}


}
