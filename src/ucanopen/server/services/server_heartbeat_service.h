#pragma once


#include "../impl/impl_server.h"


namespace ucanopen {


class ServerHeartbeatService : public impl::FrameReceiverInterface {
private:
    impl::Server& _server;
    canid_t _id;
    std::chrono::milliseconds _timeout;
    std::chrono::time_point<std::chrono::steady_clock> _timepoint;
    mutable std::mutex _mtx;
public:
    ServerHeartbeatService(impl::Server& server, std::chrono::milliseconds timeout);

    bool is_ok() const {
        std::lock_guard<std::mutex> lock(_mtx);
        return ((std::chrono::steady_clock::now() - _timepoint) <= _timeout)
            && (_server.nmt_state() == NmtState::operational);
    }
    
    void update_node_id();
    virtual FrameHandlingStatus handle_frame(const can_frame& frame) override;
};


} // namespace ucanopen
