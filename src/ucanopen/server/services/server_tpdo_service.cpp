#include "server_tpdo_service.h"


namespace ucanopen {

ServerTpdoService::ServerTpdoService(impl::Server& server)
        : _server(server) {
}


void ServerTpdoService::register_tpdo(TpdoType tpdo_type, std::chrono::milliseconds timeout, std::function<void(const can_payload&)> handler) {
    canid_t id = calculate_cob_id(to_cob_type(tpdo_type), _server.node_id());
    _tpdo_list.emplace(tpdo_type, Message{id, timeout, std::chrono::steady_clock::now(), can_payload{}, handler});
}


void ServerTpdoService::update_node_id() {
    std::lock_guard<std::mutex> lock(_mtx);
    for (auto& [tpdo_type, message] : _tpdo_list) {
        message.id = calculate_cob_id(to_cob_type(tpdo_type), _server.node_id());
    }
}


FrameHandlingStatus ServerTpdoService::handle_frame(const can_frame& frame) {
    std::lock_guard<std::mutex> lock(_mtx);
    for (auto& [tpdo_type, message] : _tpdo_list) {
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

