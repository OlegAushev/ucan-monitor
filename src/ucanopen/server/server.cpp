#include "server.h"

#include <ucanopen/od/pdo_codec.h>


namespace ucanopen {

Server::Server(std::shared_ptr<can::Socket> socket, NodeId node_id, const std::string& name, const ObjectDictionary& dictionary,
               std::shared_ptr<const od::Description> description)
        : impl::Server(socket, node_id, name, dictionary, std::move(description))
        , heartbeat_service(*this, std::chrono::milliseconds(2000))
        , tpdo_service(*this)
        , rpdo_service(*this)
        , sdo_service(*this)
        , watch_service(*this, sdo_service)
        , config_service(*this, sdo_service)
        , log_service(*this, sdo_service, tpdo_service)
        , status_service(*this) {
    _rx_services.push_back(&sdo_service);
    _rx_services.push_back(&tpdo_service);
    _rx_services.push_back(&heartbeat_service);

    // A server whose description states its statuses takes their MPDO here,
    // whatever the device.
    if (status_service.available()) {
        auto const tpdo = od::tpdo_of(*status_service.pdo());
        if (tpdo.has_value()) {
            tpdo_service.register_tpdo(*tpdo, status_service.timeout(), [this](const can_payload& payload) {
                status_service.handle_frame(payload);
            });
        } else {
            bsclog::error("{}: статусы идут в PDO «{}», а ucanopen принимает только TPDO1–TPDO4 предопределённого "
                          "набора.", _name, status_service.pdo()->name);
        }
    }
}


void Server::_set_node_id(NodeId node_id) {
    if (!node_id.valid()) { return; }

    _node_id = node_id;

    heartbeat_service.update_node_id();
    tpdo_service.update_node_id();
    rpdo_service.update_node_id();
    sdo_service.update_node_id();
}


void Server::_send() {	
    rpdo_service.send();
    watch_service.send();
    config_service.send();
}


void Server::_handle_frame(const can_frame& frame) {
    for (auto service : _rx_services) {
        auto status = service->handle_frame(frame);
        if (status == FrameHandlingStatus::success) {
            break;
        }
    }	
}


std::optional<std::string> Server::read_string(std::string_view category,
                                               std::string_view subcategory,
                                               std::string_view name,
                                               std::chrono::milliseconds timeout) {
    std::promise<void> signal_terminate;
    utils::StringReader reader(sdo_service, category, subcategory, name);
    std::future<std::optional<std::string>> future_result
                    = std::async(&utils::StringReader::get, &reader, signal_terminate.get_future());
    if (future_result.wait_for(timeout) != std::future_status::ready) {
        signal_terminate.set_value();
    }
    return future_result.get();
}


std::optional<std::string> Server::read_scalar(std::string_view category,
                                               std::string_view subcategory,
                                               std::string_view name,
                                               std::chrono::milliseconds timeout) {
    std::promise<void> signal_terminate;
    utils::ScalarReader reader(sdo_service, category, subcategory, name);
    std::future<std::optional<std::string>> future_result
                    = std::async(&utils::ScalarReader::get, &reader, signal_terminate.get_future());
    if (future_result.wait_for(timeout) != std::future_status::ready) {
        signal_terminate.set_value();
    }
    return future_result.get();
}


std::optional<ExpeditedSdoData> Server::read_expdata(std::string_view category,
                                                     std::string_view subcategory,
                                                     std::string_view name,
                                                     std::chrono::milliseconds timeout) {
    std::promise<void> signal_terminate;
    utils::ExpeditedSdoDataReader reader(sdo_service, category, subcategory, name);
    std::future<std::optional<ExpeditedSdoData>> future_result
                    = std::async(&utils::ExpeditedSdoDataReader::get, &reader, signal_terminate.get_future());
    if (future_result.wait_for(timeout) != std::future_status::ready) {
        signal_terminate.set_value();
    }
    return future_result.get();
}


std::optional<ExpeditedSdoData> Server::exec_expdata(std::string_view category,
                                                     std::string_view subcategory,
                                                     std::string_view name,
                                                     std::chrono::milliseconds timeout) {
    std::promise<void> signal_terminate;
    utils::Executor executor(sdo_service, category, subcategory, name);
    std::future<std::optional<ExpeditedSdoData>> future_result
                    = std::async(&utils::Executor::get, &executor, signal_terminate.get_future());
    if (future_result.wait_for(timeout) != std::future_status::ready) {
        signal_terminate.set_value();
    }
    return future_result.get();
}


std::optional<ExpeditedSdoData> Server::write_expdata(std::string_view category,
                                                      std::string_view subcategory,
                                                      std::string_view name,
                                                      ExpeditedSdoData sdo_data,
                                                      std::chrono::milliseconds timeout) {
    std::promise<void> signal_terminate;
    utils::ExpeditedSdoDataWriter writer(sdo_service, category, subcategory, name, sdo_data);
    std::future<std::optional<ExpeditedSdoData>> future_result
                    = std::async(&utils::ExpeditedSdoDataWriter::get, &writer, signal_terminate.get_future());
    if (future_result.wait_for(timeout) != std::future_status::ready) {
        signal_terminate.set_value();
    }
    return future_result.get();   
}


} // namespace ucanopen

