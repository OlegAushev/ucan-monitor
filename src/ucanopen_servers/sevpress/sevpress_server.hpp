#pragma once

#include "sevpress_def.hpp"
#include <algorithm>
#include <atomic>
#include <bitset>
#include <bsclog/bsclog.h>
#include <ucanopen/server/server.h>

namespace sevpress {

extern ucanopen::ObjectDictionary const object_dictionary;
extern ucanopen::ObjectDictionaryConfig const object_dictionary_config;

class Server : public ucanopen::Server, public ucanopen::SdoSubscriber {
public:
  Server(std::shared_ptr<can::Socket> socket,
         ucanopen::NodeId node_id,
         std::string const& name);
protected:
  virtual ucanopen::FrameHandlingStatus
  handle_sdo(ucanopen::ODEntryIter entry,
             ucanopen::SdoType sdo_type,
             ucanopen::ExpeditedSdoData data) override final;
private:
  struct {
    std::atomic<bool> emergency_stop{false};
    std::atomic<bool> power{false};
    std::atomic<bool> start{false};
    std::atomic<ControlMode> control_mode{ControlMode::torque};
    std::atomic<int16_t> torque_ref{0};
    std::atomic<int16_t> speed_ref{0};
  } _rpdo1;

  struct {
    std::atomic<ModelMode> model_mode{ModelMode::idle};
    std::atomic<int16_t> angle_ref{0};
    std::atomic<int16_t> current_ref{0};
    std::atomic<uint16_t> voltage_ref{0};
  } _rpdo2;

  struct {
    std::atomic<DriveState> drive_state{DriveState::init};
    std::atomic<bool> pwm_on{false};
    std::atomic<bool> critical{false};
    std::atomic<bool> error{false};
    std::atomic<bool> warning{false};
    std::atomic<ControlMode> control_mode{ControlMode::torque};
    std::atomic<ModelMode> model_mode{ModelMode::idle};
  } _tpdo1;

  struct {
    std::atomic<float> dc_voltage{0};
    std::atomic<float> stator_current{0};
  } _tpdo2;

  struct {
    std::atomic<int16_t> speed{0};
    std::atomic<int16_t> angle{0};
  } _tpdo3;

  std::array<std::bitset<sys::status::status_count>, sys::diag::level_count>
      _status{};
public:
  // RPDO
  void toggle_emergency(bool v) { _rpdo1.emergency_stop.store(v); }

  void toggle_power(bool v) { _rpdo1.power.store(v); }

  void toggle_start(bool v) { _rpdo1.start.store(v); }

  void set_control_mode(ControlMode v) { _rpdo1.control_mode.store(v); }

  void set_torque_ref(int16_t v) { _rpdo1.torque_ref.store(v); }

  void set_speed_ref(int16_t v) { _rpdo1.speed_ref.store(v); }

  void set_model_mode(ModelMode v) { _rpdo2.model_mode.store(v); }

  void set_angle_ref(int16_t v) { _rpdo2.angle_ref.store(v); }

  void set_current_ref(int16_t v) { _rpdo2.current_ref.store(v); }

  void set_voltage_ref(uint16_t v) { _rpdo2.voltage_ref.store(v); }

  // TPDO
  DriveState drive_state() const { return _tpdo1.drive_state.load(); }

  std::string_view drive_state_str() const {
    auto it = drive_state_names.find(drive_state());
    if (it == drive_state_names.end()) {
      return "н/д";
    } else {
      return it->second;
    }
  }

  std::string_view drive_state_str_upper() const {
    auto it = drive_state_names_upper.find(drive_state());
    if (it == drive_state_names_upper.end()) {
      return "Н/Д";
    } else {
      return it->second;
    }
  }

  bool is_pwm_on() const { return _tpdo1.pwm_on.load(); }

  bool has_error() const { return _tpdo1.error.load(); }

  bool has_any_warning() const { return _tpdo1.warning.load(); }

  ControlMode control_mode() const { return _tpdo1.control_mode.load(); }

  std::string_view control_mode_str() const {
    auto it = ctlmode_names.find(control_mode());
    if (it == ctlmode_names.end()) {
      return "н/д";
    } else {
      return it->second;
    }
  }

  ModelMode model_mode() const { return _tpdo1.model_mode.load(); }

  std::string_view model_mode_str() const {
    auto it = model_mode_names.find(model_mode());
    if (it == model_mode_names.end()) {
      return "н/д";
    } else {
      return it->second;
    }
  }

  float dc_voltage() const { return _tpdo2.dc_voltage.load(); }

  float stator_current() const { return _tpdo2.stator_current.load(); }

  int16_t speed() const { return _tpdo3.speed.load(); }

  int16_t angle() const { return _tpdo3.angle.load(); }

  bool has_critical() const { return _tpdo1.critical.load(); }

  auto const& status() const { return _status; }

  bool has_status(sys::diag::level lv) const {
    return _status[std::to_underlying(lv)].any();
  }
private:
  void _handle_tpdo1(ucanopen::can_payload const& payload);
  void _handle_tpdo2(ucanopen::can_payload const& payload);
  void _handle_tpdo3(ucanopen::can_payload const& payload);
  void _handle_tpdo4(ucanopen::can_payload const& payload);

  ucanopen::can_payload _create_rpdo1();
  ucanopen::can_payload _create_rpdo2();
};

} // namespace sevpress
