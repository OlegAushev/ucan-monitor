#include "sdo_sequence.h"


namespace ucanopen {


SdoSequence::SdoSequence(ServerSdoService& sdo_service)
        : SdoSubscriber(sdo_service)
        , _sdo_service(sdo_service) {}


bool SdoSequence::start(std::vector<SdoStep> steps, Next next) {
    std::lock_guard<std::mutex> lock(_mtx);
    if (_step_idx < _steps.size()) {
        return false;
    }

    _steps = std::move(steps);
    _next = std::move(next);
    _step_idx = 0;
    _awaiting = false;
    _attempts = 0;
    _unanswered = 0;
    _outcome = _steps.empty() ? Outcome::completed : Outcome::none;
    return true;
}


void SdoSequence::cancel() {
    std::lock_guard<std::mutex> lock(_mtx);
    if (_step_idx < _steps.size()) {
        _finish(Outcome::cancelled);
    }
}


bool SdoSequence::busy() const {
    std::lock_guard<std::mutex> lock(_mtx);
    return _step_idx < _steps.size();
}


std::pair<size_t, size_t> SdoSequence::progress() const {
    std::lock_guard<std::mutex> lock(_mtx);
    return {_step_idx, _steps.size()};
}


SdoSequence::Outcome SdoSequence::outcome() const {
    std::lock_guard<std::mutex> lock(_mtx);
    return _outcome;
}


std::vector<SdoStep> SdoSequence::steps() const {
    std::lock_guard<std::mutex> lock(_mtx);
    return _steps;
}


void SdoSequence::send() {
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

    _steps[_step_idx].status = SdoStep::Status::timed_out;
    if (++_unanswered == _max_unanswered) {
        // The server is gone: the rest would only time out one by one.
        _finish(Outcome::no_response);
        return;
    }
    _advance();
}


FrameHandlingStatus SdoSequence::handle_sdo(const SdoAnswer& answer) {
    std::lock_guard<std::mutex> lock(_mtx);
    if (!_awaiting || answer.key != _steps[_step_idx].key) {
        return FrameHandlingStatus::irrelevant_frame;
    }

    auto& step = _steps[_step_idx];
    auto const expected = (step.kind == SdoStep::Kind::read) ? SdoType::response_to_read : SdoType::response_to_write;
    if (answer.type == SdoType::abort) {
        step.status = SdoStep::Status::refused;
        step.abort_code = static_cast<SdoAbortCode>(answer.data.u32());
    } else if (answer.type == expected) {
        step.status = SdoStep::Status::done;
        if (step.kind == SdoStep::Kind::read) {
            step.value = answer.data;
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


void SdoSequence::_request(std::chrono::time_point<std::chrono::steady_clock> now) {
    auto& step = _steps[_step_idx];
    constexpr bool quiet = true;

    ODAccessStatus status = ODAccessStatus::not_found;
    switch (step.kind) {
    case SdoStep::Kind::read:
        status = _sdo_service.read(step.key, quiet);
        break;
    case SdoStep::Kind::write:
        status = _sdo_service.write(step.key, step.write_value.value_or(ExpeditedSdoData{}), quiet);
        break;
    case SdoStep::Kind::command:
        status = _sdo_service.command(step.key, quiet);
        break;
    }

    if (status != ODAccessStatus::success) {
        // The description does not allow it, so nothing went out.
        step.status = SdoStep::Status::refused;
        if (status == ODAccessStatus::not_found) {
            step.abort_code = SdoAbortCode::object_not_found;
        } else if (step.kind == SdoStep::Kind::read) {
            step.abort_code = SdoAbortCode::read_access_wo;
        } else {
            step.abort_code = SdoAbortCode::write_access_ro;
        }
        _advance();
        return;
    }

    _awaiting = true;
    _request_timepoint = now;
    ++_attempts;
}


void SdoSequence::_advance() {
    _awaiting = false;
    _attempts = 0;
    if (_next) {
        // a copy: the steps may grow, and move, under the reference
        auto const finished = _steps[_step_idx];
        auto more = _next(finished);
        _steps.insert(_steps.end(), more.begin(), more.end());
    }
    if (++_step_idx == _steps.size()) {
        _outcome = Outcome::completed;
    }
}


void SdoSequence::_finish(Outcome outcome) {
    for (auto& step : _steps) {
        if (step.status == SdoStep::Status::pending) {
            step.status = SdoStep::Status::cancelled;
        }
    }
    _step_idx = _steps.size();
    _awaiting = false;
    _attempts = 0;
    _outcome = outcome;
}


} // namespace ucanopen
