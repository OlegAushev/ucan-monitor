#pragma once

#include "chss_def.hpp"
#include <algorithm>
#include <array>
#include <atomic>
#include <bitset>
#include <bsclog/bsclog.h>
#include <cmath>
#include <initializer_list>
#include <ucanopen/server/server.h>

namespace chss {

extern ucanopen::ObjectDictionary const object_dictionary;

class Server : public ucanopen::Server {
public:
  Server(std::shared_ptr<can::Socket> socket,
         ucanopen::NodeId node_id,
         std::string const& name);
private:
  struct {
    std::atomic<Mode> mode{Mode::storage};
    std::atomic<ValvePosition> inlet{ValvePosition::closed};
    std::atomic<ValvePosition> outlet{ValvePosition::closed};
  } _rpdo1;

  // Подстановки для RPDO2 и RPDO3 в единицах телеметрии. СХКВ стартует с
  // нулевыми подстановками, поэтому с нулей начинается и поток.
  std::array<std::atomic<float>, substitute_count> _substitutes{};

  struct {
    std::atomic<float> receiver_pressure{0};
    std::atomic<float> inflow_rate{0};
    std::atomic<float> fill_line_pressure{0};
    std::atomic<ValvePosition> inlet_valve{ValvePosition::closed};
  } _tpdo2;

  struct {
    std::atomic<float> pressure_before_reducer{0};
    std::atomic<float> pressure_after_reducer{0};
    std::atomic<ValvePosition> outlet_valve{ValvePosition::closed};
  } _tpdo3;

  struct {
    std::atomic<float> mcu_temperature{0};
    std::atomic<Mode> mode{Mode::storage};
    std::atomic<bool> supply_ready{false};
    std::atomic<bool> receiver_full{false};
    std::atomic<uint8_t> state{0};
  } _tpdo4;

  std::array<std::bitset<status::status_count>, trouble::level_count> _status{};
public:
  // RPDO
  void set_mode(Mode v) { _rpdo1.mode.store(v); }

  void set_inlet(ValvePosition v) { _rpdo1.inlet.store(v); }

  void set_outlet(ValvePosition v) { _rpdo1.outlet.store(v); }

  Mode requested_mode() const { return _rpdo1.mode.load(); }

  // Команды RPDO1: монитор замещает КВУ. Если на шине сам КВУ, команды нужно
  // выключить: второй источник RPDO1 сбивает счётчик кадров, и СХКВ
  // отбрасывает команды обоих.
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

  float substitute(Substitute q) const {
    return _substitutes[std::to_underlying(q)].load();
  }

  // Нечисловое значение в кадр не закодировать — остаётся прежнее.
  void set_substitute(Substitute q, float v) {
    if (std::isfinite(v)) {
      _substitutes[std::to_underlying(q)].store(v);
    }
  }

  // TPDO
  static std::string_view mode_str(Mode v) {
    auto it = mode_names.find(v);
    if (it == mode_names.end()) {
      return "н/д";
    } else {
      return it->second;
    }
  }

  static std::string_view mode_str_upper(Mode v) {
    auto it = mode_names_upper.find(v);
    if (it == mode_names_upper.end()) {
      return "Н/Д";
    } else {
      return it->second;
    }
  }

  static std::string_view valve_position_str(ValvePosition v) {
    auto it = valve_position_names.find(v);
    if (it == valve_position_names.end()) {
      return "н/д";
    } else {
      return it->second;
    }
  }

  float receiver_pressure() const { return _tpdo2.receiver_pressure.load(); }

  float inflow_rate() const { return _tpdo2.inflow_rate.load(); }

  float fill_line_pressure() const { return _tpdo2.fill_line_pressure.load(); }

  ValvePosition inlet_valve() const { return _tpdo2.inlet_valve.load(); }

  float pressure_before_reducer() const {
    return _tpdo3.pressure_before_reducer.load();
  }

  float pressure_after_reducer() const {
    return _tpdo3.pressure_after_reducer.load();
  }

  ValvePosition outlet_valve() const { return _tpdo3.outlet_valve.load(); }

  float mcu_temperature() const { return _tpdo4.mcu_temperature.load(); }

  Mode mode() const { return _tpdo4.mode.load(); }

  bool supply_ready() const { return _tpdo4.supply_ready.load(); }

  bool receiver_full() const { return _tpdo4.receiver_full.load(); }

  // Номер состояния принадлежит прошивке: показываем как есть, а имя — только
  // если номер известен клиенту.
  uint8_t state() const { return _tpdo4.state.load(); }

  std::string_view state_str() const {
    auto it = state_names.find(static_cast<State>(state()));
    if (it == state_names.end()) {
      return "н/д";
    } else {
      return it->second;
    }
  }

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

  ucanopen::can_payload _create_rpdo1();
  ucanopen::can_payload _create_rpdo2();
  ucanopen::can_payload _create_rpdo3();
};

} // namespace chss
