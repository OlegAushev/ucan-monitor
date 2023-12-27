#pragma once


#include "impl/impl_server.h"
#include "services/server_heartbeat_service.h"
#include "services/server_tpdo_service.h"
#include "services/server_rpdo_service.h"
#include "services/server_watch_service.h"
#include "services/server_config_service.h"
#include "services/server_sdo_service.h"
#include "services/server_log_service.h"
#include "utils/server_od_utils.h"
#include <atomic>
#include <functional>
#include <chrono>
#include <future>


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
    Server(std::shared_ptr<can::Socket> socket, NodeId node_id, const std::string& name, const ObjectDictionary& dictionary);
    virtual ~Server() = default;
private:	
    void _send();
    void _handle_frame(const can_frame& frame);
    void _set_node_id(NodeId nodeId);

    std::vector<impl::FrameReceiverInterface*> _rx_services;

public:
    std::optional<std::string> read_string(std::string_view category,
                                           std::string_view subcategory,
                                           std::string_view name,
                                           std::chrono::milliseconds timeout = std::chrono::milliseconds(1000));
    
    std::optional<std::string> read_scalar(std::string_view category,
                                           std::string_view subcategory,
                                           std::string_view name,
                                           std::chrono::milliseconds timeout = std::chrono::milliseconds(1000));
    
    std::optional<ExpeditedSdoData> read_expdata(std::string_view category,
                                                 std::string_view subcategory,
                                                 std::string_view name,
                                                 std::chrono::milliseconds timeout = std::chrono::milliseconds(1000));
    
    std::optional<ExpeditedSdoData> exec_expdata(std::string_view category,
                                                 std::string_view subcategory,
                                                 std::string_view name,
                                                 std::chrono::milliseconds timeout = std::chrono::milliseconds(1000));

    std::optional<ExpeditedSdoData> write_expdata(std::string_view category,
                                                  std::string_view subcategory,
                                                  std::string_view name,
                                                  ExpeditedSdoData sdo_data,
                                                  std::chrono::milliseconds timeout = std::chrono::milliseconds(1000));
};

} // namespace ucanopen

