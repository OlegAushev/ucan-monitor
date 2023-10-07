#pragma once


#include "../impl/impl_server.h"
#include <functional>


namespace ucanopen {


class ServerTpdoService : public impl::FrameReceiverInterface {
private:
    impl::Server& _server;

    struct Message {
        canid_t id;
        std::chrono::milliseconds timeout;
        std::chrono::time_point<std::chrono::steady_clock> timepoint;
        can_payload payload;
        std::function<void(const can_payload&)> handler;
    };
    std::map<CobTpdo, Message> _tpdo_msgs;
    mutable std::mutex _mtx;
public:
    ServerTpdoService(impl::Server& server);
    void register_tpdo(CobTpdo tpdo, std::chrono::milliseconds timeout, std::function<void(const can_payload&)> handler);
    void update_node_id();

    bool is_ok(CobTpdo tpdo) const {
        std::lock_guard<std::mutex> lock(_mtx);
        if (!_tpdo_msgs.contains(tpdo)) { return false; };
        auto now = std::chrono::steady_clock::now();
        return (now - _tpdo_msgs.at(tpdo).timepoint) <= _tpdo_msgs.at(tpdo).timeout;
    }

    can_payload data(CobTpdo tpdo) const {
        std::lock_guard<std::mutex> lock(_mtx);
        if (!_tpdo_msgs.contains(tpdo)) { return can_payload{}; }
        return _tpdo_msgs.at(tpdo).payload;
    }

    virtual FrameHandlingStatus handle_frame(const can_frame& frame) override;
};


} // namespace ucanopen
