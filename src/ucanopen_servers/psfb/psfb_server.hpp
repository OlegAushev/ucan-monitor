#pragma once

#include "psfb_def.hpp"
#include <algorithm>
#include <atomic>
#include <bitset>
#include <bsclog/bsclog.h>
#include <ucanopen/server/server.h>

namespace psfb {

extern ucanopen::ObjectDictionary const object_dictionary;

class Server : public ucanopen::Server {
public:
  Server(std::shared_ptr<can::Socket> socket,
         ucanopen::NodeId node_id,
         std::string const& name);
private:
  struct {
    std::atomic<bool> emergency_stop{false};
    std::atomic<bool> power{false};
    std::atomic<bool> start{false};
    std::atomic<uint16_t> overlap1_ref{0};
    std::atomic<uint16_t> overlap2_ref{0};
  } _rpdo1;

  struct {
    std::atomic<ConverterState> converter_state{ConverterState::init};
    std::atomic<bool> pwm1_on{false};
    std::atomic<bool> pwm2_on{false};
    std::atomic<bool> emergency{false};
    std::atomic<bool> critical{false};
    std::atomic<bool> error{false};
    std::atomic<bool> warning{false};
    std::atomic<float> overlap1{0};
    std::atomic<float> overlap2{0};
  } _tpdo1;

  struct {
    std::atomic<float> hv_voltage{0};
    std::atomic<float> lv_voltage{0};
  } _tpdo2;

  struct {
    std::atomic<float> current1{0};
    std::atomic<float> current2{0};
  } _tpdo3;

  std::array<std::bitset<sys::status::status_count>, sys::diag::level_count>
      _status{};
public:
  // RPDO
  void toggle_emergency(bool v) { _rpdo1.emergency_stop.store(v); }

  void toggle_power(bool v) { _rpdo1.power.store(v); }

  void toggle_start(bool v) { _rpdo1.start.store(v); }

  void set_overlap1_ref(uint16_t v) { _rpdo1.overlap1_ref.store(v); }

  void set_overlap2_ref(uint16_t v) { _rpdo1.overlap2_ref.store(v); }

  // TPDO
  ConverterState converter_state() const {
    return _tpdo1.converter_state.load();
  }

  std::string_view converter_state_str() const {
    auto it = converter_state_names.find(converter_state());
    if (it == converter_state_names.end()) {
      return "н/д";
    } else {
      return it->second;
    }
  }

  std::string_view converter_state_str_upper() const {
    auto it = converter_state_names_upper.find(converter_state());
    if (it == converter_state_names_upper.end()) {
      return "Н/Д";
    } else {
      return it->second;
    }
  }

  bool is_pwm1_on() const { return _tpdo1.pwm1_on.load(); }

  bool is_pwm2_on() const { return _tpdo1.pwm2_on.load(); }

  bool has_error() const { return _tpdo1.error.load(); }

  bool has_any_warning() const { return _tpdo1.warning.load(); }

  float overlap1() const { return _tpdo1.overlap1.load(); }

  float overlap2() const { return _tpdo1.overlap2.load(); }

  float hv_voltage() const { return _tpdo2.hv_voltage.load(); }

  float lv_voltage() const { return _tpdo2.lv_voltage.load(); }

  float current1() const { return _tpdo3.current1.load(); }

  float current2() const { return _tpdo3.current2.load(); }

  bool has_emergency() const { return _tpdo1.emergency.load(); }

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
};

} // namespace psfb
