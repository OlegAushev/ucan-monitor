#include "server_rpdo_service.h"


namespace ucanopen {

ServerRpdoService::ServerRpdoService(impl::Server& server)
        : _server(server) {
}


void ServerRpdoService::register_rpdo(RpdoType rpdo_type, std::chrono::milliseconds period, std::function<can_payload(void)> creator) {
    canid_t id = calculate_cob_id(to_cob_type(rpdo_type), _server.node_id());
    _rpdo_list.insert({rpdo_type, {id, period, std::chrono::steady_clock::now(), creator}});
}


void ServerRpdoService::update_node_id() {
    for (auto& [rpdo_type, message] : _rpdo_list) {
        message.id = calculate_cob_id(to_cob_type(rpdo_type), _server.node_id());
    }
}

} // namespace ucanopen

