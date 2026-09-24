#pragma once

#include <functional>
#include <map>
#include <mutex>
#include <ucanopen/server/impl/impl_server.h>

namespace ucanopen {

class ServerRpdoService {
private:
    impl::Server& _server;
    bool _is_enabled{true};

    struct Message {
        canid_t id;
        std::chrono::milliseconds period;
        std::chrono::time_point<std::chrono::steady_clock> timepoint;
        std::function<can_payload(void)> creator;
        bool is_enabled{true};
    };
    std::map<CobRpdo, Message> _rpdo_msgs;
    mutable std::mutex _mtx;
public:
    ServerRpdoService(impl::Server& server);
    void register_rpdo(CobRpdo rpdo,
                       std::chrono::milliseconds period,
                       std::function<can_payload(void)> creator);
    void update_node_id();

    void enable() { _is_enabled = true; }

    void disable() { _is_enabled = false; }

    // A single RPDO is switched apart from the others: a node may take its
    // commands from another master while this client still streams it the
    // rest. The creator of a disabled RPDO is not called, so a counter it
    // keeps resumes in sequence.
    void enable(CobRpdo rpdo) { _set_enabled(rpdo, true); }

    void disable(CobRpdo rpdo) { _set_enabled(rpdo, false); }

    bool enabled(CobRpdo rpdo) const {
        std::lock_guard<std::mutex> lock(_mtx);
        auto iter = _rpdo_msgs.find(rpdo);
        return iter != _rpdo_msgs.end() && iter->second.is_enabled;
    }

    void send() {
        if (_is_enabled) {
            std::lock_guard<std::mutex> lock(_mtx);
            auto now = std::chrono::steady_clock::now();
            for (auto& [rpdo, message] : _rpdo_msgs) {
                if (!message.is_enabled) {
                    continue;
                }
                if (message.period == std::chrono::milliseconds(0)) {
                    continue;
                }
                if (now - message.timepoint < message.period) {
                    continue;
                }
                can_payload payload = message.creator();
                _server._socket->send(create_frame(message.id, 8, payload));
                message.timepoint = now;
            }
        }
    }
private:
    void _set_enabled(CobRpdo rpdo, bool enabled) {
        std::lock_guard<std::mutex> lock(_mtx);
        auto iter = _rpdo_msgs.find(rpdo);
        if (iter != _rpdo_msgs.end()) {
            iter->second.is_enabled = enabled;
        }
    }
};

} // namespace ucanopen
