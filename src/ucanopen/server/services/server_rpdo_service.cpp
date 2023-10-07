#include "server_rpdo_service.h"


namespace ucanopen {

ServerRpdoService::ServerRpdoService(impl::Server& server)
        : _server(server) {
}


void ServerRpdoService::register_rpdo(CobRpdo rpdo, std::chrono::milliseconds period, std::function<can_payload(void)> creator) {
    canid_t id = calculate_cob_id(to_cob(rpdo), _server.node_id());
    _rpdo_msgs.insert({rpdo, {id, period, std::chrono::steady_clock::now(), creator}});
}


void ServerRpdoService::update_node_id() {
    for (auto& [rpdo, message] : _rpdo_msgs) {
        message.id = calculate_cob_id(to_cob(rpdo), _server.node_id());
    }
}


} // namespace ucanopen
