#include <ucanopen_servers/adptbike/adptbike_def.hpp>
#include <ucanopen_servers/adptetk/adptetk_def.hpp>
#include <ucanopen_servers/hall_calibration.hpp>
#include <ucanopen_servers/svprs/svprs_def.hpp>

#include <cstdlib>
#include <iostream>

namespace {

template<class DriveState>
void check_calibration() {
    using namespace std::chrono_literals;
    using Calibration = inverter::HallCalibration;
    using Result = Calibration::Result;

    Calibration calibration;
    auto now = Calibration::Clock::now();
    unsigned node = 1;
    auto frame = [&](DriveState state) {
        now += 10ms;
        calibration.observe(state, node, 500ms, now);
    };
    auto expect = [&](Result expected, const char* scenario) {
        if (calibration.result(node, now) != expected) {
            std::cerr << scenario << '\n';
            std::exit(EXIT_FAILURE);
        }
    };

    frame(DriveState::ready);
    expect(Result::none, "ready without calibration");
    frame(DriveState::calibrating);
    frame(DriveState::ready);
    frame(DriveState::ready);
    expect(Result::completed, "calibration completed");
    calibration.dismiss(Result::completed);
    frame(DriveState::ready);
    expect(Result::none, "dismissed result");

    frame(DriveState::calibrating);
    frame(DriveState::fault);
    frame(DriveState::ready);
    expect(Result::fault, "fault and recovery between two draws");
    frame(DriveState::calibrating);
    expect(Result::none, "new attempt");
    frame(DriveState::powering_down);
    frame(DriveState::standby);
    expect(Result::interrupted, "power down during calibration");

    frame(DriveState::calibrating);
    frame(static_cast<DriveState>(12));
    frame(DriveState::ready);
    expect(Result::unknown, "unrecognized terminal state");
    frame(DriveState::calibrating);
    now += 501ms;
    frame(DriveState::ready);
    expect(Result::none, "reconnected after calibrating");

    frame(DriveState::calibrating);
    frame(DriveState::ready);
    now += 501ms;
    expect(Result::none, "telemetry lost after success");
    frame(DriveState::ready);
    expect(Result::none, "expired result after reconnection");

    frame(DriveState::calibrating);
    ++node;
    frame(DriveState::ready);
    expect(Result::none, "ready from a different node");
}

} // namespace

int main() {
    check_calibration<adptetk::DriveState>();
    check_calibration<adptbike::DriveState>();
    check_calibration<svprs::DriveState>();
}
