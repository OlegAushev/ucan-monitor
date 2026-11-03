#pragma once

#include "adptetk_def.hpp"
#include <atomic>
#include <bsclog/bsclog.h>
#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <ucanopen/od/description.h>
#include <ucanopen/server/server.h>
#include <utility>

namespace adptetk {

// The ADPT-ETK inverter. Its PDOs are those its description states: the
// state of the drive, its electrical quantities and its motion come in
// drive_status, electrical_status and motion_status, the statuses in the
// MPDO that ucanopen::Server takes, and the commands to the drive and to its
// model go in drive_command and model_command, set in physical units.
class Server : public ucanopen::Server {
public:
  Server(std::shared_ptr<can::Socket> socket,
         ucanopen::NodeId node_id,
         std::string const& name,
         std::shared_ptr<ucanopen::od::Description const> description);
private:
  ucanopen::od::Pdo const* _drive_status_pdo{nullptr};
  ucanopen::od::Pdo const* _electrical_status_pdo{nullptr};
  ucanopen::od::Pdo const* _motion_status_pdo{nullptr};
  ucanopen::od::Pdo const* _drive_command_pdo{nullptr};
  ucanopen::od::Pdo const* _model_command_pdo{nullptr};
  std::optional<ucanopen::CobTpdo> _drive_status_tpdo;

  // The tables that name the states and the modes.
  ucanopen::od::ValueTable const* _drive_states{nullptr};
  ucanopen::od::ValueTable const* _control_modes{nullptr};
  ucanopen::od::ValueTable const* _model_modes{nullptr};

  struct {
    std::atomic<bool> emergency{false};
    std::atomic<bool> power{false};
    std::atomic<bool> start{false};
    std::atomic<ControlMode> control_mode{ControlMode::torque};
    std::atomic<float> torque_ref{0}; // pu
    std::atomic<float> speed_ref{0};  // rpm
    unsigned counter{0};
  } _drive_command;

  struct {
    std::atomic<ModelMode> model_mode{ModelMode::idle};
    std::atomic<float> angle_ref{0};   // °el
    std::atomic<float> current_ref{0}; // pu
    std::atomic<float> voltage_ref{0}; // pu
    unsigned counter{0};
  } _model_command;

  struct {
    std::atomic<int64_t> state{-1}; // as its table numbers it, -1 before a frame
    std::atomic<bool> pwm_on{false};
    std::atomic<bool> emergency{false};
    std::atomic<bool> critical{false};
    std::atomic<bool> error{false};
    std::atomic<bool> warning{false};
    std::atomic<int64_t> control_mode{-1};
    std::atomic<int64_t> model_mode{-1};
  } _drive_status;

  struct {
    std::atomic<float> dc_voltage{0};    // V
    std::atomic<float> phase_current{0}; // A
  } _electrical_status;

  struct {
    std::atomic<float> speed{0}; // rpm
    std::atomic<float> angle{0}; // °el
  } _motion_status;

  // What the RPDOs report once, while it lasts; the CAN thread alone builds
  // their frames.
  std::set<std::string> _unbuilt;
  std::set<std::string> _saturated;
public:
  // RPDO
  void toggle_emergency(bool v) { _drive_command.emergency.store(v); }

  void toggle_power(bool v) { _drive_command.power.store(v); }

  void toggle_start(bool v) { _drive_command.start.store(v); }

  void set_control_mode(ControlMode v) { _drive_command.control_mode.store(v); }

  void set_torque_ref(float pu) { _drive_command.torque_ref.store(pu); }

  void set_speed_ref(float rpm) { _drive_command.speed_ref.store(rpm); }

  void set_model_mode(ModelMode v) { _model_command.model_mode.store(v); }

  void set_angle_ref(float edeg) { _model_command.angle_ref.store(edeg); }

  void set_current_ref(float pu) { _model_command.current_ref.store(pu); }

  void set_voltage_ref(float pu) { _model_command.voltage_ref.store(pu); }

  // TPDO
  bool drive_status_good() const {
    return _drive_status_tpdo.has_value() &&
           tpdo_service.good(*_drive_status_tpdo);
  }

  std::optional<DriveState> drive_state() const {
    auto const state = _drive_status.state.load();
    if (state < 0 || state >= std::ssize(drive_state_names)) {
      return std::nullopt;
    }
    return static_cast<DriveState>(state);
  }

  std::string_view drive_state_str() const {
    return _label(_drive_states, _drive_status.state.load());
  }

  bool is_pwm_on() const { return _drive_status.pwm_on.load(); }

  bool has_error() const { return _drive_status.error.load(); }

  bool has_any_warning() const { return _drive_status.warning.load(); }

  bool has_emergency() const { return _drive_status.emergency.load(); }

  bool has_critical() const { return _drive_status.critical.load(); }

  std::string_view control_mode_str() const {
    return _label(_control_modes, _drive_status.control_mode.load());
  }

  std::string_view model_mode_str() const {
    return _label(_model_modes, _drive_status.model_mode.load());
  }

  // The label of a mode of the model as its table names it, for the panel.
  std::string const& model_mode_label(ModelMode mode) const;

  float dc_voltage() const { return _electrical_status.dc_voltage.load(); }

  float stator_current() const {
    return _electrical_status.phase_current.load();
  }

  float speed() const { return _motion_status.speed.load(); }

  float angle() const { return _motion_status.angle.load(); }
private:
  static std::string_view _label(ucanopen::od::ValueTable const* table,
                                 int64_t value);

  void _handle_drive_status(ucanopen::can_payload const& payload);
  void _handle_electrical_status(ucanopen::can_payload const& payload);
  void _handle_motion_status(ucanopen::can_payload const& payload);

  std::optional<ucanopen::can_payload> _create_drive_command();
  std::optional<ucanopen::can_payload> _create_model_command();
  std::optional<ucanopen::can_payload>
  _build(ucanopen::od::Pdo const& pdo,
         std::map<std::string, double, std::less<>> const& values,
         unsigned& counter);
};

} // namespace adptetk
