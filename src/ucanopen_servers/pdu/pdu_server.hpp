#pragma once

#include "pdu_def.hpp"
#include <algorithm>
#include <array>
#include <atomic>
#include <bitset>
#include <bsclog/bsclog.h>
#include <cmath>
#include <initializer_list>
#include <optional>
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
    std::atomic<bool> connected{false};
  };

  struct {
    std::atomic<Mode> mode{Mode::idle};
    std::atomic<bool> inverter_request{false};
    std::atomic<ContactorPosition> fuelcell_main{ContactorPosition::open};
    std::atomic<ContactorPosition> fuelcell_precharge{ContactorPosition::open};
    std::atomic<ContactorPosition> inverter_main{ContactorPosition::open};
    std::atomic<ContactorPosition> inverter_precharge{ContactorPosition::open};
  } _rpdo1;

  // Подстановки для RPDO2 и RPDO3. PDU стартует с нулевых напряжений и с
  // контактами, следующими команде, — с того же начинается и поток.
  std::array<std::atomic<float>, voltage_count> _voltage_substitutes{};
  std::array<std::atomic<std::optional<ContactorPosition>>, contactor_count>
      _contactor_substitutes{};

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

  // Команды RPDO1: монитор замещает КВУ. Если на шине сам КВУ, команды нужно
  // выключить: второй источник RPDO1 сбивает счётчик кадров, и PDU отбрасывает
  // команды обоих.
  bool commanding() const {
    return rpdo_service.enabled(ucanopen::CobRpdo::rpdo1);
  }

  void set_commanding(bool v) {
    if (v) {
      rpdo_service.enable(ucanopen::CobRpdo::rpdo1);
    } else {
      rpdo_service.disable(ucanopen::CobRpdo::rpdo1);
    }
  }

  // Поток подстановок RPDO2 и RPDO3. КВУ этих кадров не передаёт, так что
  // поток идёт и при нём.
  bool streaming_substitutes() const {
    return rpdo_service.enabled(ucanopen::CobRpdo::rpdo2);
  }

  void set_streaming_substitutes(bool v) {
    for (auto rpdo : {ucanopen::CobRpdo::rpdo2, ucanopen::CobRpdo::rpdo3}) {
      if (v) {
        rpdo_service.enable(rpdo);
      } else {
        rpdo_service.disable(rpdo);
      }
    }
  }

  float voltage_substitute(Voltage v) const {
    return _voltage_substitutes[std::to_underlying(v)].load();
  }

  // Нечисловое значение в кадр не закодировать — остаётся прежнее.
  void set_voltage_substitute(Voltage v, float value) {
    if (std::isfinite(value)) {
      _voltage_substitutes[std::to_underlying(v)].store(value);
    }
  }

  // Пустое значение — контакт следует команде.
  std::optional<ContactorPosition> contactor_substitute(Contactor c) const {
    return _contactor_substitutes[std::to_underlying(c)].load();
  }

  void set_contactor_substitute(Contactor c,
                                std::optional<ContactorPosition> p) {
    _contactor_substitutes[std::to_underlying(c)].store(p);
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

  bool fuelcell_connected() const { return _fuelcell.connected.load(); }

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

  bool inverter_connected() const { return _inverter.connected.load(); }

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

  // Активен ли статус на любом уровне — как trouble::active() в прошивке.
  template<typename Status>
  bool active(Status) const {
    return std::ranges::any_of(
        _status, [](auto const& lv) { return lv.test(Status::id); });
  }
private:
  void _handle_tpdo1(ucanopen::can_payload const& payload);
  void _handle_tpdo2(ucanopen::can_payload const& payload);
  void _handle_tpdo3(ucanopen::can_payload const& payload);
  void _handle_tpdo4(ucanopen::can_payload const& payload);

  void _handle_branch_tpdo(Branch& branch,
                           int16_t voltage,
                           int16_t current,
                           ContactorReport main,
                           ContactorReport precharge,
                           BranchReport report);

  ucanopen::can_payload _create_rpdo1();
  ucanopen::can_payload _create_rpdo2();
  ucanopen::can_payload _create_rpdo3();
};

} // namespace pdu
