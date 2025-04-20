#pragma once

#include <atomic>
#include <chrono>
#include <functional>
#include <future>
#include <ucanopen/server/impl/impl_server.h>
#include <ucanopen/server/services/config/server_config_service.h>
#include <ucanopen/server/services/heartbeat/server_heartbeat_service.h>
#include <ucanopen/server/services/log/server_log_service.h>
#include <ucanopen/server/services/rpdo/server_rpdo_service.h>
#include <ucanopen/server/services/sdo/server_sdo_service.h>
#include <ucanopen/server/services/tpdo/server_tpdo_service.h>
#include <ucanopen/server/services/watch/server_watch_service.h>
#include <ucanopen/server/utils/server_od_utils.h>

namespace ucanopen {

class Server : public impl::Server {
    friend class Client;
public:
    ServerHeartbeatService heartbeat_service;
    ServerTpdoService tpdo_service;
    ServerRpdoService rpdo_service;
    ServerSdoService sdo_service;
    ServerWatchService watch_service;
    ServerConfigService config_service;
    ServerLogService log_service;
public:
    Server(std::shared_ptr<can::Socket> socket,
           NodeId node_id,
           const std::string& name,
           const ObjectDictionary& dictionary);
    virtual ~Server() = default;
private:
    void _send();
    void _handle_frame(const can_frame& frame);
    void _set_node_id(NodeId nodeId);

    std::vector<impl::FrameReceiver*> _rx_services;

public:
    std::optional<std::string>
    read_string(std::string_view category,
                std::string_view subcategory,
                std::string_view name,
                std::chrono::milliseconds timeout =
                        std::chrono::milliseconds(1000));

    std::optional<std::string>
    read_scalar(std::string_view category,
                std::string_view subcategory,
                std::string_view name,
                std::chrono::milliseconds timeout =
                        std::chrono::milliseconds(1000));

    std::optional<ExpeditedSdoData>
    read_expdata(std::string_view category,
                 std::string_view subcategory,
                 std::string_view name,
                 std::chrono::milliseconds timeout =
                         std::chrono::milliseconds(1000));

    std::optional<ExpeditedSdoData>
    exec_expdata(std::string_view category,
                 std::string_view subcategory,
                 std::string_view name,
                 std::chrono::milliseconds timeout =
                         std::chrono::milliseconds(1000));

    std::optional<ExpeditedSdoData>
    write_expdata(std::string_view category,
                  std::string_view subcategory,
                  std::string_view name,
                  ExpeditedSdoData sdo_data,
                  std::chrono::milliseconds timeout =
                          std::chrono::milliseconds(1000));

    ODAccessStatus read(std::string_view category,
                        std::string_view subcategory,
                        std::string_view name) {
        return sdo_service.read(category, subcategory, name);
    }

    ODAccessStatus write(std::string_view category,
                         std::string_view subcategory,
                         std::string_view name,
                         ExpeditedSdoData sdo_data) {
        return sdo_service.write(category, subcategory, name, sdo_data);
    }

    ODAccessStatus write(std::string_view category,
                         std::string_view subcategory,
                         std::string_view name,
                         const std::string& value) {
        return sdo_service.write(category, subcategory, name, value);
    }

    ODAccessStatus exec(std::string_view category,
                        std::string_view subcategory,
                        std::string_view name) {
        return sdo_service.exec(category, subcategory, name);
    }
public:
    void send(can_frame const& frame) { _socket->send(frame); }
};

} // namespace ucanopen
