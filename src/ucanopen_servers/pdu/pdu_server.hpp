#pragma once

#include "pdu_def.hpp"
#include <algorithm>
#include <atomic>
#include <bitset>
#include <bsclog/bsclog.h>
#include <ucanopen/server/server.h>

namespace pdu {

extern ucanopen::ObjectDictionary const object_dictionary;

class Server : public ucanopen::Server {
public:
  Server(std::shared_ptr<can::Socket> socket,
         ucanopen::NodeId node_id,
         std::string const& name);
private:
  // Телеметрия одной ветви: TPDO2 — ЭХГ, TPDO3 — инвертор.
  struct Branch {
    std::atomic<float> voltage{0};
    std::atomic<float> current{0};
    std::atomic<ContactorPosition> main_command{ContactorPosition::open};
    std::atomic<ContactorPosition> main_feedback{ContactorPosition::open};
    std::atomic<ContactorPosition> precharge_command{ContactorPosition::open};
    std::atomic<ContactorPosition> precharge_feedback{ContactorPosition::open};
    std::atomic<BranchState> state{BranchState::disconnected};
  };

  struct {
    std::atomic<Mode> mode{Mode::idle};
    std::atomic<bool> inverter_request{false};
    std::atomic<ContactorPosition> fuelcell_main{ContactorPosition::open};
    std::atomic<ContactorPosition> fuelcell_precharge{ContactorPosition::open};
    std::atomic<ContactorPosition> inverter_main{ContactorPosition::open};
    std::atomic<ContactorPosition> inverter_precharge{ContactorPosition::open};
  } _rpdo1;

  Branch _fuelcell; // TPDO2
  Branch _inverter; // TPDO3

  struct {
    std::atomic<float> battery_voltage{0};
    std::atomic<float> mcu_temperature{0};
  } _tpdo4;

  std::array<std::bitset<status::status_count>, trouble::level_count> _status{};
public:
  // RPDO
  void set_mode(Mode v) { _rpdo1.mode.store(v); }

  void toggle_inverter_request(bool v) { _rpdo1.inverter_request.store(v); }

  void set_fuelcell_main(ContactorPosition v) { _rpdo1.fuelcell_main.store(v); }

  void set_fuelcell_precharge(ContactorPosition v) {
    _rpdo1.fuelcell_precharge.store(v);
  }

  void set_inverter_main(ContactorPosition v) { _rpdo1.inverter_main.store(v); }

  void set_inverter_precharge(ContactorPosition v) {
    _rpdo1.inverter_precharge.store(v);
  }

  Mode mode() const { return _rpdo1.mode.load(); }

  std::string_view mode_str() const {
    auto it = mode_names.find(mode());
    if (it == mode_names.end()) {
      return "н/д";
    } else {
      return it->second;
    }
  }

  std::string_view mode_str_upper() const {
    auto it = mode_names_upper.find(mode());
    if (it == mode_names_upper.end()) {
      return "Н/Д";
    } else {
      return it->second;
    }
  }

  bool inverter_requested() const { return _rpdo1.inverter_request.load(); }

  // TPDO
  static std::string_view branch_state_str(BranchState v) {
    auto it = branch_state_names.find(v);
    if (it == branch_state_names.end()) {
      return "н/д";
    } else {
      return it->second;
    }
  }

  static std::string_view contactor_position_str(ContactorPosition v) {
    auto it = contactor_position_names.find(v);
    if (it == contactor_position_names.end()) {
      return "н/д";
    } else {
      return it->second;
    }
  }

  float fuelcell_voltage() const { return _fuelcell.voltage.load(); }

  float fuelcell_current() const { return _fuelcell.current.load(); }

  ContactorPosition fuelcell_main_command() const {
    return _fuelcell.main_command.load();
  }

  ContactorPosition fuelcell_main_feedback() const {
    return _fuelcell.main_feedback.load();
  }

  ContactorPosition fuelcell_precharge_command() const {
    return _fuelcell.precharge_command.load();
  }

  ContactorPosition fuelcell_precharge_feedback() const {
    return _fuelcell.precharge_feedback.load();
  }

  BranchState fuelcell_state() const { return _fuelcell.state.load(); }

  std::string_view fuelcell_state_str() const {
    return branch_state_str(fuelcell_state());
  }

  float inverter_voltage() const { return _inverter.voltage.load(); }

  float inverter_current() const { return _inverter.current.load(); }

  ContactorPosition inverter_main_command() const {
    return _inverter.main_command.load();
  }

  ContactorPosition inverter_main_feedback() const {
    return _inverter.main_feedback.load();
  }

  ContactorPosition inverter_precharge_command() const {
    return _inverter.precharge_command.load();
  }

  ContactorPosition inverter_precharge_feedback() const {
    return _inverter.precharge_feedback.load();
  }

  BranchState inverter_state() const { return _inverter.state.load(); }

  std::string_view inverter_state_str() const {
    return branch_state_str(inverter_state());
  }

  float battery_voltage() const { return _tpdo4.battery_voltage.load(); }

  float mcu_temperature() const { return _tpdo4.mcu_temperature.load(); }

  auto const& status() const { return _status; }

  bool has_status(trouble::level lv) const {
    return _status[std::to_underlying(lv)].any();
  }

  bool has_any_warning() const { return has_status(trouble::level::warning); }

  bool has_error() const { return has_status(trouble::level::error); }

  bool has_critical() const { return has_status(trouble::level::critical); }

  bool has_emergency() const { return has_status(trouble::level::emergency); }
private:
  void _handle_tpdo1(ucanopen::can_payload const& payload);
  void _handle_tpdo2(ucanopen::can_payload const& payload);
  void _handle_tpdo3(ucanopen::can_payload const& payload);
  void _handle_tpdo4(ucanopen::can_payload const& payload);

  void _handle_branch_tpdo(Branch& branch,
                           int16_t voltage,
                           int16_t current,
                           uint8_t main,
                           uint8_t precharge,
                           uint8_t state);

  ucanopen::can_payload _create_rpdo1();
};

} // namespace pdu
