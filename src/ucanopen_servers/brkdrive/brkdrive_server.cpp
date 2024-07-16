#include "brkdrive_server.h"
#include <boost/crc.hpp>


namespace brkdrive {

Server::Server(std::shared_ptr<can::Socket> socket, ucanopen::NodeId node_id, const std::string& name)
        : ucanopen::Server(socket, node_id, name, object_dictionary)
        , ucanopen::SdoSubscriber(sdo_service) {
    tpdo_service.register_tpdo(ucanopen::CobTpdo::tpdo1, std::chrono::milliseconds(1000),
                               [this](ucanopen::can_payload payload) { this->_handle_tpdo1(payload); });
    tpdo_service.register_tpdo(ucanopen::CobTpdo::tpdo2, std::chrono::milliseconds(1000),
                               [this](ucanopen::can_payload payload) { this->_handle_tpdo2(payload); });
    tpdo_service.register_tpdo(ucanopen::CobTpdo::tpdo3, std::chrono::milliseconds(1000),
                               [this](ucanopen::can_payload payload) { this->_handle_tpdo3(payload); });
    tpdo_service.register_tpdo(ucanopen::CobTpdo::tpdo4, std::chrono::milliseconds(1000),
                               [this](ucanopen::can_payload payload) { this->_handle_tpdo4(payload); });

    rpdo_service.register_rpdo(ucanopen::CobRpdo::rpdo1, std::chrono::milliseconds(100),
                               [this](){ return this->_create_rpdo1(); });
    rpdo_service.register_rpdo(ucanopen::CobRpdo::rpdo2, std::chrono::milliseconds(100),
                               [this](){ return this->_create_rpdo2(); });
    rpdo_service.register_rpdo(ucanopen::CobRpdo::rpdo3, std::chrono::milliseconds(100),
                               [this](){ return this->_create_rpdo3(); });
}


ucanopen::FrameHandlingStatus Server::handle_sdo(ucanopen::ODEntryIter entry,
                                                 [[maybe_unused]] ucanopen::SdoType sdo_type,
                                                 ucanopen::ExpeditedSdoData data) {
    if (entry->second.name == "syslog_message") {
        auto message_id = data.u32();
        if ((message_id != 0) && (message_id < syslog_messages.size())) {
            bsclog::info("{}", syslog_messages[message_id]);
        }
    }

    return ucanopen::FrameHandlingStatus::success;
}


//----------------------------------------------------------------------------------------------------------------------
void Server::_handle_tpdo1(const ucanopen::can_payload& payload){
    static_assert(sizeof(CobTpdo1) == 8);
    CobTpdo1 tpdo = ucanopen::from_payload<CobTpdo1>(payload);

    _tpdo1.angle.store(tpdo.angle);

    if (opstatus_values.contains(tpdo.opstatus)) {
        _tpdo1.opstatus.store(static_cast<OperatingStatus>(tpdo.opstatus));
    }

    _tpdo1.calibrated.store(tpdo.calibrated);

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

    if (calibration_state_values.contains(tpdo.calstatus)) {
        _tpdo1.calstatus.store(static_cast<CalibrationState>(tpdo.calstatus));
    }

    _tpdo1.trkcompleted.store(tpdo.trkcompleted);
}


void Server::_handle_tpdo2(const ucanopen::can_payload& payload) {
    static_assert(sizeof(CobTpdo2) == 8);
    CobTpdo2 tpdo = ucanopen::from_payload<CobTpdo2>(payload);

    _tpdo2.ref_angle.store(tpdo.ref_angle);
    _tpdo2.ref_brake.store(tpdo.ref_brake / 100.f);
}


void Server::_handle_tpdo3(const ucanopen::can_payload& payload) {
    static_assert(sizeof(CobTpdo3) == 8);
    CobTpdo3 tpdo = ucanopen::from_payload<CobTpdo3>(payload);

    _tpdo3.speed.store(tpdo.speed);
}


void Server::_handle_tpdo4(const ucanopen::can_payload& payload){
    static_assert(sizeof(CobTpdo4) == 8);
    CobTpdo4 tpdo = ucanopen::from_payload<CobTpdo4>(payload);

    _tpdo4.errors.store(tpdo.errors);
    _tpdo4.warnings.store(tpdo.warnings);
}


//----------------------------------------------------------------------------------------------------------------------
ucanopen::can_payload Server::_create_rpdo1() {
    static_assert(sizeof(CobRpdo1) == 8);
    static unsigned int counter = 0;
    
    CobRpdo1 rpdo{};

    rpdo.ref_angle = _rpdo1.ref_angle.load();
    rpdo.track_speed = _rpdo1.track_speed.load();
    rpdo.master_bad = _rpdo1.master_bad.load();
    rpdo.wakeup = _rpdo1.wakeup.load();
    rpdo.cmd_opstatus = std::to_underlying(_rpdo1.cmd_opstatus.load());

    rpdo.counter = counter++;

    return ucanopen::to_payload<CobRpdo1>(rpdo);
}


ucanopen::can_payload Server::_create_rpdo2() {
    static_assert(sizeof(CobRpdo2) == 8);
    static unsigned int counter = 0;

    CobRpdo2 rpdo{};

    rpdo.opmode = std::to_underlying(_rpdo2.opmode.load());
    rpdo.ctlmode = std::to_underlying(_rpdo2.ctlmode.load());
    rpdo.ctlloop = std::to_underlying(_rpdo2.ctlloop.load());
    rpdo.ref_torque = 10000.0f * _rpdo2.ref_torque.load();
    rpdo.ref_speed = _rpdo2.ref_speed.load();

    rpdo.counter = counter++;

    return ucanopen::to_payload<CobRpdo2>(rpdo);
}


ucanopen::can_payload Server::_create_rpdo3() {
    static_assert(sizeof(CobRpdo3) == 8);
    static unsigned int counter = 0;

    CobRpdo3 rpdo{};

    rpdo.ref_dcurr = 10000.0f * _rpdo3.ref_dcurr.load();
    rpdo.openloop_ref_angle = _rpdo3.openloop_ref_angle.load();

    rpdo.counter = counter++;

    return ucanopen::to_payload<CobRpdo3>(rpdo);
}


} // namespace brkdrive
