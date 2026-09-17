#include "pdu_server.hpp"

namespace pdu {

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

  // Период команды задан прошивкой (contract::cshpp::pdu_command_period):
  // реже — и PDU объявит потерю связи с КВУ.
  rpdo_service.register_rpdo(ucanopen::CobRpdo::rpdo1,
                             std::chrono::milliseconds(100),
                             [this]() { return this->_create_rpdo1(); });
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

  _handle_branch_tpdo(_fuelcell,
                      tpdo.fuelcell_voltage,
                      tpdo.fuelcell_current,
                      tpdo.fuelcell_main,
                      tpdo.fuelcell_precharge,
                      tpdo.fuelcell_branch);
}

void Server::_handle_tpdo3(ucanopen::can_payload const& payload) {
  static_assert(sizeof(CobTpdo3) == 8);
  CobTpdo3 tpdo = ucanopen::from_payload<CobTpdo3>(payload);
  if (!is_valid(tpdo)) {
    return;
  }

  _handle_branch_tpdo(_inverter,
                      tpdo.inverter_voltage,
                      tpdo.inverter_current,
                      tpdo.inverter_main,
                      tpdo.inverter_precharge,
                      tpdo.inverter_branch);
}

void Server::_handle_tpdo4(ucanopen::can_payload const& payload) {
  static_assert(sizeof(CobTpdo4) == 8);
  CobTpdo4 tpdo = ucanopen::from_payload<CobTpdo4>(payload);
  if (!is_valid(tpdo)) {
    return;
  }

  _tpdo4.battery_voltage.store(tpdo.battery_voltage / 10.f);
  _tpdo4.mcu_temperature.store(tpdo.mcu_temperature / 10.f);
}

void Server::_handle_branch_tpdo(Branch& branch,
                                 int16_t voltage,
                                 int16_t current,
                                 ContactorReport main,
                                 ContactorReport precharge,
                                 BranchReport report) {
  branch.voltage.store(voltage / 10.f);
  branch.current.store(current / 10.f);

  branch.main_command.store(unpack_contactor_bit(main.demanded));
  branch.main_feedback.store(unpack_contactor_bit(main.actual));
  branch.precharge_command.store(unpack_contactor_bit(precharge.demanded));
  branch.precharge_feedback.store(unpack_contactor_bit(precharge.actual));

  // Незнакомая стадия хранится как есть и показывается как «н/д».
  branch.state.store(static_cast<BranchState>(report.stage));
  branch.connected.store(report.connected);
}

ucanopen::can_payload Server::_create_rpdo1() {
  static_assert(sizeof(CobRpdo1) == 8);
  static unsigned int counter = 0;

  CobRpdo1 rpdo{};

  rpdo.mode = std::to_underlying(_rpdo1.mode.load());
  rpdo.inverter_request = pack_branch_request(_rpdo1.inverter_request.load());

  rpdo.fuelcell_main = pack_contactor_request(_rpdo1.fuelcell_main.load());
  rpdo.fuelcell_precharge =
      pack_contactor_request(_rpdo1.fuelcell_precharge.load());
  rpdo.inverter_main = pack_contactor_request(_rpdo1.inverter_main.load());
  rpdo.inverter_precharge =
      pack_contactor_request(_rpdo1.inverter_precharge.load());

  rpdo.counter = counter++;

  return ucanopen::to_payload<CobRpdo1>(rpdo);
}

} // namespace pdu
