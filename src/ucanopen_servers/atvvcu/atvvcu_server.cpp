#include "atvvcu_server.h"


namespace atvvcu {

Server::Server(std::shared_ptr<can::Socket> socket, ucanopen::NodeId node_id, const std::string& name)
        : ucanopen::Server(socket, node_id, name, object_dictionary)
        , ucanopen::SdoSubscriber(sdo_service) {
    _client_values.fill(0);
    _server_values.fill(0);

    tpdo_service.register_tpdo(ucanopen::CobTpdo::tpdo1, std::chrono::milliseconds(1100),
            [this](ucanopen::can_payload payload) { this->_handle_tpdo1(payload); });
    tpdo_service.register_tpdo(ucanopen::CobTpdo::tpdo2, std::chrono::milliseconds(1100),
            [this](ucanopen::can_payload payload) { this->_handle_tpdo2(payload); });
    tpdo_service.register_tpdo(ucanopen::CobTpdo::tpdo3, std::chrono::milliseconds(1100),
            [this](ucanopen::can_payload payload) { this->_handle_tpdo3(payload); });
    tpdo_service.register_tpdo(ucanopen::CobTpdo::tpdo4, std::chrono::milliseconds(1100),
            [this](ucanopen::can_payload payload) { this->_handle_tpdo4(payload); });

    // rpdo_service.register_rpdo(ucanopen::CobRpdo::rpdo1, std::chrono::milliseconds(25),
    //         [this](){ return this->_create_rpdo1(); });
    // rpdo_service.register_rpdo(ucanopen::CobRpdo::rpdo2, std::chrono::milliseconds(50),
    //         [this](){ return this->_create_rpdo2(); });
    // rpdo_service.register_rpdo(ucanopen::CobRpdo::rpdo3, std::chrono::milliseconds(100),
    //         [this](){ return this->_create_rpdo3(); });
    // rpdo_service.register_rpdo(ucanopen::CobRpdo::rpdo4, std::chrono::milliseconds(500),
    //         [this](){ return this->_create_rpdo4(); });
}


void Server::_handle_tpdo4(const ucanopen::can_payload& payload) {
    //[[maybe_unused]] CobTpdo4 message = ucanopen::from_payload<CobTpdo4>(payload);
    //_errors = message.errors;
    //_warnings = message.warnings;
}


ucanopen::FrameHandlingStatus Server::handle_sdo(ucanopen::ODEntryIter entry,
                                                 [[maybe_unused]] ucanopen::SdoType sdo_type,
                                                 ucanopen::ExpeditedSdoData data) {
    if (entry->second.name == "syslog_message") {
        auto message_id = data.u32();
        if ((message_id != 0) && (message_id < syslog_messages.size())) {
            bsclog::info("{}", syslog_messages[message_id]);
        }
    } else if (entry->second.category == _dictionary.config.config_category && entry->second.type == ucanopen::OD_ENUM16) {
        
    } else if (entry->second.name == "pdm_contactor_state") {
        for (auto i = 0; i < pdm_contactor_count; ++i) {
            std::lock_guard<std::mutex> lock(_mtx);
            _pdm_contactor_states[i] = data.u32() & (1 << i);
        }
    }

    return ucanopen::FrameHandlingStatus::success;
}


ucanopen::can_payload Server::_create_rpdo1() {
    static unsigned int counter = 0;
    CobRpdo1 message;
    message.pdm_battery_connect = _pdm_contactor_states[0];
    message.pdm_motor1_bypass = _pdm_contactor_states[1];
    message.pdm_motor2_bypass = _pdm_contactor_states[2];
    message.pdm_motor3_bypass = _pdm_contactor_states[3];
    message.pdm_motor4_bypass = _pdm_contactor_states[4];
    message.pdm_charge_mode = _pdm_contactor_states[5];
    message.pdm_charge_allow = _pdm_contactor_states[6];
    message.pdm_equip_bypass = _pdm_contactor_states[7];
    
    //{.counter = counter, ._reserved = 0, .value = _server_values[0]};
    counter = (counter + 1) % 4;
    return ucanopen::to_payload<CobRpdo1>(message);
}


} // namespace atvvcu
