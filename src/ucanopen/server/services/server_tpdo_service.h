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
    std::map<TpdoType, Message> _tpdo_list;
    mutable std::mutex _mtx;
public:
    ServerTpdoService(impl::Server& server);
    void register_tpdo(TpdoType tpdo_type, std::chrono::milliseconds timeout, std::function<void(const can_payload&)> handler);
    void update_node_id();

    bool is_ok(TpdoType tpdo_type) const {
        std::lock_guard<std::mutex> lock(_mtx);
        if (!_tpdo_list.contains(tpdo_type)) { return false; };
        auto now = std::chrono::steady_clock::now();
        return (now - _tpdo_list.at(tpdo_type).timepoint) <= _tpdo_list.at(tpdo_type).timeout;
    }

    can_payload data(TpdoType tpdo_type) const {
        std::lock_guard<std::mutex> lock(_mtx);
        if (!_tpdo_list.contains(tpdo_type)) { return can_payload{}; }
        return _tpdo_list.at(tpdo_type).payload;
    }

    virtual FrameHandlingStatus handle_frame(const can_frame& frame) override;
};


} // namespace ucanopen
