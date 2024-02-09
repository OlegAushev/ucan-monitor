#include "atvvcu_server.h"
#include "ucanopen/ucanopen_def.h"
#include "ucanopen_servers/atvvcu/atvvcu_def.h"
#include <utility>


namespace atvvcu {

Server::Server(std::shared_ptr<can::Socket> socket, ucanopen::NodeId node_id, const std::string& name)
        : ucanopen::Server(socket, node_id, name, object_dictionary)
        , ucanopen::SdoSubscriber(sdo_service)
{
    tpdo_service.register_tpdo(ucanopen::CobTpdo::tpdo1, std::chrono::milliseconds(1100),
            [this](ucanopen::can_payload payload) { this->_handle_tpdo1(payload); });
    tpdo_service.register_tpdo(ucanopen::CobTpdo::tpdo2, std::chrono::milliseconds(1100),
            [this](ucanopen::can_payload payload) { this->_handle_tpdo2(payload); });
    tpdo_service.register_tpdo(ucanopen::CobTpdo::tpdo3, std::chrono::milliseconds(1100),
            [this](ucanopen::can_payload payload) { this->_handle_tpdo3(payload); });
    tpdo_service.register_tpdo(ucanopen::CobTpdo::tpdo4, std::chrono::milliseconds(1100),
            [this](ucanopen::can_payload payload) { this->_handle_tpdo4(payload); });

    rpdo_service.register_rpdo(ucanopen::CobRpdo::rpdo1, std::chrono::milliseconds(100),
            [this](){ return this->_create_rpdo1(); });
    rpdo_service.register_rpdo(ucanopen::CobRpdo::rpdo2, std::chrono::milliseconds(100),
            [this](){ return this->_create_rpdo2(); });
    rpdo_service.register_rpdo(ucanopen::CobRpdo::rpdo3, std::chrono::milliseconds(50),
            [this](){ return this->_create_rpdo3(); });
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


void Server::_handle_tpdo1(const ucanopen::can_payload& payload) {
    static_assert(sizeof(CobTpdo1) == 8);
    CobTpdo1 tpdo = ucanopen::from_payload<CobTpdo1>(payload);

    SystemData data_{};
    data_.vcu_state = (tpdo.vcu_state < vcu_states.size()) ? std::string_view(vcu_states[tpdo.vcu_state]) : "unknown";
    data_.vcu_opmode = (tpdo.vcu_opmode < vcu_opmodes.size()) ? std::string_view(vcu_opmodes[tpdo.vcu_opmode]) : "unknown";

    system_data = data_;
}


void Server::_handle_tpdo2(const ucanopen::can_payload& payload) {
    static_assert(sizeof(CobTpdo2) == 8);
    CobTpdo2 tpdo = ucanopen::from_payload<CobTpdo2>(payload);

    pdm_contactor_state[std::to_underlying(PdmContactor::battery_p)] = tpdo.pdm_battery_p; 
    pdm_contactor_state[std::to_underlying(PdmContactor::battery_n)] = tpdo.pdm_battery_n; 
    pdm_contactor_state[std::to_underlying(PdmContactor::front_bypass)] = tpdo.pdm_front_bypass; 
    pdm_contactor_state[std::to_underlying(PdmContactor::back_bypass)] = tpdo.pdm_back_bypass; 
    pdm_contactor_state[std::to_underlying(PdmContactor::aux_bypass)] = tpdo.pdm_aux_bypass; 
    pdm_contactor_state[std::to_underlying(PdmContactor::charge_allow)] = tpdo.pdm_charge_allow; 
    pdm_contactor_state[std::to_underlying(PdmContactor::charge_mode)] = tpdo.pdm_charge_mode;

    bms_voltage = uint16_t(tpdo.bms_voltage) / 10.0f;
    bms_charge_pct = uint8_t(tpdo.bms_charge_pct);
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

    switch (tpdo.controller_faultlevel) {
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

    data.faultcode = tpdo.faultcode;

    motordrive_data[wheel] = data;
}


void Server::_handle_tpdo4(const ucanopen::can_payload& payload) {
    static_assert(sizeof(CobTpdo4) == 8);

    CobTpdo4 tpdo = ucanopen::from_payload<CobTpdo4>(payload);

    size_t domain = tpdo.domain;
    if (domain >= error_domain_count) {
        return;
    }

    errors[domain] = tpdo.errors;
    warnings[domain] = tpdo.warnings;
}


ucanopen::can_payload Server::_create_rpdo1() {
    static_assert(sizeof(CobRpdo1) == 8);
    static unsigned int counter = 0;
    
    CobRpdo1 rpdo{};
    rpdo.opmode = std::to_underlying(vcu_opmode.load());
    rpdo.power = power_enabled;
    rpdo.run = run_enabled;

    //rpdo.counter = counter;
    counter = (counter + 1) % 4;

    return ucanopen::to_payload<CobRpdo1>(rpdo);
}


ucanopen::can_payload Server::_create_rpdo2() {
    static_assert(sizeof(CobRpdo2) == 8);
    static unsigned int counter = 0;
    CobRpdo2 rpdo{};

    rpdo.pdm_battery_p = pdm_contactor_ref_state[std::to_underlying(PdmContactor::battery_p)];
    rpdo.pdm_battery_n = pdm_contactor_ref_state[std::to_underlying(PdmContactor::battery_n)];
    rpdo.pdm_front_bypass = pdm_contactor_ref_state[std::to_underlying(PdmContactor::front_bypass)];
    rpdo.pdm_back_bypass = pdm_contactor_ref_state[std::to_underlying(PdmContactor::back_bypass)];
    rpdo.pdm_aux_bypass = pdm_contactor_ref_state[std::to_underlying(PdmContactor::aux_bypass)];
    rpdo.pdm_charge_allow = pdm_contactor_ref_state[std::to_underlying(PdmContactor::charge_allow)];
    rpdo.pdm_charge_mode = pdm_contactor_ref_state[std::to_underlying(PdmContactor::charge_mode)];

    rpdo.counter = counter;
    counter = (counter + 1) % 4;

    return ucanopen::to_payload<CobRpdo2>(rpdo);
}


ucanopen::can_payload Server::_create_rpdo3() {
    static_assert(sizeof(CobRpdo3) == 8);
    static unsigned int counter = 0;
    static unsigned int wheel = 0;
    CobRpdo3 rpdo{};

    rpdo.ctlmode = _motordrive_ref_ctlmode[wheel];
    rpdo.enable_controller = _motordrive_ref_enable[wheel];
    rpdo.discharge = _motordrive_ref_discharge[wheel];
    rpdo.mainrelay = _motordrive_ref_mainrelay[wheel];
    rpdo.gear = _motordrive_ref_gear[wheel];
    rpdo.footbrake = _motordrive_ref_footbrake[wheel];
    rpdo.handbrake = _motordrive_ref_handbrake[wheel];
    rpdo.faultreset = _motordrive_ref_faultreset[wheel];
    rpdo.speed_cmd = _motordrive_ref_speed[wheel];
    rpdo.torque_cmd = _motordrive_ref_torque[wheel];

    rpdo.wheel = wheel;
    wheel = (wheel + 1) % 4;

    rpdo.counter = counter;
    counter = (counter + 1) % 16;

    return ucanopen::to_payload<CobRpdo3>(rpdo);
}


} // namespace atvvcu
