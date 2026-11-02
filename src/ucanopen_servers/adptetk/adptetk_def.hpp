#pragma once

#include <array>
#include <string_view>

// The states and modes of the drive that the control panel goes by. Each
// enumerator is named as its value in the table of the description that
// names it, and numbered as that table numbers it: the server checks both at
// startup.

namespace adptetk {

enum class DriveState {
  init,
  standby,
  powering_up,
  powering_down,
  ready,
  running,
  stopping,
  testing,
  calibrating,
  fault,
};

inline constexpr std::array<std::string_view, 10> drive_state_names = {
    "init",
    "standby",
    "powering_up",
    "powering_down",
    "ready",
    "running",
    "stopping",
    "testing",
    "calibrating",
    "fault",
};

enum class ControlMode {
  torque,
  speed,
  angle
};

inline constexpr std::array<std::string_view, 3> control_mode_names = {
    "torque",
    "speed",
    "angle",
};

enum class ModelMode {
  idle,
  closed_loop,
  semiclosed_loop,
  open_current_loop,
  open_voltage_loop,
  direct_pwm
};

inline constexpr std::array<std::string_view, 6> model_mode_names = {
    "idle",
    "closed_loop",
    "semiclosed_loop",
    "open_current_loop",
    "open_voltage_loop",
    "direct_pwm",
};

} // namespace adptetk
