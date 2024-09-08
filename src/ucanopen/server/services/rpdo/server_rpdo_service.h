#pragma once


#include <ucanopen/server/impl/impl_server.h>
#include <functional>


namespace ucanopen {

class ServerRpdoService {
private:
    impl::Server& _server;
    bool _is_enabled{false};

    struct Message {
        canid_t id;
        std::chrono::milliseconds period;
        std::chrono::time_point<std::chrono::steady_clock> timepoint;
        std::function<can_payload(void)> creator;
    };
    std::map<CobRpdo, Message> _rpdo_msgs;
public:
    ServerRpdoService(impl::Server& server);
    void register_rpdo(CobRpdo rpdo, std::chrono::milliseconds period, std::function<can_payload(void)> creator);
    void update_node_id();

    void enable() {
        _is_enabled = true;
    }

    void disable() {
        _is_enabled = false;
    }

    void send() {
        if (_is_enabled) {
            auto now = std::chrono::steady_clock::now();
            for (auto& [rpdo, message] : _rpdo_msgs) {
                if (message.period == std::chrono::milliseconds(0)) { continue; }  
                if (now - message.timepoint < message.period) { continue; }
                can_payload payload = message.creator();
                _server._socket->send(create_frame(message.id, 8, payload));
                message.timepoint = now;	
            }
        }
    }
};


} // namespace ucanopen
