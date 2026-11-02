#include "server_heartbeat_service.h"


namespace ucanopen {


ServerHeartbeatService::ServerHeartbeatService(impl::Server& server, std::chrono::milliseconds timeout)
        : _server(server) {
    _id = calculate_cob_id(Cob::heartbeat, _server.node_id());
    _timeout = timeout;
}


void ServerHeartbeatService::update_node_id() {
    std::lock_guard<std::mutex> lock(_mtx);
    _id = calculate_cob_id(Cob::heartbeat, _server.node_id());
    ++_epoch;
}


FrameHandlingStatus ServerHeartbeatService::handle_frame(const can_frame& frame) {
    std::lock_guard<std::mutex> lock(_mtx);
    if (frame.can_id != _id) { return FrameHandlingStatus::id_mismatch; }
    
    auto const now = std::chrono::steady_clock::now();
    auto const state = static_cast<NmtState>(frame.data[0]);
    if (state == NmtState::initialization || (now - _timepoint) > _timeout) {
        ++_epoch;
    }
    _timepoint = now;
    _server._nmt_state = state;
    return FrameHandlingStatus::success;
}


} // namespace ucanopen
