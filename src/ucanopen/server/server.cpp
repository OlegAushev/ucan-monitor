#include "server.h"


namespace ucanopen {

Server::Server(std::shared_ptr<can::Socket> socket, NodeId node_id, const std::string& name, const ObjectDictionary& dictionary)
        : impl::Server(socket, node_id, name, dictionary)
        , heartbeat_service(*this, std::chrono::milliseconds(2000))
        , tpdo_service(*this)
        , rpdo_service(*this)
        , sdo_service(*this)
        , watch_service(*this, sdo_service)
        , config_service(*this)
        , log_service(*this, sdo_service, tpdo_service) {
    _rx_services.push_back(&sdo_service);
    _rx_services.push_back(&tpdo_service);
    _rx_services.push_back(&heartbeat_service);
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
    utils::StringReader reader(*this, sdo_service, category, subcategory, name);
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
    utils::ScalarReader reader(*this, sdo_service, category, subcategory, name);
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
    utils::ExpeditedSdoDataReader reader(*this, sdo_service, category, subcategory, name);
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
    utils::Executor executor(*this, sdo_service, category, subcategory, name);
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
    utils::ExpeditedSdoDataWriter writer(*this, sdo_service, category, subcategory, name, sdo_data);
    std::future<std::optional<ExpeditedSdoData>> future_result
                    = std::async(&utils::ExpeditedSdoDataWriter::get, &writer, signal_terminate.get_future());
    if (future_result.wait_for(timeout) != std::future_status::ready) {
        signal_terminate.set_value();
    }
    return future_result.get();   
}


} // namespace ucanopen

