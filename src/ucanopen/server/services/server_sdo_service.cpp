#include "server_sdo_service.h"


namespace ucanopen {

ServerSdoService::ServerSdoService(impl::Server& server)
        : _server(server) {
    _id = calculate_cob_id(CobType::tsdo, _server.node_id());
}


void ServerSdoService::update_node_id() {
    _id = calculate_cob_id(CobType::tsdo, _server.node_id());
}


FrameHandlingStatus ServerSdoService::handle_frame(const can_frame& frame) {
    if (frame.can_id != _id) { return FrameHandlingStatus::id_mismatch; }
    
    uint32_t cs_code = get_cs_code(frame);

    switch (cs_code) {
    case sdo_cs_codes::server_init_read:
        return _handle_read_expedited(frame);
    case sdo_cs_codes::server_init_write:
        return _handle_write_expedited(frame);	
    case sdo_cs_codes::abort:
        return _handle_abort(frame);
    default:
        return FrameHandlingStatus::invalid_format;
    }
}


FrameHandlingStatus ServerSdoService::_handle_read_expedited(const can_frame& frame) {
    ExpeditedSdo sdo(frame.data);
    auto entry = _server.dictionary().entries.find({sdo.index, sdo.subindex});
    if (entry == _server.dictionary().entries.end()) {
        return FrameHandlingStatus::object_not_found;
    }

    const auto& [key, object] = *entry;

    SdoType sdo_type;
    if (object.type == ODObjectType::OD_EXEC) {
        sdo_type = SdoType::response_to_exec;
        Log() << "Executed {" << _server.name() << "::"
              << object.category << "::" << object.subcategory << "::" << object.name << "}.\n" << LogPrefix::ok;
    } else {
        sdo_type = SdoType::response_to_read;
        if (object.category != _server.dictionary().config.watch_category) {
            Log() << "{" << _server.name() << "::"
                  << object.category << "::" << object.subcategory << "::" << object.name
                  << "} = " << sdo.data.to_string(object.type) << '\n' << LogPrefix::ok;
        }
    }

    for (auto& subscriber : _subscriber_list) {
        subscriber->handle_sdo(entry, sdo_type, sdo.data);
    }
    
    return FrameHandlingStatus::success;
}


FrameHandlingStatus ServerSdoService::_handle_write_expedited(const can_frame& frame) {
    ExpeditedSdo sdo(frame.data);
    auto entry = _server.dictionary().entries.find({sdo.index, sdo.subindex});
    if (entry == _server.dictionary().entries.end()) {
        return FrameHandlingStatus::object_not_found;
    }

    const auto& [key, object] = *entry;

    SdoType sdo_type;
    if (object.type == ODObjectType::OD_EXEC) {
        sdo_type = SdoType::response_to_exec;
        Log() << "Executed {" << _server.name() << "::"
              << object.category << "::" << object.subcategory << "::" << object.name << "}.\n" << LogPrefix::ok;
    } else {
        sdo_type = SdoType::response_to_write;
        Log() << "Updated {" << _server.name() << "::"
              << object.category << "::" << object.subcategory << "::" << object.name << "}.\n" << LogPrefix::ok;
    }

    for (auto& subscriber : _subscriber_list) {
        subscriber->handle_sdo(entry, sdo_type, sdo.data);
    }

    return FrameHandlingStatus::success;
}


FrameHandlingStatus ServerSdoService::_handle_abort(const can_frame& frame) {
    AbortSdo abort_sdo(frame.data);

    auto entry = _server.dictionary().entries.find({abort_sdo.index, abort_sdo.subindex});
    if (entry == _server.dictionary().entries.end()) {
        return FrameHandlingStatus::object_not_found;
    }

    const auto& [key, object] = *entry;

    std::string error_msg;
    if (sdo_abort_messages.contains(static_cast<SdoAbortCode>(abort_sdo.error_code))) {
        error_msg = sdo_abort_messages.at(static_cast<SdoAbortCode>(abort_sdo.error_code));
    } else {
        error_msg = "reason unknown";
    }
    Log() << "Aborted SDO {" << _server.name() << "::"
          << object.category << "::" << object.subcategory << "::" << object.name
          << "}: " << error_msg
          << " (error code: 0x" << std::hex << abort_sdo.error_code << std::dec << ").\n" << LogPrefix::failed;

    return FrameHandlingStatus::success;
}

} // namespace ucanopen

