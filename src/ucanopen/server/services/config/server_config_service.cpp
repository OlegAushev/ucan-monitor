#include "server_config_service.h"


namespace ucanopen {


ServerConfigService::ServerConfigService(impl::Server& server, ServerSdoService& sdo_service)
        : SdoSubscriber(sdo_service)
        , _server(server)
        , _sdo_service(sdo_service) {
    const auto& entries = server.dictionary().entries;
    for (auto entry = entries.begin(); entry != entries.end(); ++entry) {
        // create conf entries list
        const auto& object = entry->second;
        if (object.category == server.dictionary().config.config_category) {
            _objects[object.subcategory].push_back(&object);
            _entries.push_back(entry);
        }
    }
}


bool ServerConfigService::start(std::vector<ConfigStep> steps) {
    std::lock_guard<std::mutex> lock(_mtx);
    if (_step_idx < _steps.size()) {
        return false;
    }

    _steps = std::move(steps);
    _step_idx = 0;
    _awaiting = false;
    _attempts = 0;
    _unanswered = 0;
    _outcome = _steps.empty() ? Outcome::completed : Outcome::none;
    return true;
}


void ServerConfigService::cancel() {
    std::lock_guard<std::mutex> lock(_mtx);
    if (_step_idx < _steps.size()) {
        _finish(Outcome::cancelled);
    }
}


bool ServerConfigService::busy() const {
    std::lock_guard<std::mutex> lock(_mtx);
    return _step_idx < _steps.size();
}


std::pair<size_t, size_t> ServerConfigService::progress() const {
    std::lock_guard<std::mutex> lock(_mtx);
    return {_step_idx, _steps.size()};
}


ServerConfigService::Outcome ServerConfigService::outcome() const {
    std::lock_guard<std::mutex> lock(_mtx);
    return _outcome;
}


std::vector<ConfigStep> ServerConfigService::steps() const {
    std::lock_guard<std::mutex> lock(_mtx);
    return _steps;
}


void ServerConfigService::send() {
    std::lock_guard<std::mutex> lock(_mtx);
    if (_step_idx >= _steps.size()) {
        return;
    }

    auto now = std::chrono::steady_clock::now();
    if (!_awaiting) {
        _request(now);
        return;
    }

    if (now - _request_timepoint < _timeout) {
        return;
    }

    if (_attempts < _max_attempts) {
        _request(now);
        return;
    }

    _steps[_step_idx].status = ConfigStep::Status::timed_out;
    if (++_unanswered == _max_unanswered) {
        // The server is gone: the rest would only time out one by one.
        _finish(Outcome::no_response);
        return;
    }
    _advance();
}


FrameHandlingStatus ServerConfigService::handle_sdo(ODEntryIter entry, SdoType sdo_type, ExpeditedSdoData sdo_data) {
    std::lock_guard<std::mutex> lock(_mtx);
    if (!_awaiting || entry != _steps[_step_idx].entry) {
        return FrameHandlingStatus::irrelevant_frame;
    }

    auto& step = _steps[_step_idx];
    auto answer = step.write_value.has_value() ? SdoType::response_to_write : SdoType::response_to_read;
    if (sdo_type == SdoType::abort) {
        step.status = ConfigStep::Status::refused;
        step.abort_code = static_cast<SdoAbortCode>(sdo_data.u32());
    } else if (sdo_type == answer) {
        step.status = ConfigStep::Status::done;
        if (!step.write_value.has_value()) {
            step.value = sdo_data;
        }
    } else {
        // An answer to someone else's request for the same object, e.g. the
        // echo of a write while this step reads it back.
        return FrameHandlingStatus::irrelevant_frame;
    }

    _unanswered = 0;
    _advance();
    return FrameHandlingStatus::success;
}


void ServerConfigService::_request(std::chrono::time_point<std::chrono::steady_clock> now) {
    auto& step = _steps[_step_idx];
    const auto& object = step.entry->second;
    constexpr bool quiet = true;

    ODAccessStatus status;
    if (step.write_value.has_value()) {
        status = _sdo_service.write(object.category, object.subcategory, object.name, *step.write_value, quiet);
    } else {
        status = _sdo_service.read(object.category, object.subcategory, object.name, quiet);
    }

    if (status != ODAccessStatus::success) {
        // The dictionary does not allow it, so nothing went out.
        step.status = ConfigStep::Status::refused;
        if (status == ODAccessStatus::not_found) {
            step.abort_code = SdoAbortCode::object_not_found;
        } else if (step.write_value.has_value()) {
            step.abort_code = SdoAbortCode::write_access_ro;
        } else {
            step.abort_code = SdoAbortCode::read_access_wo;
        }
        _advance();
        return;
    }

    _awaiting = true;
    _request_timepoint = now;
    ++_attempts;
}


void ServerConfigService::_advance() {
    _awaiting = false;
    _attempts = 0;
    if (++_step_idx == _steps.size()) {
        _outcome = Outcome::completed;
    }
}


void ServerConfigService::_finish(Outcome outcome) {
    for (auto& step : _steps) {
        if (step.status == ConfigStep::Status::pending) {
            step.status = ConfigStep::Status::cancelled;
        }
    }
    _step_idx = _steps.size();
    _awaiting = false;
    _attempts = 0;
    _outcome = outcome;
}


} // namespace ucanopen
