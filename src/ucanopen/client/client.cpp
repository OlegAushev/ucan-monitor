#include "client.h"


namespace ucanopen {


Client::Client(NodeId node_id, std::shared_ptr<can::Socket> socket)
        : _node_id(node_id)
        , _socket(socket)
        , _nmt_state(NmtState::initialization) {
    _sync_msg.timepoint = std::chrono::steady_clock::now();
    _heartbeat_msg.timepoint = std::chrono::steady_clock::now();

    bsclog::info("Starting aux uCANopen thread...");

    std::future<void> signal_exit = _signal_exit_run_thread.get_future();
    _thread_run = std::thread(&Client::_run, this, std::move(signal_exit));
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    _nmt_state = NmtState::operational;
}


Client::~Client() {
    bsclog::info("Sending signal to aux uCANopen thread to stop...");
    _signal_exit_run_thread.set_value();
    _thread_run.join();
}


SetupStatus Client::set_node_id(NodeId node_id) {
    if (node_id == _node_id) {
        bsclog::info("Refused to set uCANopen client ID to {}(0x{:X}): already set.", node_id.get(), node_id.get());        
        return SetupStatus::success;
    }

    if (!node_id.valid()) {
        bsclog::error("Failed to set uCANopen client ID to {}(0x{:X}): invalid ID.", node_id.get(), node_id.get());
        return SetupStatus::invalid_argument;
    }

    if (!_is_free(node_id)) {
        bsclog::error("Failed to set uCANopen client ID to {}(0x{:X}): occupied ID.", node_id.get(), node_id.get());
        return SetupStatus::invalid_argument;
    }
    
    _node_id = node_id;
    bsclog::success("Set uCANopen client ID to {}(0x{:X})", node_id.get(), node_id.get());
    return SetupStatus::success;
}


void Client::register_server(std::shared_ptr<Server> server) {
    auto server_same_name = std::find_if(_servers.begin(), _servers.end(), 
            [server](const auto& s) { return server->name() == s->name(); });
    if (server_same_name != _servers.end()) {
        bsclog::error("Failed to register uCANopen {} server ID {}(0x{:X}): occupied name.", server->name(), server->node_id().get(), server->node_id().get());
        return;
    }

    auto server_same_id = std::find_if(_servers.begin(), _servers.end(), 
            [server](const auto& s) { return server->node_id() == s->node_id(); });
    if (server_same_id != _servers.end()) {
        bsclog::error("Failed to register uCANopen {} server ID {}(0x{:X}): occupied ID.", server->name(), server->node_id().get(), server->node_id().get());
        return;
    }

    if (server->node_id() == _node_id) {
        bsclog::error("Failed to register uCANopen {} server ID {}(0x{:X}): occupied ID.", server->name(), server->node_id().get(), server->node_id().get());
        return;
    }

    _servers.insert(server);
    _register_rx_messages(server);
    bsclog::success("Registered uCANopen {} server ID {}(0x{:X})", server->name(), server->node_id().get(), server->node_id().get());
}


SetupStatus Client::set_server_node_id(std::string_view name, NodeId node_id) {
    auto server = _get_server(name);
    if (server == nullptr) {
        bsclog::error("Failed to set uCANopen {} server ID to {}(0x{:X}): server not found.", name, node_id.get(), node_id.get());
        return SetupStatus::invalid_argument;
    }

    if (node_id == server->node_id()) {
        bsclog::info("Refused to set uCANopen {} server ID to {}(0x{:X}): already set.", name, node_id.get(), node_id.get());
        return SetupStatus::success;
    }
    
    if (!node_id.valid()) {
        bsclog::error("Failed to set uCANopen {} server ID to {}(0x{:X}): invalid ID.", name, node_id.get(), node_id.get());
        return SetupStatus::invalid_argument;
    }

    if (!_is_free(node_id)) {
        bsclog::error("Failed to set uCANopen {} server ID to {}(0x{:X}): occupied ID.", name, node_id.get(), node_id.get());
        return SetupStatus::invalid_argument;
    }

    _unregister_rx_messages(server);
    server->_set_node_id(node_id);
    // erase outdated elements from [id; server] map
    // for (auto it = _rxid_to_server.begin(); it != _rxid_to_server.end();) {
    //     if (it->second->name() == name) {
    //         it = _rxid_to_server.erase(it);
    //     } else {
    //         ++it;
    //     }
    // }
    _register_rx_messages(server);
    bsclog::success("Set uCANopen {} server ID to {}(0x{:X}).", name, node_id.get(), node_id.get());
    return SetupStatus::success;
}


void Client::_run(std::future<void> signal_exit) {
    bsclog::info("Started aux uCANopen thread.");

    while (signal_exit.wait_for(_socket->good() ? std::chrono::milliseconds(0) : std::chrono::milliseconds(1))
            == std::future_status::timeout) {
        auto now = std::chrono::steady_clock::now();

        /* SYNC */
        if (_sync_msg.is_enabled) {
            if (now - _sync_msg.timepoint >= _sync_msg.period) {
                _socket->send(create_frame(Cob::sync, _node_id, {}));
                _sync_msg.timepoint = now;
            }
        }

        /* HEARTBEAT */
        if (now - _heartbeat_msg.timepoint >= _heartbeat_msg.period) {
            _socket->send(create_frame(Cob::heartbeat, _node_id, {static_cast<uint8_t>(_nmt_state)}));
            _heartbeat_msg.timepoint = now;
        }

        /* TPDO */
        if (_is_tpdo_enabled) {
            for (auto& [tpdo_type, message] : _tpdo_msgs) {
                if (!message.creator) { continue; }
                if (now - message.timepoint >= message.period) {
                    _socket->send(create_frame(to_cob(tpdo_type), _node_id, message.creator()));
                    message.timepoint = now;
                }
            }
        }

        /* SEND TO SERVERS */
        for (auto& server : _servers) {
            server->_send();
        }

        /* RECV */
        can_frame frame;
        can::Status recv_sts = _socket->recv(frame);
        while (recv_sts == can::Status::ok) {
            //(void) std::async(&Client::_on_frame_received, this, frame);
            _on_frame_received(frame);
            recv_sts = _socket->recv(frame);
        }
    }

    bsclog::info("Stopped aux uCANopen thread.");
}


void Client::_on_frame_received(const can_frame& frame) {
    auto server_iter = _rxid_to_server.find(frame.can_id);
    if (server_iter != _rxid_to_server.end()) {
        server_iter->second->_handle_frame(frame);
    }
}


void Client::_register_rx_messages(std::shared_ptr<Server> server) {
    canid_t tpdo1 = calculate_cob_id(Cob::tpdo1, server->node_id());
    canid_t tpdo2 = calculate_cob_id(Cob::tpdo2, server->node_id());
    canid_t tpdo3 = calculate_cob_id(Cob::tpdo3, server->node_id());
    canid_t tpdo4 = calculate_cob_id(Cob::tpdo4, server->node_id());
    canid_t tsdo = calculate_cob_id(Cob::tsdo, server->node_id());
    canid_t heartbeat = calculate_cob_id(Cob::heartbeat, server->node_id());

    _rxid_to_server.insert({tpdo1, server});
    _rxid_to_server.insert({tpdo2, server});
    _rxid_to_server.insert({tpdo3, server});
    _rxid_to_server.insert({tpdo4, server});
    _rxid_to_server.insert({tsdo, server});
    _rxid_to_server.insert({heartbeat, server});
}


void Client::_unregister_rx_messages(std::shared_ptr<Server> server) {
    for (auto iter = _rxid_to_server.begin(); iter != _rxid_to_server.end();) {
        if (iter->second == server) {
            iter = _rxid_to_server.erase(iter);
        } else {
            ++iter;
        }
    }
}


bool Client::_is_free(NodeId node_id) const {
    if (node_id == _node_id) {
        return false;
    }

    return std::none_of(_servers.begin(), _servers.end(),
            [node_id](const auto& server){ return server->node_id() == node_id; });
}


} // namespace ucanopen
