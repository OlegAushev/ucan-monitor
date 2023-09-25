#include "bmsmain_server.h"


namespace bmsmain {

Server::Server(std::shared_ptr<can::Socket> socket, ucanopen::NodeId node_id, const std::string& name)
        : ucanopen::Server(socket, node_id, name, ucanopen::ObjectDictionary{}) {
    tpdo_service.register_tpdo(ucanopen::TpdoType::tpdo1, std::chrono::milliseconds(2000),
            [this](ucanopen::can_payload payload) { this->_handle_tpdo1(payload); });
}


void Server::_handle_tpdo1(const ucanopen::can_payload& payload) {
    static_assert(sizeof(CobTpdo1) == 8);
    CobTpdo1 message = ucanopen::from_payload<CobTpdo1>(payload);

    int16_t current = message.current;
    _current = 0.1 * current;

    int8_t temp_min = message.temp_min;
    _temp_min = temp_min;

    int8_t temp_max = message.temp_max;
    _temp_max = temp_max;

    _charge = message.charge_percentage;
    _voltage = 0.1 * message.voltage;
}

}

