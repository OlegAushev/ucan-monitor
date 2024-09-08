#include "server_sdo_service.h"


namespace ucanopen {

ServerSdoService::ServerSdoService(impl::Server& server)
        : _server(server) {
    _id = calculate_cob_id(Cob::tsdo, _server.node_id());
}


void ServerSdoService::update_node_id() {
    _id = calculate_cob_id(Cob::tsdo, _server.node_id());
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
    auto entry = _server.dictionary().entries.find({static_cast<uint16_t>(sdo.index), static_cast<uint8_t>(sdo.subindex)});
    if (entry == _server.dictionary().entries.end()) {
        return FrameHandlingStatus::object_not_found;
    }

    const auto& [key, object] = *entry;

    SdoType sdo_type;
    if (object.data_type == ODObjectDataType::OD_EXEC) {
        sdo_type = SdoType::response_to_exec;
        bsclog::success("Executed {}::{}::{}::{}.", _server.name(), object.category, object.subcategory, object.name);
    } else {
        sdo_type = SdoType::response_to_read;
        if (object.category != _server.dictionary().config.watch_category) {
            bsclog::success("{}::{}::{}::{} = {}.",
                            _server.name(), object.category, object.subcategory, object.name, sdo.data.to_string(object.data_type));
        }
    }

    for (auto& subscriber : _subscriber_list) {
        subscriber->handle_sdo(entry, sdo_type, sdo.data);
    }
    
    return FrameHandlingStatus::success;
}


FrameHandlingStatus ServerSdoService::_handle_write_expedited(const can_frame& frame) {
    ExpeditedSdo sdo(frame.data);
    auto entry = _server.dictionary().entries.find({static_cast<uint16_t>(sdo.index), static_cast<uint8_t>(sdo.subindex)});
    if (entry == _server.dictionary().entries.end()) {
        return FrameHandlingStatus::object_not_found;
    }

    const auto& [key, object] = *entry;

    SdoType sdo_type;
    if (object.data_type == ODObjectDataType::OD_EXEC) {
        sdo_type = SdoType::response_to_exec;
        bsclog::success("Executed {}::{}::{}::{}.", _server.name(), object.category, object.subcategory, object.name);
    } else {
        sdo_type = SdoType::response_to_write;
        bsclog::success("Updated {}::{}::{}::{}.", _server.name(), object.category, object.subcategory, object.name);
    }

    for (auto& subscriber : _subscriber_list) {
        subscriber->handle_sdo(entry, sdo_type, sdo.data);
    }

    return FrameHandlingStatus::success;
}


FrameHandlingStatus ServerSdoService::_handle_abort(const can_frame& frame) {
    AbortSdo abort_sdo(frame.data);

    auto entry = _server.dictionary().entries.find({static_cast<uint16_t>(abort_sdo.index), static_cast<uint8_t>(abort_sdo.subindex)});
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
    bsclog::error("Aborted SDO {}::{}::{}::{}: {} (error code: 0x{:X})",
                    _server.name(), object.category, object.subcategory, object.name,
                    error_msg, abort_sdo.error_code);

    ExpeditedSdo sdo(frame.data);
    auto sdo_type = SdoType::abort;
    for (auto& subscriber : _subscriber_list) {
        subscriber->handle_sdo(entry, sdo_type, sdo.data);
    }

    return FrameHandlingStatus::success;
}


void ServerSdoService::restore_default_parameter(std::string_view category, std::string_view subcategory, std::string_view name) {
    auto od_entry = _server.find_od_entry(category, subcategory, name);
    if (od_entry == _server.dictionary().entries.end()) {
        bsclog::error("Cannot restore {}::{}::{}::{}: no such OD entry.", _server.name(), category, subcategory, name);
        return;
    }

    ODObjectKey key{od_entry->first.index, od_entry->first.subindex};
    uint32_t key_u32 = 0;
    memcpy(&key_u32, &key, sizeof(key));
    ExpeditedSdoData data(key_u32);
    write("ctl", "sys", "restore_default_parameter", data);
}


ODAccessStatus ServerSdoService::read(std::string_view category, std::string_view subcategory, std::string_view name) {
    ODEntryIter entry;
    auto status = _server.find_od_entry_to_read(category, subcategory, name, entry);
    if (status != ODAccessStatus::success) {
        return status;
    }

    const auto& [key, object] = *entry;

    ExpeditedSdo message{};
    message.cs = sdo_cs_codes::client_init_read;
    message.index = key.index;
    message.subindex = key.subindex;

    if (object.category != _server._dictionary.config.watch_category) {
        bsclog::info("Sending request to read {}::{}::{}::{}...", _server.name(), object.category, object.subcategory, object.name);
    }
    _server._socket->send(create_frame(Cob::rsdo, _server.node_id(), message.to_payload()));   
    return ODAccessStatus::success;
}


ODAccessStatus ServerSdoService::write(std::string_view category, std::string_view subcategory, std::string_view name,
                                   ExpeditedSdoData sdo_data) {
    ODEntryIter entry;
    auto status = _server.find_od_entry_to_write(category, subcategory, name, entry);
    if (status != ODAccessStatus::success) {
        return status;
    }

    const auto& [key, object] = *entry;

    ExpeditedSdo message{};
    message.cs = sdo_cs_codes::client_init_write;
    message.expedited_transfer = 1;
    message.data_size_indicated = 1;
    message.data_empty_bytes = 8 - od_object_data_type_sizes[object.data_type];
    message.index = key.index;
    message.subindex = key.subindex;
    message.data = sdo_data;

    bsclog::info("Sending request to write {}::{}::{}::{} = {}...",
                    _server.name(), object.category, object.subcategory, object.name, sdo_data.to_string(object.data_type));
    _server._socket->send(create_frame(Cob::rsdo, _server.node_id(), message.to_payload()));
    return ODAccessStatus::success;
}


ODAccessStatus ServerSdoService::write(std::string_view category, std::string_view subcategory, std::string_view name,
                                   const std::string& value) {
    ODEntryIter entry;
    auto status = _server.find_od_entry_to_write(category, subcategory, name, entry);
    if (status != ODAccessStatus::success) {
        return status;
    }

    const auto& [key, object] = *entry;
    ExpeditedSdoData sdo_data;

    switch (object.data_type) {
    case OD_BOOL:
        if (value == "TRUE" || value == "true" || value == "ON" || value == "on" || value == "1")
            sdo_data = ExpeditedSdoData(true);
        else if (value == "FALSE" || value == "false" || value == "OFF" || value == "off" || value == "0")
            sdo_data = ExpeditedSdoData(true);
        else
            return ODAccessStatus::invalid_value;
        break;
    case OD_INT8:
        sdo_data = ExpeditedSdoData(int8_t(std::stoi(value)));
        break;
    case OD_INT16:
        sdo_data = ExpeditedSdoData(int16_t(std::stoi(value)));
        break;
    case OD_INT32:
        sdo_data = ExpeditedSdoData(int32_t(std::stoi(value)));
        break;
    case OD_UINT8:
        sdo_data = ExpeditedSdoData(uint8_t(std::stoul(value)));
        break;
    case OD_UINT16:
        sdo_data = ExpeditedSdoData(uint16_t(std::stoul(value)));
        break;
    case OD_UINT32:
        sdo_data = ExpeditedSdoData(uint32_t(std::stoul(value)));
        break;
    case OD_FLOAT32:
        sdo_data = ExpeditedSdoData(float(std::stof(value)));
        break;
    default:
        return ODAccessStatus::invalid_value;
    }

    ExpeditedSdo message{};
    message.cs = sdo_cs_codes::client_init_write;
    message.expedited_transfer = 1;
    message.data_size_indicated = 1;
    message.data_empty_bytes = 8 - od_object_data_type_sizes[object.data_type];
    message.index = key.index;
    message.subindex = key.subindex;
    message.data = sdo_data;

    bsclog::info("Sending request to write {}::{}::{}::{} = {}...",
                    _server.name(), object.category, object.subcategory, object.name, value);
    _server._socket->send(create_frame(Cob::rsdo, _server.node_id(), message.to_payload()));
    return ODAccessStatus::success;
}


ODAccessStatus ServerSdoService::exec(std::string_view category, std::string_view subcategory, std::string_view name) {
    ODEntryIter entry;
    auto status = _server.find_od_entry_to_exec(category, subcategory, name, entry);
    if (status != ODAccessStatus::success) {
        return status;
    }

    const auto& [key, object] = *entry;

    ExpeditedSdo message{};
    message.cs = sdo_cs_codes::client_init_write;
    message.expedited_transfer = 1;
    message.data_size_indicated = 1;
    message.data_empty_bytes = 0;
    message.index = key.index;
    message.subindex = key.subindex;

    bsclog::info("Sending request to execute {}::{}::{}::{}...",
                    _server.name(), object.category, object.subcategory, object.name);
    _server._socket->send(create_frame(Cob::rsdo, _server.node_id(), message.to_payload()));
    return ODAccessStatus::success;
}


} // namespace ucanopen

