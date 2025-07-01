#pragma once

#include "sevpress_def.hpp"
#include <algorithm>
#include <atomic>
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
    std::atomic<ControlMode> ctlmode{ControlMode::torque};
    std::atomic<float> ref_torque{0};
    std::atomic<int16_t> ref_speed{0};
  } _rpdo1;

  struct {
    std::atomic<int16_t> ref_angle{0};
    std::atomic<float> ref_current{0};
    std::atomic<float> ref_voltage{0};
    std::atomic<OperatingMode> opmode{OperatingMode::normal};
    std::atomic<ControlLoop> ctlloop{ControlLoop::closed};
  } _rpdo2;

  struct {
    std::atomic<DriveState> drive_state{DriveState::init};
    std::atomic<bool> pwm_on{false};
    std::atomic<bool> error{false};
    std::atomic<bool> warning{false};
    std::atomic<OperatingMode> opmode{OperatingMode::normal};
    std::atomic<ControlMode> ctlmode{ControlMode::torque};
    std::atomic<ControlLoop> ctlloop{ControlLoop::closed};

    std::atomic<std::string_view> calibration_state{"н/д"};
  } _tpdo1;

  struct {
    std::atomic<float> dc_voltage{0};
    std::atomic<float> stator_current{0};
  } _tpdo2;

  struct {
    std::atomic<int16_t> speed{0};
    std::atomic<int16_t> angle{0};
  } _tpdo3;

  std::array<std::atomic_uint32_t, syslog::domains.size()> _errors{};
  std::array<std::atomic_uint16_t, syslog::domains.size()> _warnings{};
public:
  // RPDO
  void toggle_emergency(bool v) { _rpdo1.emergency_stop.store(v); }

  void toggle_power(bool v) { _rpdo1.power.store(v); }

  void toggle_start(bool v) { _rpdo1.start.store(v); }

  void set_ctlmode(ControlMode v) { _rpdo1.ctlmode.store(v); }

  void set_ref_torque(float v) {
    _rpdo1.ref_torque.store(std::clamp(v, -1.0f, 1.0f));
  }

  void set_ref_speed(int16_t value) { _rpdo1.ref_speed.store(value); }

  void set_ref_angle(int16_t v) { _rpdo2.ref_angle.store(v); }

  void set_ref_current(float v) {
    _rpdo2.ref_current.store(std::clamp(v, -1.0f, 1.0f));
  }

  void set_ref_voltage(float v) {
    _rpdo2.ref_voltage.store(std::clamp(v, 0.0f, 1.0f));
  }

  void set_opmode(OperatingMode v) { _rpdo2.opmode.store(v); }

  void set_ctlloop(ControlLoop v) { _rpdo2.ctlloop.store(v); }

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

  OperatingMode opmode() const { return _tpdo1.opmode.load(); }

  std::string_view opmode_str() const {
    auto it = opmode_names.find(opmode());
    if (it == opmode_names.end()) {
      return "н/д";
    } else {
      return it->second;
    }
  }

  ControlMode ctlmode() const { return _tpdo1.ctlmode.load(); }

  std::string_view ctlmode_str() const {
    auto it = ctlmode_names.find(ctlmode());
    if (it == ctlmode_names.end()) {
      return "н/д";
    } else {
      return it->second;
    }
  }

  ControlLoop ctlloop() const { return _tpdo1.ctlloop.load(); }

  std::string_view ctlloop_str() const {
    auto it = ctlloop_names.find(ctlloop());
    if (it == ctlloop_names.end()) {
      return "н/д";
    } else {
      return it->second;
    }
  }

  float dc_voltage() const { return _tpdo2.dc_voltage.load(); }

  float stator_current() const { return _tpdo2.stator_current.load(); }

  int16_t speed() const { return _tpdo3.speed.load(); }

  int16_t angle() const { return _tpdo3.angle.load(); }

  bool has_any_error() const {
    for (auto& entry : _errors) {
      if (entry.load() != 0) {
        return true;
      }
    }
    return false;
  }

  auto const& errors() const { return _errors; }

  auto const& warnings() const { return _warnings; }
private:
  void _handle_tpdo1(ucanopen::can_payload const& payload);
  void _handle_tpdo2(ucanopen::can_payload const& payload);
  void _handle_tpdo3(ucanopen::can_payload const& payload);
  void _handle_tpdo4(ucanopen::can_payload const& payload);

  ucanopen::can_payload _create_rpdo1();
  ucanopen::can_payload _create_rpdo2();
};

} // namespace sevpress
