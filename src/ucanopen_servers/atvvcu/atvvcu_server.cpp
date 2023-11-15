#include "atvvcu_server.h"
#include "ucanopen/ucanopen_def.h"
#include "ucanopen_servers/atvvcu/atvvcu_def.h"
#include <utility>


namespace atvvcu {

Server::Server(std::shared_ptr<can::Socket> socket, ucanopen::NodeId node_id, const std::string& name)
        : ucanopen::Server(socket, node_id, name, object_dictionary)
        , ucanopen::SdoSubscriber(sdo_service)
{
    tpdo_service.register_tpdo(ucanopen::CobTpdo::tpdo2, std::chrono::milliseconds(1100),
            [this](ucanopen::can_payload payload) { this->_handle_tpdo2(payload); });
    tpdo_service.register_tpdo(ucanopen::CobTpdo::tpdo3, std::chrono::milliseconds(1100),
            [this](ucanopen::can_payload payload) { this->_handle_tpdo3(payload); });
    // tpdo_service.register_tpdo(ucanopen::CobTpdo::tpdo3, std::chrono::milliseconds(1100),
    //         [this](ucanopen::can_payload payload) { this->_handle_tpdo3(payload); });
    // tpdo_service.register_tpdo(ucanopen::CobTpdo::tpdo4, std::chrono::milliseconds(1100),
    //         [this](ucanopen::can_payload payload) { this->_handle_tpdo4(payload); });

    rpdo_service.register_rpdo(ucanopen::CobRpdo::rpdo2, std::chrono::milliseconds(100),
            [this](){ return this->_create_rpdo2(); });
    rpdo_service.register_rpdo(ucanopen::CobRpdo::rpdo3, std::chrono::milliseconds(50),
            [this](){ return this->_create_rpdo3(); });
    // rpdo_service.register_rpdo(ucanopen::CobRpdo::rpdo3, std::chrono::milliseconds(100),
    //         [this](){ return this->_create_rpdo3(); });
    // rpdo_service.register_rpdo(ucanopen::CobRpdo::rpdo4, std::chrono::milliseconds(500),
    //         [this](){ return this->_create_rpdo4(); });
}


ucanopen::FrameHandlingStatus Server::handle_sdo(ucanopen::ODEntryIter entry,
                                                 [[maybe_unused]] ucanopen::SdoType sdo_type,
                                                 ucanopen::ExpeditedSdoData data) {
    if (entry->second.name == "syslog_message") {
        auto message_id = data.u32();
        if ((message_id != 0) && (message_id < syslog_messages.size())) {
            bsclog::info("{}", syslog_messages[message_id]);
        }
    } else if (entry->second.name == "pdm_contactor_state") {
        // if (_pdm_mtx.try_lock()) {
        //     for (auto i = 0; i < pdm_contactor_count; ++i) {
        //         _pdm_contactor_states[i] = data.u32() & (1 << i);
        //     }
        //     _pdm_mtx.unlock();
        // }
    }

    return ucanopen::FrameHandlingStatus::success;
}


void Server::_handle_tpdo2(const ucanopen::can_payload& payload) {
    static_assert(sizeof(CobTpdo2) == 8);
    CobTpdo2 tpdo = ucanopen::from_payload<CobTpdo2>(payload);

    pdm_contactor_state[std::to_underlying(PdmContactor::battery_connect)] = tpdo.pdm_battery_connect; 
    pdm_contactor_state[std::to_underlying(PdmContactor::motor1_bypass)] = tpdo.pdm_motor1_bypass; 
    pdm_contactor_state[std::to_underlying(PdmContactor::motor2_bypass)] = tpdo.pdm_motor2_bypass; 
    pdm_contactor_state[std::to_underlying(PdmContactor::motor3_bypass)] = tpdo.pdm_motor3_bypass; 
    pdm_contactor_state[std::to_underlying(PdmContactor::motor4_bypass)] = tpdo.pdm_motor4_bypass; 
    pdm_contactor_state[std::to_underlying(PdmContactor::charge_mode)] = tpdo.pdm_charge_mode; 
    pdm_contactor_state[std::to_underlying(PdmContactor::charge_allow)] = tpdo.pdm_charge_allow; 
    pdm_contactor_state[std::to_underlying(PdmContactor::equip_bypass)] = tpdo.pdm_equip_bypass; 
}


void Server::_handle_tpdo3(const ucanopen::can_payload& payload) {
    static_assert(sizeof(CobTpdo3) == 8);

    CobTpdo3 tpdo = ucanopen::from_payload<CobTpdo3>(payload);
    size_t wheel = tpdo.wheel;
    MotorDriveData data{};

    data.errors = tpdo.controller_errors;
    data.ctlmode = (tpdo.ctlmode == 0) ? "speed" : "torque";
    data.enabled = tpdo.controller_enabled;

    switch (tpdo.discharge) {
    case 0:
        data.discharge = "charged";
        break;
    case 1: case 2:
        data.discharge = "discharged";
        break;
    case 3:
        data.discharge = "failed";
    }

    switch (tpdo.controller_fault_level) {
    case 0:
        data.faultlevel = "no error";
        break;
    case 1:
        data.faultlevel = "level 1";
        break;
    case 2:
        data.faultlevel = "level 2";
        break;
    case 3:
        data.faultlevel = "level 3";
        break;
    case 4:
        data.faultlevel = "level 4";
        break;
    default:
        data.faultlevel = "unknown";
        break;
    }

    data.faultcode = tpdo.fault_code;

    motordrive_data[wheel] = data;
}


ucanopen::can_payload Server::_create_rpdo2() {
    static_assert(sizeof(CobRpdo2) == 8);
    static unsigned int counter = 0;
    CobRpdo2 rpdo{};

    rpdo.pdm_battery_connect = pdm_contactor_ref_state[std::to_underlying(PdmContactor::battery_connect)];
    rpdo.pdm_motor1_bypass = pdm_contactor_ref_state[std::to_underlying(PdmContactor::motor1_bypass)];
    rpdo.pdm_motor2_bypass = pdm_contactor_ref_state[std::to_underlying(PdmContactor::motor2_bypass)];
    rpdo.pdm_motor3_bypass = pdm_contactor_ref_state[std::to_underlying(PdmContactor::motor3_bypass)];
    rpdo.pdm_motor4_bypass = pdm_contactor_ref_state[std::to_underlying(PdmContactor::motor4_bypass)];
    rpdo.pdm_charge_mode = pdm_contactor_ref_state[std::to_underlying(PdmContactor::charge_mode)];
    rpdo.pdm_charge_allow = pdm_contactor_ref_state[std::to_underlying(PdmContactor::charge_allow)];
    rpdo.pdm_equip_bypass = pdm_contactor_ref_state[std::to_underlying(PdmContactor::equip_bypass)];

    rpdo.counter = counter;
    counter = (counter + 1) % 4;

    return ucanopen::to_payload<CobRpdo2>(rpdo);
}


ucanopen::can_payload Server::_create_rpdo3() {
    static_assert(sizeof(CobRpdo3) == 8);
    static unsigned int counter = 0;
    static unsigned int wheel = 0;
    CobRpdo3 rpdo{};

    rpdo.ctlmode = motordrive_ref_ctlmode[wheel];
    rpdo.enable_controller = motordrive_ref_enable[wheel];
    rpdo.discharge = motordrive_ref_discharge[wheel];
    rpdo.mainrelay = motordrive_ref_mainrelay[wheel];
    rpdo.gear = motordrive_ref_gear[wheel];
    rpdo.footbrake = motordrive_ref_footbrake[wheel];
    rpdo.handbrake = motordrive_ref_handbrake[wheel];
    rpdo.fault_reset = motordrive_ref_faultreset[wheel];
    rpdo.speed_cmd = motordrive_ref_speed[wheel];
    rpdo.torque_cmd = motordrive_ref_torque[wheel];

    rpdo.wheel = wheel;
    wheel = (wheel + 1) % 4;

    rpdo.counter = counter;
    counter = (counter + 1) % 16;

    return ucanopen::to_payload<CobRpdo3>(rpdo);
}


} // namespace atvvcu
