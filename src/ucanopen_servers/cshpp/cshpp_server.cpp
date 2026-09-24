#include "cshpp_server.hpp"

namespace cshpp {

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

  // Период команд задан прошивкой (contract::cshpp::operator_command_period).
  // Их потерю установка только отмечает и работает дальше.
  rpdo_service.register_rpdo(ucanopen::CobRpdo::rpdo1,
                             std::chrono::milliseconds(100),
                             [this]() { return this->_create_rpdo1(); });
  rpdo_service.register_rpdo(ucanopen::CobRpdo::rpdo2,
                             std::chrono::milliseconds(100),
                             [this]() { return this->_create_rpdo2(); });
  rpdo_service.register_rpdo(ucanopen::CobRpdo::rpdo3,
                             std::chrono::milliseconds(100),
                             [this]() { return this->_create_rpdo3(); });
  // Монитор, запущенный на шине со станцией, не должен перебить её ни одним
  // кадром.
  set_commanding(false);

  // Период потока задан прошивкой (contract::substitute_period). Получив первый
  // кадр, установка следит за потоком: прервётся — поднимет потерю потока
  // подстановок, и та дорастёт до нормального останова.
  rpdo_service.register_rpdo(ucanopen::CobRpdo::rpdo4,
                             std::chrono::milliseconds(100),
                             [this]() { return this->_create_rpdo4(); });
}

//----------------------------------------------------------------------------------------------------------------------
void Server::_handle_tpdo1(ucanopen::can_payload const& payload) {
  static_assert(sizeof(CobTpdo1) == 8);
  CobTpdo1 tpdo = ucanopen::from_payload<CobTpdo1>(payload);
  if (!is_valid(tpdo)) {
    return;
  }

  auto level = static_cast<size_t>(tpdo.level);
  if (level < trouble::level_count) {
    _status[level] = std::bitset<status::status_count>(tpdo.flags);
  }
}

void Server::_handle_tpdo2(ucanopen::can_payload const& payload) {
  static_assert(sizeof(CobTpdo2) == 8);
  CobTpdo2 tpdo = ucanopen::from_payload<CobTpdo2>(payload);
  if (!is_valid(tpdo)) {
    return;
  }

  _tpdo2.hydrogen_pressure.store(tpdo.hydrogen_pressure / 10.f);
  _tpdo2.hydrogen_flow.store(tpdo.hydrogen_flow / 10.f);
  _tpdo2.mode.store(static_cast<Mode>(tpdo.mode));
  _tpdo2.stage.store(tpdo.state);
}

void Server::_handle_tpdo3(ucanopen::can_payload const& payload) {
  static_assert(sizeof(CobTpdo3) == 8);
  CobTpdo3 tpdo = ucanopen::from_payload<CobTpdo3>(payload);
  if (!is_valid(tpdo)) {
    return;
  }

  _tpdo3.hydrogen_supply.store(unpack_valve_bit(tpdo.hydrogen_supply.open));
  _tpdo3.fuelcell_inlet.store(unpack_valve_bit(tpdo.fuelcell_inlet.open));
}

// Счётчик у каждого кадра свой: установка проверяет их порознь.
ucanopen::can_payload Server::_create_rpdo1() {
  static_assert(sizeof(CobRpdo1) == 8);
  static unsigned int counter = 0;

  CobRpdo1 rpdo{};

  rpdo.mode = std::to_underlying(requested_mode());

  rpdo.counter = counter++;

  return ucanopen::to_payload<CobRpdo1>(rpdo);
}

ucanopen::can_payload Server::_create_rpdo2() {
  static_assert(sizeof(CobRpdo2) == 8);
  static unsigned int counter = 0;

  CobRpdo2 rpdo{};

  rpdo.hydrogen_supply = pack_valve_request(valve_level(Valve::hydrogen_supply));
  rpdo.fuelcell_inlet = pack_valve_request(valve_level(Valve::fuelcell_inlet));
  rpdo.storage_inlet = pack_valve_request(valve_level(Valve::storage_inlet));
  rpdo.storage_outlet = pack_valve_request(valve_level(Valve::storage_outlet));

  rpdo.counter = counter++;

  return ucanopen::to_payload<CobRpdo2>(rpdo);
}

ucanopen::can_payload Server::_create_rpdo3() {
  static_assert(sizeof(CobRpdo3) == 8);
  static unsigned int counter = 0;

  CobRpdo3 rpdo{};

  rpdo.fuelcell_main =
      pack_contactor_request(contactor_level(Contactor::fuelcell_main));
  rpdo.fuelcell_precharge =
      pack_contactor_request(contactor_level(Contactor::fuelcell_precharge));
  rpdo.inverter_main =
      pack_contactor_request(contactor_level(Contactor::inverter_main));
  rpdo.inverter_precharge =
      pack_contactor_request(contactor_level(Contactor::inverter_precharge));

  rpdo.counter = counter++;

  return ucanopen::to_payload<CobRpdo3>(rpdo);
}

ucanopen::can_payload Server::_create_rpdo4() {
  static_assert(sizeof(CobRpdo4) == 8);
  static unsigned int counter = 0;

  CobRpdo4 rpdo{};

  rpdo.hydrogen_pressure =
      encode_deci(substitute(Substitute::hydrogen_pressure));
  rpdo.hydrogen_flow = encode_deci(substitute(Substitute::hydrogen_flow));

  rpdo.counter = counter++;

  return ucanopen::to_payload<CobRpdo4>(rpdo);
}

} // namespace cshpp
