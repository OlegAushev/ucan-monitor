#include "chss_server.hpp"

namespace chss {

Server::Server(std::shared_ptr<can::Socket> socket,
               ucanopen::NodeId node_id,
               std::string const& name)
    : ucanopen::Server(socket, node_id, name, object_dictionary) {
  tpdo_service.register_tpdo(
      ucanopen::CobTpdo::tpdo1,
      std::chrono::milliseconds(1000),
      [this](ucanopen::can_payload payload) { this->_handle_tpdo1(payload); });
  tpdo_service.register_tpdo(
      ucanopen::CobTpdo::tpdo2,
      std::chrono::milliseconds(1000),
      [this](ucanopen::can_payload payload) { this->_handle_tpdo2(payload); });
  tpdo_service.register_tpdo(
      ucanopen::CobTpdo::tpdo3,
      std::chrono::milliseconds(1000),
      [this](ucanopen::can_payload payload) { this->_handle_tpdo3(payload); });
  tpdo_service.register_tpdo(
      ucanopen::CobTpdo::tpdo4,
      std::chrono::milliseconds(1000),
      [this](ucanopen::can_payload payload) { this->_handle_tpdo4(payload); });

  // Период команды задан прошивкой (contract::cshpp::chss_command_period):
  // реже — и СХКВ объявит потерю связи с КВУ.
  rpdo_service.register_rpdo(ucanopen::CobRpdo::rpdo1,
                             std::chrono::milliseconds(100),
                             [this]() { return this->_create_rpdo1(); });

  // Период потока задан прошивкой (contract::substitute_period). Получив первый
  // кадр, СХКВ следит за потоком: прервётся — поднимет потерю потока
  // подстановок, и та дорастёт до блокировки.
  rpdo_service.register_rpdo(ucanopen::CobRpdo::rpdo2,
                             std::chrono::milliseconds(100),
                             [this]() { return this->_create_rpdo2(); });
  rpdo_service.register_rpdo(ucanopen::CobRpdo::rpdo3,
                             std::chrono::milliseconds(100),
                             [this]() { return this->_create_rpdo3(); });
}

//----------------------------------------------------------------------------------------------------------------------
void Server::_handle_tpdo1(ucanopen::can_payload const& payload) {
  static_assert(sizeof(CobTpdo1) == 8);
  CobTpdo1 tpdo = ucanopen::from_payload<CobTpdo1>(payload);

  auto level = static_cast<size_t>(tpdo.level);
  if (level < trouble::level_count) {
    _status[level] = std::bitset<status::status_count>(tpdo.flags);
  }
}

void Server::_handle_tpdo2(ucanopen::can_payload const& payload) {
  static_assert(sizeof(CobTpdo2) == 8);
  CobTpdo2 tpdo = ucanopen::from_payload<CobTpdo2>(payload);

  _tpdo2.receiver_pressure.store(tpdo.receiver_pressure / 10.f);
  _tpdo2.inflow_rate.store(tpdo.inflow_rate / 10.f);
  _tpdo2.fill_line_pressure.store(tpdo.fill_line_pressure / 10.f);

  if (valve_position_values.contains(tpdo.inlet_valve)) {
    _tpdo2.inlet_valve.store(static_cast<ValvePosition>(tpdo.inlet_valve));
  }
}

void Server::_handle_tpdo3(ucanopen::can_payload const& payload) {
  static_assert(sizeof(CobTpdo3) == 8);
  CobTpdo3 tpdo = ucanopen::from_payload<CobTpdo3>(payload);

  _tpdo3.pressure_before_reducer.store(tpdo.pressure_before_reducer / 10.f);
  _tpdo3.pressure_after_reducer.store(tpdo.pressure_after_reducer / 10.f);

  if (valve_position_values.contains(tpdo.outlet_valve)) {
    _tpdo3.outlet_valve.store(static_cast<ValvePosition>(tpdo.outlet_valve));
  }
}

void Server::_handle_tpdo4(ucanopen::can_payload const& payload) {
  static_assert(sizeof(CobTpdo4) == 8);
  CobTpdo4 tpdo = ucanopen::from_payload<CobTpdo4>(payload);

  _tpdo4.mcu_temperature.store(tpdo.mcu_temperature / 10.f);

  if (mode_values.contains(tpdo.mode)) {
    _tpdo4.mode.store(static_cast<Mode>(tpdo.mode));
  }

  _tpdo4.supply_ready.store((tpdo.flags & supply_ready_mask) != 0);
  _tpdo4.receiver_full.store((tpdo.flags & receiver_full_mask) != 0);
  _tpdo4.state.store(tpdo.state);
}

ucanopen::can_payload Server::_create_rpdo1() {
  static_assert(sizeof(CobRpdo1) == 8);
  static unsigned int counter = 0;

  CobRpdo1 rpdo{};

  rpdo.mode = std::to_underlying(_rpdo1.mode.load());
  rpdo.inlet_cmd = std::to_underlying(_rpdo1.inlet.load());
  rpdo.outlet_cmd = std::to_underlying(_rpdo1.outlet.load());

  rpdo.counter = counter++;

  return ucanopen::to_payload<CobRpdo1>(rpdo);
}

// Счётчик у каждого кадра подстановок свой: СХКВ проверяет их порознь.
ucanopen::can_payload Server::_create_rpdo2() {
  static_assert(sizeof(CobRpdo2) == 8);
  static unsigned int counter = 0;

  CobRpdo2 rpdo{};

  rpdo.receiver_pressure =
      encode_deci(substitute(Substitute::receiver_pressure));
  rpdo.pressure_before_reducer =
      encode_deci(substitute(Substitute::pressure_before_reducer));
  rpdo.pressure_after_reducer =
      encode_deci(substitute(Substitute::pressure_after_reducer));

  rpdo.counter = counter++;

  return ucanopen::to_payload<CobRpdo2>(rpdo);
}

ucanopen::can_payload Server::_create_rpdo3() {
  static_assert(sizeof(CobRpdo3) == 8);
  static unsigned int counter = 0;

  CobRpdo3 rpdo{};

  rpdo.fill_line_pressure =
      encode_deci(substitute(Substitute::fill_line_pressure));
  rpdo.inflow_rate = encode_deci(substitute(Substitute::inflow_rate));

  rpdo.counter = counter++;

  return ucanopen::to_payload<CobRpdo3>(rpdo);
}

} // namespace chss
