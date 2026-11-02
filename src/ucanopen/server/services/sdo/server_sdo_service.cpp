#include "server_sdo_service.h"

#include <ucanopen/od/value.h>

#include <format>


namespace ucanopen {


namespace {


std::string key_text(od::Key key) {
    return std::format("{:04X}h:{:02X}", key.index, key.subindex);
}


// What the log calls an object: its path, or its key if it has none.
std::string name_of(od::Key key, const std::optional<od::Item>& item) {
    if (!item.has_value() || item->path.empty()) {
        return key_text(key);
    }
    return item->path;
}


bool readable(const od::Item& item) {
    return item.available && item.access != od::Access::wo;
}


bool writable(const od::Item& item) {
    return item.available && (item.access == od::Access::rw || item.access == od::Access::wo);
}


} // namespace


ServerSdoService::ServerSdoService(impl::Server& server)
        : _server(server) {
    _id = calculate_cob_id(Cob::tsdo, _server.node_id());
}


void ServerSdoService::update_node_id() {
    _id = calculate_cob_id(Cob::tsdo, _server.node_id());
}


FrameHandlingStatus ServerSdoService::handle_frame(const can_frame& frame) {
    if (frame.can_id != _id) { return FrameHandlingStatus::id_mismatch; }
    // emblib's server answers with 8 bytes, as it takes them
    if (frame.can_dlc < 8) { return FrameHandlingStatus::invalid_format; }

    switch (get_cs_code(frame)) {
    case sdo_cs_codes::server_init_read:
        return _handle(frame, SdoType::response_to_read);
    case sdo_cs_codes::server_init_write:
        return _handle(frame, SdoType::response_to_write);
    case sdo_cs_codes::abort:
        return _handle(frame, SdoType::abort);
    default:
        return FrameHandlingStatus::invalid_format;
    }
}


FrameHandlingStatus ServerSdoService::_handle(const can_frame& frame, SdoType type) {
    ExpeditedSdo sdo(frame.data);
    SdoAnswer answer{{static_cast<uint16_t>(sdo.index), static_cast<uint8_t>(sdo.subindex)}, std::nullopt, type, sdo.data};
    answer.item = _resolve(answer.key);
    bool const quiet = _take_quiet_request(answer.key);

    auto const name = name_of(answer.key, answer.item);
    switch (type) {
    case SdoType::response_to_read:
        if (!quiet && answer.item.has_value() && answer.item->cls != od::Class::watch) {
            bsclog::success("{}: {} = {}.", _server.name(), name, od::format_value(*answer.item, answer.data));
        }
        break;
    case SdoType::response_to_write:
        if (answer.item.has_value() && answer.item->command) {
            bsclog::success("{}: выполнено {}.", _server.name(), name);
        } else if (!quiet) {
            bsclog::success("{}: записано {}.", _server.name(), name);
        }
        break;
    case SdoType::abort: {
        // a refusal is worth a line even when asked quietly
        AbortSdo abort(frame.data);
        auto const message = sdo_abort_messages.find(static_cast<SdoAbortCode>(abort.error_code));
        bsclog::error("{}: отказ SDO {}: {} (0x{:08X}).",
                      _server.name(),
                      name,
                      (message == sdo_abort_messages.end()) ? std::string("reason unknown") : message->second,
                      abort.error_code);
        answer.data = ExpeditedSdoData(abort.error_code);
        break;
    }
    }

    for (auto& subscriber : _subscriber_list) {
        subscriber->handle_sdo(answer);
    }
    return FrameHandlingStatus::success;
}


ODAccessStatus ServerSdoService::read(od::Key key, bool quiet) {
    auto item = _resolve(key);
    if (!item.has_value()) {
        bsclog::error("{}: не прочитать {}: описание словаря его не называет.", _server.name(), key_text(key));
        return ODAccessStatus::not_found;
    }
    if (!readable(*item)) {
        bsclog::error("{}: {} не читается.", _server.name(), name_of(key, item));
        return ODAccessStatus::access_denied;
    }

    if (!quiet && item->cls != od::Class::watch) {
        bsclog::info("{}: чтение {}...", _server.name(), name_of(key, item));
    }
    _mark_request(key, quiet);
    _send(*item, sdo_cs_codes::client_init_read, std::nullopt);
    return ODAccessStatus::success;
}


ODAccessStatus ServerSdoService::write(od::Key key, ExpeditedSdoData data, bool quiet) {
    auto item = _resolve(key);
    if (!item.has_value()) {
        bsclog::error("{}: не записать {}: описание словаря его не называет.", _server.name(), key_text(key));
        return ODAccessStatus::not_found;
    }
    if (!writable(*item)) {
        bsclog::error("{}: {} не записывается.", _server.name(), name_of(key, item));
        return ODAccessStatus::access_denied;
    }

    if (!quiet) {
        bsclog::info("{}: запись {} = {}...", _server.name(), name_of(key, item), od::format_value(*item, data));
    }
    _mark_request(key, quiet);
    _send(*item, sdo_cs_codes::client_init_write, data);
    return ODAccessStatus::success;
}


ODAccessStatus ServerSdoService::command(od::Key key, bool quiet) {
    auto item = _resolve(key);
    if (!item.has_value()) {
        bsclog::error("{}: не выполнить {}: описание словаря его не называет.", _server.name(), key_text(key));
        return ODAccessStatus::not_found;
    }
    if (!item->command || !writable(*item)) {
        bsclog::error("{}: {} — не команда.", _server.name(), name_of(key, item));
        return ODAccessStatus::access_denied;
    }

    if (!quiet) {
        bsclog::info("{}: команда {}...", _server.name(), name_of(key, item));
    }
    _mark_request(key, quiet);
    _send(*item, sdo_cs_codes::client_init_write, ExpeditedSdoData(item->signature.value_or(0)));
    return ODAccessStatus::success;
}


ODAccessStatus ServerSdoService::read(std::string_view path, bool quiet) {
    auto key = _find(path);
    return key.has_value() ? read(*key, quiet) : ODAccessStatus::not_found;
}


ODAccessStatus ServerSdoService::write(std::string_view path, ExpeditedSdoData data, bool quiet) {
    auto key = _find(path);
    return key.has_value() ? write(*key, data, quiet) : ODAccessStatus::not_found;
}


ODAccessStatus ServerSdoService::command(std::string_view path, bool quiet) {
    auto key = _find(path);
    return key.has_value() ? command(*key, quiet) : ODAccessStatus::not_found;
}


std::optional<od::Item> ServerSdoService::_resolve(od::Key key) const {
    const auto& description = _server.description();
    return description ? description->resolve(key) : std::nullopt;
}


std::optional<od::Key> ServerSdoService::_find(std::string_view path) const {
    const auto& description = _server.description();
    auto key = description ? description->find(path) : std::nullopt;
    if (!key.has_value()) {
        bsclog::error("{}: описание словаря не называет {}.", _server.name(), path);
    }
    return key;
}


void ServerSdoService::_send(const od::Item& item, uint32_t cs, std::optional<ExpeditedSdoData> data) {
    ExpeditedSdo message{};
    message.cs = cs;
    message.index = item.key.index;
    message.subindex = item.key.subindex;
    if (data.has_value()) {
        // emblib's server takes a write whose size is that of the object
        message.expedited_transfer = 1;
        message.data_size_indicated = 1;
        message.data_empty_bytes = static_cast<uint32_t>(4 - od::size_of(item.datatype)) & 0x3;
        message.data = *data;
    }
    _server._socket->send(create_frame(Cob::rsdo, _server.node_id(), message.to_payload()));
}


void ServerSdoService::_mark_request(od::Key key, bool quiet) {
    std::lock_guard<std::mutex> lock(_quiet_mtx);
    if (quiet) {
        _quiet_requests.insert(key);
    } else {
        _quiet_requests.erase(key);
    }
}


bool ServerSdoService::_take_quiet_request(od::Key key) {
    std::lock_guard<std::mutex> lock(_quiet_mtx);
    return _quiet_requests.erase(key) > 0;
}


} // namespace ucanopen
