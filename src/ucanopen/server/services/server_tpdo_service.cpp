#include "server_tpdo_service.h"


namespace ucanopen {

ServerTpdoService::ServerTpdoService(impl::Server& server)
        : _server(server) {
}


void ServerTpdoService::register_tpdo(CobTpdo tpdo, std::chrono::milliseconds timeout, std::function<void(const can_payload&)> handler) {
    canid_t id = calculate_cob_id(to_cob(tpdo), _server.node_id());
    _tpdo_msgs.emplace(tpdo, Message{id, timeout, std::chrono::steady_clock::now(), can_payload{}, handler});
}


void ServerTpdoService::update_node_id() {
    std::lock_guard<std::mutex> lock(_mtx);
    for (auto& [tpdo, message] : _tpdo_msgs) {
        message.id = calculate_cob_id(to_cob(tpdo), _server.node_id());
    }
}


FrameHandlingStatus ServerTpdoService::handle_frame(const can_frame& frame) {
    std::lock_guard<std::mutex> lock(_mtx);
    for (auto& [tpdo, message] : _tpdo_msgs) {
        if (frame.can_id != message.id) { continue; }

        message.timepoint = std::chrono::steady_clock::now();
        can_payload payload{};
        std::copy(frame.data, std::next(frame.data, frame.can_dlc), payload.begin());
        message.payload = payload;
        message.handler(payload);
        return FrameHandlingStatus::success;
    }
    return FrameHandlingStatus::id_mismatch;
}

} // namespace ucanopen

