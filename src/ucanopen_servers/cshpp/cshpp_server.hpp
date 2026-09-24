#pragma once

#include "cshpp_def.hpp"
#include <algorithm>
#include <array>
#include <atomic>
#include <bitset>
#include <bsclog/bsclog.h>
#include <cmath>
#include <initializer_list>
#include <ucanopen/server/server.h>

namespace cshpp {

extern ucanopen::ObjectDictionary const object_dictionary;

class Server : public ucanopen::Server {
public:
  Server(std::shared_ptr<can::Socket> socket,
         ucanopen::NodeId node_id,
         std::string const& name);
private:
  // Команды станции оператора: режим (RPDO1) и ручные уровни продувки —
  // клапаны (RPDO2) и контакторы PDU (RPDO3). Незаданный уровень — «закрыт» и
  // «разомкнут», как у самой установки.
  std::atomic<Mode> _mode_request{Mode::idle};
  std::array<std::atomic<ValvePosition>, valve_count> _valve_levels{};
  std::array<std::atomic<ContactorPosition>, contactor_count>
      _contactor_levels{};

  // Подстановки для RPDO4 в единицах телеметрии. Установка стартует с нулевых
  // подстановок, поэтому с нулей начинается и поток.
  std::array<std::atomic<float>, substitute_count> _substitutes{};

  struct {
    std::atomic<float> hydrogen_pressure{0};
    std::atomic<float> hydrogen_flow{0};
    std::atomic<Mode> mode{Mode::idle};
    std::atomic<uint8_t> stage{0};
  } _tpdo2;

  struct {
    std::atomic<ValvePosition> hydrogen_supply{ValvePosition::closed};
    std::atomic<ValvePosition> fuelcell_inlet{ValvePosition::closed};
  } _tpdo3;

  std::array<std::bitset<status::status_count>, trouble::level_count> _status{};
public:
  // Команды станции RPDO1–3 включаются и выключаются вместе: монитор замещает
  // станцию оператора. Если на шине сама станция, команды нужно выключить:
  // второй источник сбивает счётчик кадров, и установка отбрасывает команды
  // обоих. Поэтому они выключены, пока их не включат.
  bool commanding() const {
    return rpdo_service.enabled(ucanopen::CobRpdo::rpdo1);
  }

  void set_commanding(bool v) {
    for (auto rpdo : {ucanopen::CobRpdo::rpdo1,
                      ucanopen::CobRpdo::rpdo2,
                      ucanopen::CobRpdo::rpdo3}) {
      if (v) {
        rpdo_service.enable(rpdo);
      } else {
        rpdo_service.disable(rpdo);
      }
    }
  }

  // Поток подстановок RPDO4. Станция этого кадра не передаёт, так что поток
  // идёт и при ней.
  bool streaming_substitutes() const {
    return rpdo_service.enabled(ucanopen::CobRpdo::rpdo4);
  }

  void set_streaming_substitutes(bool v) {
    if (v) {
      rpdo_service.enable(ucanopen::CobRpdo::rpdo4);
    } else {
      rpdo_service.disable(ucanopen::CobRpdo::rpdo4);
    }
  }

  // RPDO
  void set_mode(Mode v) { _mode_request.store(v); }

  Mode requested_mode() const { return _mode_request.load(); }

  void set_valve_level(Valve v, ValvePosition p) {
    _valve_levels[std::to_underlying(v)].store(p);
  }

  ValvePosition valve_level(Valve v) const {
    return _valve_levels[std::to_underlying(v)].load();
  }

  void set_contactor_level(Contactor c, ContactorPosition p) {
    _contactor_levels[std::to_underlying(c)].store(p);
  }

  ContactorPosition contactor_level(Contactor c) const {
    return _contactor_levels[std::to_underlying(c)].load();
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

  static std::string_view valve_position_str(ValvePosition v) {
    auto it = valve_position_names.find(v);
    if (it == valve_position_names.end()) {
      return "н/д";
    } else {
      return it->second;
    }
  }

  float hydrogen_pressure() const { return _tpdo2.hydrogen_pressure.load(); }

  float hydrogen_flow() const { return _tpdo2.hydrogen_flow.load(); }

  Mode mode() const { return _tpdo2.mode.load(); }

  // Номер этапа принадлежит прошивке: показываем как есть, а имя — только
  // если номер известен клиенту.
  uint8_t stage() const { return _tpdo2.stage.load(); }

  std::string_view stage_str() const {
    auto it = stage_names.find(static_cast<Stage>(stage()));
    if (it == stage_names.end()) {
      return "н/д";
    } else {
      return it->second;
    }
  }

  ValvePosition hydrogen_supply_valve() const {
    return _tpdo3.hydrogen_supply.load();
  }

  ValvePosition fuelcell_inlet_valve() const {
    return _tpdo3.fuelcell_inlet.load();
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

  ucanopen::can_payload _create_rpdo1();
  ucanopen::can_payload _create_rpdo2();
  ucanopen::can_payload _create_rpdo3();
  ucanopen::can_payload _create_rpdo4();
};

} // namespace cshpp
