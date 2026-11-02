#pragma once


#include <ucanopen/server/impl/impl_server.h>


namespace ucanopen {


class ServerHeartbeatService : public impl::FrameReceiver {
private:
    impl::Server& _server;
    canid_t _id;
    std::chrono::milliseconds _timeout;
    std::chrono::time_point<std::chrono::steady_clock> _timepoint{};
    unsigned _epoch{0};
    mutable std::mutex _mtx;
public:
    ServerHeartbeatService(impl::Server& server, std::chrono::milliseconds timeout);

    bool good() const {
        std::lock_guard<std::mutex> lock(_mtx);
        return ((std::chrono::steady_clock::now() - _timepoint) <= _timeout)
            && (_server.nmt_state() == NmtState::operational);
    }
    
    // The connection to the device, counted: a new one begins with its
    // boot-up, with a heartbeat after silence and with a change of node-ID.
    // What a service learned of the device holds for one connection.
    unsigned epoch() const {
        std::lock_guard<std::mutex> lock(_mtx);
        return _epoch;
    }

    void update_node_id();
    virtual FrameHandlingStatus handle_frame(const can_frame& frame) override;
};


} // namespace ucanopen
