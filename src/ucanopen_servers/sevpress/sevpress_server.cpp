#include "sevpress_server.hpp"

namespace sevpress {

Server::Server(std::shared_ptr<can::Socket> socket,
               ucanopen::NodeId node_id,
               std::string const& name)
    : ucanopen::Server(socket, node_id, name, object_dictionary),
      ucanopen::SdoSubscriber(sdo_service) {
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
  rpdo_service.register_rpdo(ucanopen::CobRpdo::rpdo2,
                             std::chrono::milliseconds(100),
                             [this]() { return this->_create_rpdo2(); });
}

ucanopen::FrameHandlingStatus
Server::handle_sdo(ucanopen::ODEntryIter entry,
                   [[maybe_unused]] ucanopen::SdoType sdo_type,
                   ucanopen::ExpeditedSdoData data) {
  if (entry->second.name == "syslogmsg") {
    SyslogMessage message(data.u32());
    if (message.valid()) {
      bsclog::log("{} @syslog: {}", message.level(), message.message());
    }
  }

  return ucanopen::FrameHandlingStatus::success;
}

//----------------------------------------------------------------------------------------------------------------------
void Server::_handle_tpdo1(ucanopen::can_payload const& payload) {
  static_assert(sizeof(CobTpdo1) == 8);
  CobTpdo1 tpdo = ucanopen::from_payload<CobTpdo1>(payload);

  if (drive_state_values.contains(tpdo.drive_state)) {
    _tpdo1.drive_state.store(static_cast<DriveState>(tpdo.drive_state));
  }

  _tpdo1.pwm_on.store(static_cast<bool>(tpdo.pwm_on));
  _tpdo1.error.store(static_cast<bool>(tpdo.error));
  _tpdo1.warning.store(static_cast<bool>(tpdo.warning));

  if (opmode_values.contains(tpdo.opmode)) {
    _tpdo1.opmode.store(static_cast<OperatingMode>(tpdo.opmode));
  }

  if (ctlmode_values.contains(tpdo.ctlmode)) {
    _tpdo1.ctlmode.store(static_cast<ControlMode>(tpdo.ctlmode));
  }

  if (ctlloop_values.contains(tpdo.ctlloop)) {
    _tpdo1.ctlloop.store(static_cast<ControlLoop>(tpdo.ctlloop));
  }
}

void Server::_handle_tpdo2(ucanopen::can_payload const& payload) {
  static_assert(sizeof(CobTpdo2) == 8);
  CobTpdo2 tpdo = ucanopen::from_payload<CobTpdo2>(payload);

  _tpdo2.dc_voltage.store(tpdo.dc_voltage / 10.f);
  _tpdo2.stator_current.store(tpdo.phase_current / 10.f);
}

void Server::_handle_tpdo3(ucanopen::can_payload const& payload) {
  static_assert(sizeof(CobTpdo3) == 8);
  CobTpdo3 tpdo = ucanopen::from_payload<CobTpdo3>(payload);

  _tpdo3.speed.store(tpdo.speed);
  _tpdo3.angle.store(tpdo.angle);
}

void Server::_handle_tpdo4(ucanopen::can_payload const& payload) {
  static_assert(sizeof(CobTpdo4) == 8);
  CobTpdo4 tpdo = ucanopen::from_payload<CobTpdo4>(payload);

  size_t domain = tpdo.domain;
  if (domain >= syslog::domains.size()) {
    return;
  }

  _errors[domain].store(tpdo.errors);
  _warnings[domain].store(tpdo.warnings);
}

ucanopen::can_payload Server::_create_rpdo1() {
  static_assert(sizeof(CobRpdo1) == 8);
  static unsigned int counter = 0;

  CobRpdo1 rpdo{};

  rpdo.emergency = _rpdo1.emergency_stop.load();
  rpdo.power = _rpdo1.power.load();
  rpdo.start = _rpdo1.start.load();
  rpdo.ctlmode = std::to_underlying(_rpdo1.ctlmode.load());

  rpdo.torque_ref = 10000.0f * _rpdo1.ref_torque.load();
  rpdo.speed_ref = _rpdo1.ref_speed.load();

  rpdo.counter = counter++;

  return ucanopen::to_payload<CobRpdo1>(rpdo);
}

ucanopen::can_payload Server::_create_rpdo2() {
  static_assert(sizeof(CobRpdo2) == 8);
  static unsigned int counter = 0;

  CobRpdo2 rpdo{};

  rpdo.angle_ref = _rpdo2.ref_angle.load();
  rpdo.current_ref = 10000.f * _rpdo2.ref_current.load();
  rpdo.voltage_ref = 10000.f * _rpdo2.ref_voltage.load();

  rpdo.opmode = std::to_underlying(_rpdo2.opmode.load());
  rpdo.ctlloop = std::to_underlying(_rpdo2.ctlloop.load());

  rpdo.counter = counter++;

  return ucanopen::to_payload<CobRpdo2>(rpdo);
}

} // namespace sevpress
