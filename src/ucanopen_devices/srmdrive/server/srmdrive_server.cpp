#include "srmdrive_server.h"
#include <boost/crc.hpp>


namespace srmdrive {

Server::Server(std::shared_ptr<can::Socket> socket, ucanopen::NodeId node_id, const std::string& name)
        : ucanopen::Server(socket, node_id, name, object_dictionary)
        , ucanopen::SdoSubscriber(sdo_service) {
    tpdo_service.register_tpdo(ucanopen::TpdoType::tpdo1, std::chrono::milliseconds(1000),
                               [this](ucanopen::can_payload payload) { this->_handle_tpdo1(payload); });
    tpdo_service.register_tpdo(ucanopen::TpdoType::tpdo2, std::chrono::milliseconds(1000),
                               [this](ucanopen::can_payload payload) { this->_handle_tpdo2(payload); });
    tpdo_service.register_tpdo(ucanopen::TpdoType::tpdo3, std::chrono::milliseconds(1000),
                               [this](ucanopen::can_payload payload) { this->_handle_tpdo3(payload); });
    tpdo_service.register_tpdo(ucanopen::TpdoType::tpdo4, std::chrono::milliseconds(1000),
                               [this](ucanopen::can_payload payload) { this->_handle_tpdo4(payload); });

    rpdo_service.register_rpdo(ucanopen::RpdoType::rpdo1, std::chrono::milliseconds(100),
                               [this](){ return this->_create_rpdo1(); });
    rpdo_service.register_rpdo(ucanopen::RpdoType::rpdo2, std::chrono::milliseconds(100),
                               [this](){ return this->_create_rpdo2(); });
}


ucanopen::FrameHandlingStatus Server::handle_sdo(ucanopen::ODEntryIter entry,
                                                 ucanopen::SdoType sdo_type,
                                                 ucanopen::ExpeditedSdoData data) {
    if (entry->second.name == "syslog_message") {
        auto message_id = data.u32();
        if ((message_id != 0) && (message_id < syslog_messages.size())) {
            Log() << syslog_messages[message_id] << '\n';
        }
    } else if (entry->second.category == _dictionary.config.config_category && entry->second.type == ucanopen::OD_ENUM16) {
        
    }

    return ucanopen::FrameHandlingStatus::success;
}


//----------------------------------------------------------------------------------------------------------------------
void Server::_handle_tpdo1(const ucanopen::can_payload& payload){
    CobTpdo1 tpdo = ucanopen::from_payload<CobTpdo1>(payload);
    _tpdo1.run = tpdo.run;
    _tpdo1.error = tpdo.error;
    _tpdo1.warning = tpdo.warning;
    _tpdo1.overheat = tpdo.overheat;
    _tpdo1.ctlmode = (tpdo.ctlmode == 0) ? "torque" : "speed";
    _tpdo1.ctlloop = (tpdo.ctlloop == 0) ? "closed" : "open";

    if (tpdo.drive_state >= drive_states.size()) {
        _tpdo1.drive_state = "n/a";
    } else {
        _tpdo1.drive_state = drive_states[tpdo.drive_state];
    }

    _tpdo1.torque = tpdo.torque;
    _tpdo1.speed = tpdo.speed;
}


void Server::_handle_tpdo2(const ucanopen::can_payload& payload) {
    CobTpdo2 tpdo = ucanopen::from_payload<CobTpdo2>(payload);
    _tpdo2.dc_voltage = tpdo.dc_voltage;
    _tpdo2.stator_current = tpdo.stator_current;
    _tpdo2.field_current = float(tpdo.field_current) / 10.f;
    _tpdo2.mech_power = tpdo.mech_power;
    _tpdo2.manual_field_current = tpdo.manual_field_current;
}


void Server::_handle_tpdo3(const ucanopen::can_payload& payload) {
    CobTpdo3 tpdo = ucanopen::from_payload<CobTpdo3>(payload);
    _tpdo3.pwrmodule_temp = int(tpdo.pwrmodule_temp) - 40;
    _tpdo3.excmodule_temp = int(tpdo.excmodule_temp) - 40;
    _tpdo3.pcb_temp = int(tpdo.pcb_temp) - 40;
    _tpdo3.aw_temp = int(tpdo.aw_temp) - 40;
    _tpdo3.fw_temp = int(tpdo.fw_temp) - 40;
}


void Server::_handle_tpdo4(const ucanopen::can_payload& payload){
    CobTpdo4 tpdo = ucanopen::from_payload<CobTpdo4>(payload);
    _errors = tpdo.errors;
    _warnings = tpdo.warnings;
}


//----------------------------------------------------------------------------------------------------------------------
ucanopen::can_payload Server::_create_rpdo1() {
    static unsigned int counter = 0;
    CobRpdo1 rpdo;

    rpdo.power = _power_enabled;
    rpdo.run = _run_enabled;
    rpdo.ctlmode = std::to_underlying(_ctlmode);
    rpdo.emergency_stop = _emergency_enabled;
    rpdo.torque_ref = 10000.0f * _torque_perunit_ref;
    rpdo.speed_ref = _speed_rpm_ref;

    rpdo.counter = counter;
    counter = (counter + 1) & 0x3;

    auto payload = ucanopen::to_payload<CobRpdo1>(rpdo);

    boost::crc_basic<8> crc8(0x7, 0x0, 0x0, false, false);
    crc8.process_bytes(payload.data(), 7);
    payload[7] = crc8.checksum();

    return payload;
}


ucanopen::can_payload Server::_create_rpdo2() {
    static unsigned int counter = 0;
    CobRpdo2 rpdo;

    rpdo.manual_fieldctl = _manual_fieldctl;
    rpdo.ctlloop = std::to_underlying(_ctlloop);
    rpdo.field_current_ref = 10.0f * _field_current_ref;
    rpdo.stator_current_ref = 10000.0f * _stator_current_perunit_ref;

    rpdo.counter = counter;
    counter = (counter + 1) & 0x3;

    return ucanopen::to_payload<CobRpdo2>(rpdo);
}



}

