#include "server_heartbeat_service.h"


namespace ucanopen {


ServerHeartbeatService::ServerHeartbeatService(impl::Server& server, std::chrono::milliseconds timeout)
        : _server(server) {
    _id = calculate_cob_id(Cob::heartbeat, _server.node_id());
    _timeout = timeout;
    _timepoint = std::chrono::steady_clock::now();
}


void ServerHeartbeatService::update_node_id() {
    std::lock_guard<std::mutex> lock(_mtx);
    _id = calculate_cob_id(Cob::heartbeat, _server.node_id());
}


FrameHandlingStatus ServerHeartbeatService::handle_frame(const can_frame& frame) {
    std::lock_guard<std::mutex> lock(_mtx);
    if (frame.can_id != _id) { return FrameHandlingStatus::id_mismatch; }
    
    _timepoint = std::chrono::steady_clock::now();
    _server._nmt_state = static_cast<NmtState>(frame.data[0]);
    return FrameHandlingStatus::success;
}


} // namespace ucanopen
