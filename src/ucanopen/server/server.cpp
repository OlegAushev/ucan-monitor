#include "server.h"

#include <ucanopen/od/pdo_codec.h>


namespace ucanopen {

Server::Server(std::shared_ptr<can::Socket> socket, NodeId node_id, const std::string& name,
               std::shared_ptr<const od::Description> description)
        : impl::Server(socket, node_id, name, std::move(description))
        , heartbeat_service(*this, std::chrono::milliseconds(2000))
        , tpdo_service(*this)
        , rpdo_service(*this)
        , sdo_service(*this)
        , watch_service(*this, sdo_service, heartbeat_service)
        , config_service(sdo_service)
        , info_service(*this, sdo_service, heartbeat_service)
        , log_service(*this, sdo_service)
        , status_service(*this) {
    _rx_services.push_back(&sdo_service);
    _rx_services.push_back(&tpdo_service);
    _rx_services.push_back(&heartbeat_service);

    // A server whose description states its statuses takes their MPDO here,
    // whatever the device.
    if (status_service.available()) {
        auto const tpdo = od::tpdo_of(*status_service.pdo());
        if (tpdo.has_value()) {
            tpdo_service.register_tpdo(*tpdo, status_service.timeout(), [this](const can_payload& payload) {
                status_service.handle_frame(payload);
            });
        } else {
            bsclog::error("{}: статусы идут в PDO «{}», а ucanopen принимает только TPDO1–TPDO4 предопределённого "
                          "набора.", _name, status_service.pdo()->name);
        }
    }
}


void Server::_set_node_id(NodeId node_id) {
    if (!node_id.valid()) { return; }

    _node_id = node_id;

    heartbeat_service.update_node_id();
    tpdo_service.update_node_id();
    rpdo_service.update_node_id();
    sdo_service.update_node_id();
}


void Server::_send() {
    rpdo_service.send();
    watch_service.send();
    config_service.send();
    info_service.send();
}


void Server::_handle_frame(const can_frame& frame) {
    for (auto service : _rx_services) {
        auto status = service->handle_frame(frame);
        if (status == FrameHandlingStatus::success) {
            break;
        }
    }
}


} // namespace ucanopen
