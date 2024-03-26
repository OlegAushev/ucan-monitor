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


void Server::_handle_tpdo1(const ucanopen::can_payload& payload) {
    static_assert(sizeof(CobTpdo1) == 8);
    CobTpdo1 tpdo = ucanopen::from_payload<CobTpdo1>(payload);

    _vcu_state.store((tpdo.vcu_state < vcu_states.size()) ? std::string_view(vcu_states[tpdo.vcu_state]) : "unknown");
    _vcu_debug_mode.store(tpdo.vcu_dbg);

    dash._debug_mode.store(tpdo.dash_dbg);
    dash._remote_control.store(tpdo.remote_control);
    dash._emergency.store(tpdo.emergency);
    dash._faultreset.store(tpdo.faultreset);
    dash._power_enabled.store(tpdo.power);
    dash._run_enabled.store(tpdo.run);
    
    pdm._debug_mode.store(tpdo.pdm_dbg);
    pdm._contactor_feedback_state[std::to_underlying(pdm::Contactor::battery_p)].store(tpdo.pdm_battery_p);
    pdm._contactor_feedback_state[std::to_underlying(pdm::Contactor::battery_n)].store(tpdo.pdm_battery_n);
    pdm._contactor_feedback_state[std::to_underlying(pdm::Contactor::front_bypass)].store(tpdo.pdm_front_bypass);
    pdm._contactor_feedback_state[std::to_underlying(pdm::Contactor::back_bypass)].store(tpdo.pdm_back_bypass);
    pdm._contactor_feedback_state[std::to_underlying(pdm::Contactor::aux_bypass)].store(tpdo.pdm_aux_bypass);
    pdm._contactor_feedback_state[std::to_underlying(pdm::Contactor::charge_allow)].store(tpdo.pdm_charge_allow);
    pdm._contactor_feedback_state[std::to_underlying(pdm::Contactor::charge_mode)].store(tpdo.pdm_charge_mode);

    drive._debug_mode.store(tpdo.drive_dbg);

    gear_selector._debug_mode.store(tpdo.gear_dbg);
    if (tpdo.gear <= gear::gears.size()) { gear_selector._gear.store(gear::Gear(tpdo.gear)); }

    accl_pedal._debug_mode.store(tpdo.accl_dbg);
    accl_pedal._pressure.store(tpdo.accl / 100.0f);

    brakes._debug_mode.store(tpdo.brake_dbg);
    brakes._left_pressed.store(tpdo.brake_left);
    brakes._right_pressed.store(tpdo.brake_right);
}


void Server::_handle_tpdo2(const ucanopen::can_payload& payload) {
    static_assert(sizeof(CobTpdo2) == 8);
    CobTpdo2 tpdo = ucanopen::from_payload<CobTpdo2>(payload);

    bms._voltage = uint16_t(tpdo.bms_voltage) / 10.0f;
    bms._charge_pct = uint8_t(tpdo.bms_charge_pct);
}


void Server::_handle_tpdo3(const ucanopen::can_payload& payload) {
    static_assert(sizeof(CobTpdo3) == 8);
    CobTpdo3 tpdo = ucanopen::from_payload<CobTpdo3>(payload);

    size_t wheel = tpdo.wheel;
    drive::Controller::Data data{};

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

    drive._data[wheel] = data;
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

    rpdo.dash_dbg = dash._ref_debug_mode;
    rpdo.emergency = dash._ref_emergency;
    rpdo.faultreset = dash._ref_faultreset;
    rpdo.power = dash._ref_power_enabled;
    rpdo.run = dash._ref_run_enabled;

    rpdo.pdm_dbg = pdm._ref_debug_mode;
    rpdo.pdm_battery_p = pdm._contactor_ref_state[std::to_underlying(pdm::Contactor::battery_p)];
    rpdo.pdm_battery_n = pdm._contactor_ref_state[std::to_underlying(pdm::Contactor::battery_n)];
    rpdo.pdm_front_bypass = pdm._contactor_ref_state[std::to_underlying(pdm::Contactor::front_bypass)];
    rpdo.pdm_back_bypass = pdm._contactor_ref_state[std::to_underlying(pdm::Contactor::back_bypass)];
    rpdo.pdm_aux_bypass = pdm._contactor_ref_state[std::to_underlying(pdm::Contactor::aux_bypass)];
    rpdo.pdm_charge_allow = pdm._contactor_ref_state[std::to_underlying(pdm::Contactor::charge_allow)];
    rpdo.pdm_charge_mode = pdm._contactor_ref_state[std::to_underlying(pdm::Contactor::charge_mode)];

    rpdo.drive_dbg = drive._ref_debug_mode;

    rpdo.gear_dbg = gear_selector._ref_debug_mode;
    rpdo.gear = std::to_underlying(gear_selector._ref_gear.load());

    rpdo.accl_dbg = accl_pedal._ref_debug_mode;
    rpdo.accl = accl_pedal._ref_pressure * 100.f;

    rpdo.brake_dbg = brakes._ref_debug_mode;
    rpdo.brake_left = brakes._ref_left_pressed;
    rpdo.brake_right = brakes._ref_right_pressed;

    rpdo.counter = counter;
    counter = (counter + 1) % 4;

    return ucanopen::to_payload<CobRpdo1>(rpdo);
}


ucanopen::can_payload Server::_create_rpdo2() {
    static_assert(sizeof(CobRpdo2) == 8);
    static unsigned int counter = 0;
    static unsigned int wheel = 0;
    CobRpdo2 rpdo{};

    rpdo.ctlmode = drive._ref_ctlmode[wheel];
    rpdo.enable_controller = drive._ref_enable[wheel];
    rpdo.discharge = drive._ref_discharge[wheel];
    rpdo.mainrelay = drive._ref_mainrelay[wheel];
    rpdo.gear = drive._ref_gear[wheel];
    rpdo.footbrake = drive._ref_footbrake[wheel];
    rpdo.handbrake = drive._ref_handbrake[wheel];
    rpdo.faultreset = drive._ref_faultreset[wheel];
    rpdo.speed_cmd = drive._ref_speed[wheel];
    rpdo.torque_cmd = drive._ref_torque[wheel];

    rpdo.wheel = wheel;
    wheel = (wheel + 1) % 4;

    rpdo.counter = counter;
    counter = (counter + 1) % 16;

    return ucanopen::to_payload<CobRpdo2>(rpdo);
}


} // namespace atvvcu
