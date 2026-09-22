#include "psfb_server.hpp"

namespace psfb {

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

  rpdo_service.register_rpdo(ucanopen::CobRpdo::rpdo1,
                             std::chrono::milliseconds(100),
                             [this]() { return this->_create_rpdo1(); });
}

//----------------------------------------------------------------------------------------------------------------------
void Server::_handle_tpdo1(ucanopen::can_payload const& payload) {
  static_assert(sizeof(CobTpdo1) == 8);
  CobTpdo1 tpdo = ucanopen::from_payload<CobTpdo1>(payload);

  if (converter_state_values.contains(tpdo.converter_state)) {
    _tpdo1.converter_state.store(
            static_cast<ConverterState>(tpdo.converter_state));
  }

  _tpdo1.pwm1_on.store(static_cast<bool>(tpdo.pwm1_on));
  _tpdo1.pwm2_on.store(static_cast<bool>(tpdo.pwm2_on));
  _tpdo1.emergency.store(static_cast<bool>(tpdo.emergency));
  _tpdo1.critical.store(static_cast<bool>(tpdo.critical));
  _tpdo1.error.store(static_cast<bool>(tpdo.error));
  _tpdo1.warning.store(static_cast<bool>(tpdo.warning));

  _tpdo1.overlap1.store(tpdo.overlap1 / overlap_scale);
  _tpdo1.overlap2.store(tpdo.overlap2 / overlap_scale);
}

void Server::_handle_tpdo2(ucanopen::can_payload const& payload) {
  static_assert(sizeof(CobTpdo2) == 8);
  CobTpdo2 tpdo = ucanopen::from_payload<CobTpdo2>(payload);

  _tpdo2.hv_voltage.store(tpdo.hv_voltage / 10.f);
  _tpdo2.lv_voltage.store(tpdo.lv_voltage / 10.f);
}

void Server::_handle_tpdo3(ucanopen::can_payload const& payload) {
  static_assert(sizeof(CobTpdo3) == 8);
  CobTpdo3 tpdo = ucanopen::from_payload<CobTpdo3>(payload);

  _tpdo3.current1.store(tpdo.current1 / 10.f);
  _tpdo3.current2.store(tpdo.current2 / 10.f);
}

void Server::_handle_tpdo4(ucanopen::can_payload const& payload) {
  static_assert(sizeof(CobTpdo4) == 8);
  CobTpdo4 tpdo = ucanopen::from_payload<CobTpdo4>(payload);

  auto level = static_cast<size_t>(tpdo.level);
  if (level < sys::diag::level_count) {
    _status[level] = std::bitset<sys::status::status_count>(tpdo.flags);
  }
}

ucanopen::can_payload Server::_create_rpdo1() {
  static_assert(sizeof(CobRpdo1) == 8);
  static unsigned int counter = 0;

  CobRpdo1 rpdo{};

  rpdo.emergency = _rpdo1.emergency_stop.load();
  rpdo.power = _rpdo1.power.load();
  rpdo.start = _rpdo1.start.load();

  rpdo.overlap1 = _rpdo1.overlap1_ref.load();
  rpdo.overlap2 = _rpdo1.overlap2_ref.load();

  rpdo.counter = counter++;

  return ucanopen::to_payload<CobRpdo1>(rpdo);
}

} // namespace psfb
