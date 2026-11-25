#pragma once

#include <chrono>
#include <mutex>

namespace inverter {

class HallCalibration {
public:
    enum class Result { none, completed, fault, interrupted, unknown };
    using Clock = std::chrono::steady_clock;

    // Observe every drive_status frame, even while the panel is closed.
    template<class DriveState>
    void observe(DriveState state, unsigned node, std::chrono::milliseconds timeout,
                 Clock::time_point now = Clock::now()) {
        std::lock_guard lock(_mutex);
        if (node != _node || now > _valid_until || state == DriveState::calibrating) {
            _result = Result::none;
        } else if (_calibrating) {
            switch (state) {
            case DriveState::ready:         _result = Result::completed; break;
            case DriveState::fault:         _result = Result::fault; break;
            case DriveState::powering_down: _result = Result::interrupted; break;
            default:                        _result = Result::unknown; break;
            }
        } else if (state == DriveState::init ||
                   (_result == Result::completed && state != DriveState::ready)) {
            _result = Result::none;
        }
        _calibrating = state == DriveState::calibrating;
        _node = node;
        _valid_until = now + timeout;
    }

    Result result(unsigned node, Clock::time_point now = Clock::now()) const {
        std::lock_guard lock(_mutex);
        return node == _node && now <= _valid_until ? _result : Result::none;
    }

    void dismiss(Result shown) {
        std::lock_guard lock(_mutex);
        if (_result == shown) {
            _result = Result::none;
        }
    }

private:
    mutable std::mutex _mutex;
    unsigned _node{0};
    Clock::time_point _valid_until{};
    bool _calibrating{false};
    Result _result{Result::none};
};

} // namespace inverter
