#include "server_status_service.h"

#include <ucanopen/od/pdo_codec.h>

#include <algorithm>


namespace ucanopen {


ServerStatusService::ServerStatusService(impl::Server& server)
        : _server(server) {
    const auto& description = _server.description();
    if (!description) {
        return;
    }

    for (const auto& [index, record] : description->records()) {
        if (record.cls == od::Class::status) {
            _record = &record;
            break;
        }
    }
    for (const auto& pdo : description->pdos()) {
        if (pdo.direction == od::Direction::tx && pdo.mapping == od::Mapping::sam) {
            _pdo = &pdo;
            break;
        }
    }
    if (_record == nullptr) {
        return;
    }

    for (const auto& field : description->layouts()[_record->layout].fields) {
        if (field.subindex != 0x00) {
            _statuses.push_back({&field, std::nullopt});
        }
    }
    if (_pdo != nullptr) {
        _timeout = std::max(std::chrono::milliseconds(5 * _pdo->period_ms), std::chrono::milliseconds(500));
    }
}


bool ServerStatusService::good() const {
    std::lock_guard<std::mutex> lock(_mtx);
    return (std::chrono::steady_clock::now() - _timepoint) <= _timeout;
}


std::vector<ServerStatusService::Status> ServerStatusService::statuses() const {
    std::lock_guard<std::mutex> lock(_mtx);
    return _statuses;
}


bool ServerStatusService::active(std::string_view name) const {
    if (!good()) {
        return false;
    }
    std::lock_guard<std::mutex> lock(_mtx);
    auto status = std::find_if(_statuses.begin(), _statuses.end(), [name](const Status& s) {
        return s.field->name == name;
    });
    return status != _statuses.end() && status->severity.has_value() && *status->severity != 0;
}


void ServerStatusService::handle_frame(const can_payload& payload) {
    auto value = od::decode_sam(payload, _server.node_id().get());
    if (!value.has_value()) {
        return;
    }

    std::lock_guard<std::mutex> lock(_mtx);
    _timepoint = std::chrono::steady_clock::now();
    if (_record == nullptr || value->key.index != _record->index) {
        return;
    }
    auto status = std::find_if(_statuses.begin(), _statuses.end(), [&value](const Status& s) {
        return s.field->subindex == value->key.subindex;
    });
    if (status != _statuses.end()) {
        status->severity = value->data.u8();
    }
}


} // namespace ucanopen
