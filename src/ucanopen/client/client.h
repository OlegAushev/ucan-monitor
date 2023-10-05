#pragma once


#include "../ucanopen_def.h"
#include "../server/server.h"
#include <cansocket/cansocket.h>
#include <sstream>
#include <iomanip>
#include <functional>
#include <thread>
#include <future>
#include <atomic>
#include <chrono>
#include <map>

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
    struct SyncInfo {
        bool is_enabled = false;
        std::chrono::milliseconds period = std::chrono::milliseconds(1000);
        std::chrono::time_point<std::chrono::steady_clock> timepoint;		
    };
    SyncInfo _sync_info;

    /* HEARTBEAT */
    struct HeartbeatInfo {
        std::chrono::milliseconds period = std::chrono::milliseconds(1000);
        std::chrono::time_point<std::chrono::steady_clock> timepoint;
    };
    HeartbeatInfo _heartbeat_info;
            
    /* TPDO client --> server */
    bool _is_tpdo_enabled = false;
    struct TpdoInfo {
        std::chrono::milliseconds period;
        std::chrono::time_point<std::chrono::steady_clock> timepoint;
        std::function<can_payload(void)> creator;
    };
    std::map<TpdoType, TpdoInfo> _tpdo_list;

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
        std::vector<std::string> names;//(_servers.size());
        for (const auto& server : _servers) {
            names.push_back(server->name());
        }
        return names;
    }

    std::shared_ptr<Server> server(std::string_view name) {
        auto server_iter = std::find_if(_servers.begin(), _servers.end(),
                [name](const auto& s) { return s->name() == name; });
        if (server_iter == _servers.end()) {
            return nullptr;
        }
        return *server_iter;
    }

    SetupStatus set_server_node_id(std::string_view name, NodeId node_id);

    void enable_sync() {
        _sync_info.is_enabled = true;
        bsclog::info("Enabled uCANopen client SYNC messages, period = {}.", _sync_info.period);
    }

    void disable_sync() {
        _sync_info.is_enabled = false;
        bsclog::info("Disabled uCANopen client SYNC messages.");
    }

    void set_sync_period(std::chrono::milliseconds period) {
        _sync_info.period = period;
        bsclog::info("Set uCANopen client SYNC messages period = {}.", period);
    }

    void set_heartbeat_period(std::chrono::milliseconds period) {
        _heartbeat_info.period = period;
        bsclog::info("Set uCANopen client HEARTBEAT messages period = {}.", period);
    }

    void register_tpdo(TpdoType tpdo_type, std::chrono::milliseconds period, std::function<can_payload(void)> creator) {
        _tpdo_list.insert({tpdo_type, {period, std::chrono::steady_clock::now(), creator}});
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
            bsclog::error("Failed to enable RPDO messages on {} server: server not found.", name);
            return;
        }
        server->rpdo_service.enable();
        bsclog::info("Enabled {} server RPDO messages.", name);
    }

    void disable_rpdo_on_server(std::string_view name) {
        auto server = _get_server(name);
        if (server == nullptr) {
            bsclog::error("Failed to disable RPDO messages on {} server: server not found.", name);
            return;
        }
        server->rpdo_service.disable();
        bsclog::info("Disabled {} server RPDO messages.", name);
    }

    void enable_watch_on_server(std::string_view name) {
        auto server = _get_server(name);
        if (server == nullptr) {
            bsclog::error("Failed to enable watch messages on {} server: server not found.", name);
            return;
        }
        server->watch_service.enable();
        bsclog::info("Enabled {} server watch messages.", name);
    }

    void disable_watch_on_server(std::string_view name) {
        auto server = _get_server(name);
        if (server == nullptr) {
            bsclog::error("Failed to disable watch messages on {} server: server not found.", name);
            return;
        }
        server->watch_service.disable();
        bsclog::info("Disabled {} server watch messages.", name);
    }

    void set_watch_period_on_server(std::string_view name, std::chrono::milliseconds period) {
        auto server = _get_server(name);
        if (server == nullptr) {
            bsclog::error("Failed to set watch messages period on {} server: server not found.", name);
            return;
        }
        server->watch_service.set_period(period);
        bsclog::info("Set {} server watch messages to {}.", name, period);
    }

protected:
    void _register_rx_messages(std::shared_ptr<Server> server);
    void _unregister_rx_messages(std::shared_ptr<Server> server);
    bool _is_free(NodeId nodeId) const;

    std::shared_ptr<Server> _get_server(std::string_view name) {
        auto server_iter = std::find_if(_servers.begin(), _servers.end(),
                [name](const auto& s) { return s->name() == name; });
        if (server_iter == _servers.end()) {
            return std::shared_ptr<Server>(nullptr);
        }
        return *server_iter;
    }
};


} // namespace ucanopen
