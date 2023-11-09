#include "atvvcu_server.h"
#include "ucanopen_servers/atvvcu/atvvcu_def.h"
#include <utility>


namespace atvvcu {

Server::Server(std::shared_ptr<can::Socket> socket, ucanopen::NodeId node_id, const std::string& name)
        : ucanopen::Server(socket, node_id, name, object_dictionary)
        , ucanopen::SdoSubscriber(sdo_service)
{
    tpdo_service.register_tpdo(ucanopen::CobTpdo::tpdo1, std::chrono::milliseconds(1100),
            [this](ucanopen::can_payload payload) { this->_handle_tpdo1(payload); });
    // tpdo_service.register_tpdo(ucanopen::CobTpdo::tpdo2, std::chrono::milliseconds(1100),
    //         [this](ucanopen::can_payload payload) { this->_handle_tpdo2(payload); });
    // tpdo_service.register_tpdo(ucanopen::CobTpdo::tpdo3, std::chrono::milliseconds(1100),
    //         [this](ucanopen::can_payload payload) { this->_handle_tpdo3(payload); });
    // tpdo_service.register_tpdo(ucanopen::CobTpdo::tpdo4, std::chrono::milliseconds(1100),
    //         [this](ucanopen::can_payload payload) { this->_handle_tpdo4(payload); });

    rpdo_service.register_rpdo(ucanopen::CobRpdo::rpdo1, std::chrono::milliseconds(100),
            [this](){ return this->_create_rpdo1(); });
    rpdo_service.register_rpdo(ucanopen::CobRpdo::rpdo2, std::chrono::milliseconds(50),
            [this](){ return this->_create_rpdo2(); });
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


void Server::_handle_tpdo1([[maybe_unused]] const ucanopen::can_payload& payload) {
    static_assert(sizeof(CobTpdo1) == 8);
    CobTpdo1 tpdo = ucanopen::from_payload<CobTpdo1>(payload);

    _pdm_contactor_states[std::to_underlying(PdmContactor::battery_connect)] = tpdo.pdm_battery_connect; 
    _pdm_contactor_states[std::to_underlying(PdmContactor::motor1_bypass)] = tpdo.pdm_motor1_bypass; 
    _pdm_contactor_states[std::to_underlying(PdmContactor::motor2_bypass)] = tpdo.pdm_motor2_bypass; 
    _pdm_contactor_states[std::to_underlying(PdmContactor::motor3_bypass)] = tpdo.pdm_motor3_bypass; 
    _pdm_contactor_states[std::to_underlying(PdmContactor::motor4_bypass)] = tpdo.pdm_motor4_bypass; 
    _pdm_contactor_states[std::to_underlying(PdmContactor::charge_mode)] = tpdo.pdm_charge_mode; 
    _pdm_contactor_states[std::to_underlying(PdmContactor::charge_allow)] = tpdo.pdm_charge_allow; 
    _pdm_contactor_states[std::to_underlying(PdmContactor::equip_bypass)] = tpdo.pdm_equip_bypass; 
}


ucanopen::can_payload Server::_create_rpdo1() {
    static_assert(sizeof(CobRpdo1) == 8);
    static unsigned int counter = 0;
    CobRpdo1 rpdo{};

    rpdo.pdm_battery_connect = _pdm_contactor_refstates[std::to_underlying(PdmContactor::battery_connect)];
    rpdo.pdm_motor1_bypass = _pdm_contactor_refstates[std::to_underlying(PdmContactor::motor1_bypass)];
    rpdo.pdm_motor2_bypass = _pdm_contactor_refstates[std::to_underlying(PdmContactor::motor2_bypass)];
    rpdo.pdm_motor3_bypass = _pdm_contactor_refstates[std::to_underlying(PdmContactor::motor3_bypass)];
    rpdo.pdm_motor4_bypass = _pdm_contactor_refstates[std::to_underlying(PdmContactor::motor4_bypass)];
    rpdo.pdm_charge_mode = _pdm_contactor_refstates[std::to_underlying(PdmContactor::charge_mode)];
    rpdo.pdm_charge_allow = _pdm_contactor_refstates[std::to_underlying(PdmContactor::charge_allow)];
    rpdo.pdm_equip_bypass = _pdm_contactor_refstates[std::to_underlying(PdmContactor::equip_bypass)];

    rpdo.counter = counter;
    counter = (counter + 1) % 4;

    return ucanopen::to_payload<CobRpdo1>(rpdo);
}


ucanopen::can_payload Server::_create_rpdo2() {
    static_assert(sizeof(CobRpdo2) == 8);
    static unsigned int counter = 0;
    static unsigned int wheel = 0;
    CobRpdo2 rpdo{};

    rpdo.Ctrl_Mode = motordrive_refmode[wheel];
    rpdo.Enable = motordrive_refstatus[wheel];
    rpdo.Discharge_Enable = motordrive_refdischarge[wheel];
    rpdo.Main_Relay_State = motordrive_refrelay[wheel];
    rpdo.Gear_State = motordrive_refgear[wheel];
    rpdo.Brake_State = motordrive_reffootbrake[wheel];
    rpdo.Handbrake_State = motordrive_refhandbrake[wheel];
    rpdo.Fault_Reset = motordrive_reffaultreset[wheel];
    rpdo.Speed_Cmd = motordrive_refspeed[wheel];
    rpdo.Trq_Cmd = motordrive_reftorque[wheel];

    rpdo.wheel = wheel;
    wheel = (wheel + 1) % 4;

    rpdo.counter = counter;
    counter = (counter + 1) % 16;

    return ucanopen::to_payload<CobRpdo2>(rpdo);
}


} // namespace atvvcu
