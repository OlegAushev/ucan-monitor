#include "srmdrive_server.h"
#include <boost/crc.hpp>


namespace srmdrive {

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
}


ucanopen::FrameHandlingStatus Server::handle_sdo(ucanopen::ODEntryIter entry,
                                                 [[maybe_unused]] ucanopen::SdoType sdo_type,
                                                 ucanopen::ExpeditedSdoData data) {
    if (entry->second.name == "syslogmsg") {
        auto message_id = data.u32();
        if ((message_id != 0) && (message_id < syslog_messages.size())) {
            bsclog::info("{}", syslog_messages[message_id]);
        }
    } else if (entry->second.category == _dictionary.config.config_category && entry->second.data_type == ucanopen::OD_ENUM16) {
        
    }

    return ucanopen::FrameHandlingStatus::success;
}


//----------------------------------------------------------------------------------------------------------------------
void Server::_handle_tpdo1(const ucanopen::can_payload& payload){
    CobTpdo1 tpdo = ucanopen::from_payload<CobTpdo1>(payload);
    Tpdo1 tpdo1_{};
    tpdo1_.run = tpdo.run;
    tpdo1_.error = tpdo.error;
    tpdo1_.warning = tpdo.warning;
    tpdo1_.overheat = tpdo.overheat;
    
    tpdo1_.ctlmode = (tpdo.ctlmode == 0) ? "torque" : "speed";
    
    switch (tpdo.ctlloop) {
    case std::to_underlying(ControlLoop::closed):
        tpdo1_.ctlloop = "closed";
        break;
    case std::to_underlying(ControlLoop::open):
        tpdo1_.ctlloop = "open";
        break;
    case std::to_underlying(ControlLoop::semiclosed):
        tpdo1_.ctlloop = "semi-closed";
        break;
    default:
        tpdo1_.ctlloop = "unknown";
        break;
    }

    if (tpdo.drive_state >= drive_states.size()) {
        tpdo1_.drive_state = "n/a";
    } else {
        tpdo1_.drive_state = drive_states[tpdo.drive_state];
    }

    tpdo1_.torque = tpdo.torque;
    tpdo1_.speed = tpdo.speed;

    _tpdo1 = tpdo1_;
}


void Server::_handle_tpdo2(const ucanopen::can_payload& payload) {
    CobTpdo2 tpdo = ucanopen::from_payload<CobTpdo2>(payload);
    Tpdo2 tpdo2_{};
    tpdo2_.dc_voltage = tpdo.dc_voltage;
    tpdo2_.stator_current = tpdo.stator_current;
    tpdo2_.field_current = float(tpdo.field_current);
    tpdo2_.mech_power = tpdo.mech_power;
    tpdo2_.manual_field_current = tpdo.manual_field_current;

    _tpdo2 = tpdo2_;
}


void Server::_handle_tpdo3(const ucanopen::can_payload& payload) {
    CobTpdo3 tpdo = ucanopen::from_payload<CobTpdo3>(payload);
    Tpdo3 tpdo3_{};
    tpdo3_.pwrmodule_temp = int(tpdo.pwrmodule_temp) - 40;
    tpdo3_.excmodule_temp = int(tpdo.excmodule_temp) - 40;
    tpdo3_.pcb_temp = int(tpdo.pcb_temp) - 40;
    tpdo3_.aw_temp = int(tpdo.aw_temp) - 40;
    tpdo3_.fw_temp = int(tpdo.fw_temp) - 40;

    _tpdo3 = tpdo3_;
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
    rpdo.ctlmode = std::to_underlying(_ctlmode.load());
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
    rpdo.ctlloop = std::to_underlying(_ctlloop.load());
    rpdo.field_current_ref = 10.0f * _field_current_ref;
    rpdo.d_current_ref = 10000.0f * _d_current_perunit_ref;

    rpdo.counter = counter;
    counter = (counter + 1) & 0x3;

    auto payload = ucanopen::to_payload<CobRpdo2>(rpdo);

    boost::crc_basic<8> crc8(0x7, 0x0, 0x0, false, false);
    crc8.process_bytes(payload.data(), 7);
    payload[7] = crc8.checksum();

    return payload;
}



}

