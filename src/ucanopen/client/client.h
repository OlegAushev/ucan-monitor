#pragma once

#include "../server/server.h"
#include "../ucanopen_def.h"
#include <atomic>
#include <cansocket/cansocket.h>
#include <chrono>
#include <functional>
#include <future>
#include <iomanip>
#include <map>
#include <sstream>
#include <thread>

#include <bsclog/bsclog.h>

namespace ucanopen {

class Client {
private:
    NodeId _node_id;
    std::shared_ptr<can::Socket> _socket;
    NmtState _nmt_state;

    std::set<std::shared_ptr<Server>> _servers;
    std::map<canid_t, std::shared_ptr<Server>> _rxid_to_server;

    /* SYNC */
    struct SyncMessage {
        bool is_enabled = true;
        std::chrono::milliseconds period{1000};
        std::chrono::time_point<std::chrono::steady_clock> timepoint;
    };
    SyncMessage _sync_msg;

    /* HEARTBEAT */
    struct HeartbeatMessage {
        std::chrono::milliseconds period{1000};
        std::chrono::time_point<std::chrono::steady_clock> timepoint;
    };
    HeartbeatMessage _heartbeat_msg;

    /* TPDO client --> server */
    bool _is_tpdo_enabled = true;
    struct TpdoMessage {
        std::chrono::milliseconds period;
        std::chrono::time_point<std::chrono::steady_clock> timepoint;
        std::function<can_payload(void)> creator;
    };
    std::map<CobTpdo, TpdoMessage> _tpdo_msgs;

    /* THREADS */
    std::thread _thread_run;
    std::promise<void> _signal_exit_run_thread;
    void _run(std::future<void> future_exit);

    void _on_frame_received(const can_frame& frame);

public:
    Client(NodeId node_id, std::shared_ptr<can::Socket> socket);
    ~Client();
    NodeId node_id() const { return _node_id; }
    SetupStatus set_node_id(NodeId nodeId);
    void register_server(std::shared_ptr<Server> server);

    std::vector<std::string> server_names() const {
        std::vector<std::string> names; //(_servers.size());
        for (const auto& server : _servers) {
            names.push_back(server->name());
        }
        return names;
    }

    std::shared_ptr<Server> server(std::string_view name) {
        auto server_iter = std::find_if(
                _servers.begin(),
                _servers.end(),
                [name](const auto& s) { return s->name() == name; });
        if (server_iter == _servers.end()) {
            return nullptr;
        }
        return *server_iter;
    }

    SetupStatus set_server_node_id(std::string_view name, NodeId node_id);

    void enable_sync() {
        _sync_msg.is_enabled = true;
        bsclog::info("Enabled uCANopen client SYNC messages, period = {}.",
                     _sync_msg.period);
    }

    void disable_sync() {
        _sync_msg.is_enabled = false;
        bsclog::info("Disabled uCANopen client SYNC messages.");
    }

    void set_sync_period(std::chrono::milliseconds period) {
        _sync_msg.period = period;
        bsclog::info("Set uCANopen client SYNC messages period to {}.", period);
    }

    void set_heartbeat_period(std::chrono::milliseconds period) {
        _heartbeat_msg.period = period;
        bsclog::info("Set uCANopen client HEARTBEAT messages period to {}.",
                     period);
    }

    void register_tpdo(CobTpdo tpdo_type,
                       std::chrono::milliseconds period,
                       std::function<can_payload(void)> creator) {
        _tpdo_msgs.insert(
                {tpdo_type,
                 {period, std::chrono::steady_clock::now(), creator}});
    }

    void enable_tpdo() {
        _is_tpdo_enabled = true;
        bsclog::info("Enabled uCANopen client TPDO messages.");
    }

    void disable_tpdo() {
        _is_tpdo_enabled = false;
        bsclog::info("Disabled uCANopen client TPDO messages.");
    }

    void enable_rpdo_on_server(std::string_view name) {
        auto server = _get_server(name);
        if (server == nullptr) {
            bsclog::error(
                    "Failed to enable RPDO messages on {} server: server not found.",
                    name);
            return;
        }
        server->rpdo_service.enable();
        bsclog::info("Enabled {} server RPDO messages.", name);
    }

    void disable_rpdo_on_server(std::string_view name) {
        auto server = _get_server(name);
        if (server == nullptr) {
            bsclog::error(
                    "Failed to disable RPDO messages on {} server: server not found.",
                    name);
            return;
        }
        server->rpdo_service.disable();
        bsclog::info("Disabled {} server RPDO messages.", name);
    }

    void enable_watch_on_server(std::string_view name) {
        auto server = _get_server(name);
        if (server == nullptr) {
            bsclog::error(
                    "Failed to enable watch messages on {} server: server not found.",
                    name);
            return;
        }
        server->watch_service.enable();
        bsclog::info("Enabled {} server watch messages.", name);
    }

    void disable_watch_on_server(std::string_view name) {
        auto server = _get_server(name);
        if (server == nullptr) {
            bsclog::error(
                    "Failed to disable watch messages on {} server: server not found.",
                    name);
            return;
        }
        server->watch_service.disable();
        bsclog::info("Disabled {} server watch messages.", name);
    }

    void set_watch_period_on_server(std::string_view name,
                                    std::chrono::milliseconds period) {
        auto server = _get_server(name);
        if (server == nullptr) {
            bsclog::error(
                    "Failed to set watch messages period on {} server: server not found.",
                    name);
            return;
        }
        server->watch_service.set_period(period);
        bsclog::info("Set {} server watch messages period to {}.",
                     name,
                     period);
    }

    void set_log_capacity_on_server(std::string_view name, size_t size) {
        auto server = _get_server(name);
        if (server == nullptr) {
            bsclog::error(
                    "Failed to set watch history size on {} server: server not found.",
                    name);
            return;
        }
        server->log_service.set_log_capacity(size);
        bsclog::info("Set {} server watch history size to {}.", name, size);
    }

protected:
    void _register_rx_messages(std::shared_ptr<Server> server);
    void _unregister_rx_messages(std::shared_ptr<Server> server);
    bool _is_free(NodeId nodeId) const;

    std::shared_ptr<Server> _get_server(std::string_view name) {
        auto server_iter = std::find_if(
                _servers.begin(),
                _servers.end(),
                [name](const auto& s) { return s->name() == name; });
        if (server_iter == _servers.end()) {
            return std::shared_ptr<Server>(nullptr);
        }
        return *server_iter;
    }
};

} // namespace ucanopen
