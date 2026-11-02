#pragma once

#include <atomic>
#include <chrono>
#include <functional>
#include <ucanopen/od/description.h>
#include <ucanopen/server/impl/impl_server.h>
#include <ucanopen/server/services/heartbeat/server_heartbeat_service.h>
#include <ucanopen/server/services/info/server_info_service.h>
#include <ucanopen/server/services/log/server_log_service.h>
#include <ucanopen/server/services/rpdo/server_rpdo_service.h>
#include <ucanopen/server/services/sdo/server_sdo_service.h>
#include <ucanopen/server/services/status/server_status_service.h>
#include <ucanopen/server/services/tpdo/server_tpdo_service.h>
#include <ucanopen/server/services/watch/server_watch_service.h>
#include <ucanopen/server/utils/sdo_sequence.h>

namespace ucanopen {

// A device on the bus, read and driven by the description of its dictionary.
class Server : public impl::Server {
    friend class Client;
public:
    ServerHeartbeatService heartbeat_service;
    ServerTpdoService tpdo_service;
    ServerRpdoService rpdo_service;
    ServerSdoService sdo_service;
    ServerWatchService watch_service;
    SdoSequence config_service; // the bulk transfers of the settings
    ServerInfoService info_service;
    ServerLogService log_service;
    ServerStatusService status_service;
public:
    Server(std::shared_ptr<can::Socket> socket,
           NodeId node_id,
           const std::string& name,
           std::shared_ptr<const od::Description> description);
    virtual ~Server() = default;
private:
    void _send();
    void _handle_frame(const can_frame& frame);
    void _set_node_id(NodeId nodeId);

    std::vector<impl::FrameReceiver*> _rx_services;

public:
    ODAccessStatus read(std::string_view path) {
        return sdo_service.read(path);
    }

    ODAccessStatus write(std::string_view path, ExpeditedSdoData data) {
        return sdo_service.write(path, data);
    }

    ODAccessStatus command(std::string_view path) {
        return sdo_service.command(path);
    }
public:
    void send(can_frame const& frame) { _socket->send(frame); }
};

} // namespace ucanopen
