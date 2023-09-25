#include "server_config_service.h"


namespace ucanopen {

ServerConfigService::ServerConfigService(impl::Server& server)
        : _server(server) {
    for (const auto& [key, object] : server.dictionary().entries) {
        // create conf entries list
        if (object.category == server.dictionary().config.config_category) {
            _object_list[object.subcategory].push_back(object.name);
        }
    }
}

} // namespace ucanopen

