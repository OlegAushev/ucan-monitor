#pragma once


#include "ucanopen/server/server.h"
#include "launchpad_def.h"
#include "log/log.h"


namespace launchpad {

extern const ucanopen::ObjectDictionary object_dictionary;
extern const ucanopen::ObjectDictionaryConfig object_dictionary_config;


class Server : public ucanopen::Server, public ucanopen::SdoSubscriber {
private:
    std::array<float, 4> _client_values;
    std::array<float, 4> _server_values;

    uint32_t _errors = 0;
    uint16_t _warnings = 0;
protected:
    void _handle_tpdo1([[maybe_unused]] const ucanopen::can_payload& payload) {}
    void _handle_tpdo2([[maybe_unused]] const ucanopen::can_payload& payload) {}
    void _handle_tpdo3([[maybe_unused]] const ucanopen::can_payload& payload) {}
    void _handle_tpdo4(const ucanopen::can_payload& payload);

    ucanopen::can_payload _create_rpdo1() {
        static unsigned int counter = 0;
        CobRpdo1 message{.counter = counter, ._reserved = 0, .value = _server_values[0]};
        counter = (counter + 1) % 4;
        return ucanopen::to_payload<CobRpdo1>(message);
    }

    ucanopen::can_payload _create_rpdo2() {
        static unsigned int counter = 0;
        CobRpdo2 message{.counter = counter, ._reserved = 0, .value = _server_values[1]};
        counter = (counter + 1) % 4;
        return ucanopen::to_payload<CobRpdo2>(message);
    }

    ucanopen::can_payload _create_rpdo3() {
        static unsigned int counter = 0;
        CobRpdo3 message{.counter = counter, ._reserved = 0, .value = _server_values[2]};
        counter = (counter + 1) % 4;
        return ucanopen::to_payload<CobRpdo3>(message);
    }

    ucanopen::can_payload _create_rpdo4() {
        static unsigned int counter = 0;
        CobRpdo4 message{.counter = counter, ._reserved = 0, .value = _server_values[3]};
        counter = (counter + 1) % 4;
        return ucanopen::to_payload<CobRpdo4>(message);
    }

    virtual ucanopen::FrameHandlingStatus handle_sdo(ucanopen::ODEntryIter entry,
                                                     ucanopen::SdoType sdo_type,
                                                     ucanopen::ExpeditedSdoData data) override;
public:
    Server(std::shared_ptr<can::Socket> socket, ucanopen::NodeId node_id, const std::string& name);

    void set_client_value(ucanopen::TpdoType tpdo_type, double value) { _client_values[static_cast<size_t>(tpdo_type)] = value; }
    void set_server_value(ucanopen::RpdoType rpdo_type, double value) { _server_values[static_cast<size_t>(rpdo_type)] = value; }

    uint32_t errors() const { return _errors; }
    uint16_t warnings() const { return _warnings; }

    ucanopen::can_payload create_client_tpdo1() {
        static unsigned int counter = 0;
        CobClientTpdo1 message{.counter = counter, ._reserved = 0, .value = _client_values[0]};
        counter = (counter + 1) % 4;
        return ucanopen::to_payload<CobClientTpdo1>(message);
    }

    ucanopen::can_payload create_client_tpdo2() {
        static unsigned int counter = 0;
        CobClientTpdo2 message{.counter = counter, ._reserved = 0, .value = _client_values[1]};
        counter = (counter + 1) % 4;
        return ucanopen::to_payload<CobClientTpdo2>(message);
    }

    ucanopen::can_payload create_client_tpdo3() {
        static unsigned int counter = 0;
        CobClientTpdo3 message{.counter = counter, ._reserved = 0, .value = _client_values[2]};
        counter = (counter + 1) % 4;
        return ucanopen::to_payload<CobClientTpdo3>(message);
    }

    ucanopen::can_payload create_client_tpdo4() {
        static unsigned int counter = 0;
        CobClientTpdo4 message{.counter = counter, ._reserved = 0, .value = _client_values[3]};
        counter = (counter + 1) % 4;
        return ucanopen::to_payload<CobClientTpdo4>(message);
    }
};

} // namespace launchpad

