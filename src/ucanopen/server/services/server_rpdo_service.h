#pragma once


#include "../impl/impl_server.h"
#include <functional>


namespace ucanopen {

class ServerRpdoService {
private:
    impl::Server& _server;
    bool _is_enabled = false;

    struct Message {
        canid_t id;
        std::chrono::milliseconds period;
        std::chrono::time_point<std::chrono::steady_clock> timepoint;
        std::function<can_payload(void)> creator;
    };
    std::map<RpdoType, Message> _rpdo_list;
public:
    ServerRpdoService(impl::Server& server);
    void register_rpdo(RpdoType rpdo_type, std::chrono::milliseconds period, std::function<can_payload(void)> creator);
    void update_node_id();

    void enable() {
        _is_enabled = true;
        Log() << "Enabled uCANopen server {" << _server.name() << "} RPDO messages.\n" << LogPrefix::ok;
    }

    void disable() {
        _is_enabled = false;
        Log() << "Disabled uCANopen server {" << _server.name() << "} RPDO messages.\n" << LogPrefix::ok;
    }

    void send() {
        if (_is_enabled) {
            for (auto& [rpdo_type, message] : _rpdo_list) {
                if (message.period == std::chrono::milliseconds(0)) { continue; }
                
                auto now = std::chrono::steady_clock::now();
                if (now - message.timepoint < message.period) { continue; }

                can_payload payload = message.creator();
                _server._socket->send(create_frame(message.id, 8, payload));
                message.timepoint = now;	
            }
        }
    }
};

} // namespace ucanopen

