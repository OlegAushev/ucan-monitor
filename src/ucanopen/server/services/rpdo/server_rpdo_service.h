#pragma once

#include <functional>
#include <map>
#include <mutex>
#include <optional>
#include <ucanopen/server/impl/impl_server.h>
#include <utility>

namespace ucanopen {

class ServerRpdoService {
private:
    impl::Server& _server;
    bool _is_enabled{true};

    // A creator that gives no frame skips the period: the device, which
    // watches the RPDO, sees it missing rather than a frame made up for it.
    struct Message {
        canid_t id;
        std::chrono::milliseconds period;
        std::chrono::time_point<std::chrono::steady_clock> timepoint;
        std::function<std::optional<can_payload>(void)> creator;
        bool is_enabled{true};
        std::function<bool(void)> interlock{};
        std::optional<can_payload> sent{}; // the last frame sent
    };
    std::map<CobRpdo, Message> _rpdo_msgs;
    mutable std::mutex _mtx;
public:
    ServerRpdoService(impl::Server& server);
    void register_rpdo(CobRpdo rpdo,
                       std::chrono::milliseconds period,
                       std::function<std::optional<can_payload>(void)> creator);
    void update_node_id();

    bool enabled() const { return _is_enabled; }

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

    // An RPDO that another master on the bus sends as well. While interlock()
    // holds, the RPDO is switched off and cannot be switched on; it stays off
    // once the master is gone, so a node the master has left does not pick up
    // whatever this client asked of it last.
    void set_interlock(CobRpdo rpdo, std::function<bool(void)> interlock) {
        std::lock_guard<std::mutex> lock(_mtx);
        auto iter = _rpdo_msgs.find(rpdo);
        if (iter != _rpdo_msgs.end()) {
            iter->second.interlock = std::move(interlock);
        }
    }

    // The last frame sent of the RPDO, none before the first.
    std::optional<can_payload> data(CobRpdo rpdo) const {
        std::lock_guard<std::mutex> lock(_mtx);
        auto iter = _rpdo_msgs.find(rpdo);
        if (iter == _rpdo_msgs.end()) {
            return std::nullopt;
        }
        return iter->second.sent;
    }

    bool interlocked(CobRpdo rpdo) const {
        std::lock_guard<std::mutex> lock(_mtx);
        auto iter = _rpdo_msgs.find(rpdo);
        return iter != _rpdo_msgs.end() && _engaged(iter->second);
    }

    void send() {
        if (_is_enabled) {
            std::lock_guard<std::mutex> lock(_mtx);
            auto now = std::chrono::steady_clock::now();
            for (auto& [rpdo, message] : _rpdo_msgs) {
                if (!message.is_enabled) {
                    continue;
                }
                if (_engaged(message)) {
                    message.is_enabled = false;
                    continue;
                }
                if (message.period == std::chrono::milliseconds(0)) {
                    continue;
                }
                if (now - message.timepoint < message.period) {
                    continue;
                }
                message.timepoint = now;
                auto payload = message.creator();
                if (!payload.has_value()) {
                    continue;
                }
                _server._socket->send(create_frame(message.id, 8, *payload));
                message.sent = payload;
            }
        }
    }
private:
    static bool _engaged(const Message& message) {
        return message.interlock && message.interlock();
    }

    void _set_enabled(CobRpdo rpdo, bool enabled) {
        std::lock_guard<std::mutex> lock(_mtx);
        auto iter = _rpdo_msgs.find(rpdo);
        if (iter == _rpdo_msgs.end()) {
            return;
        }
        if (enabled && _engaged(iter->second)) {
            return;
        }
        iter->second.is_enabled = enabled;
    }
};

} // namespace ucanopen
