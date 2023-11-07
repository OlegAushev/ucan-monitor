#pragma once


#include "ucanopen/server/server.h"
#include "atvvcu_def.h"
#include "bsclog/bsclog.h"
#include <atomic>


namespace atvvcu {

extern const ucanopen::ObjectDictionary object_dictionary;
extern const ucanopen::ObjectDictionaryConfig object_dictionary_config;


class Server : public ucanopen::Server, public ucanopen::SdoSubscriber {
private:
    std::array<std::atomic_bool, pdm_contactor_count> _pdm_contactor_states = {};
    std::array<std::atomic_bool, pdm_contactor_count> _pdm_contactor_refstates = {};


    std::array<float, 4> _client_values;
    std::array<float, 4> _server_values;

    uint32_t _errors = 0;
    uint16_t _warnings = 0;
protected:
    void _handle_tpdo1(const ucanopen::can_payload& payload);
    void _handle_tpdo2([[maybe_unused]] const ucanopen::can_payload& payload) {}
    void _handle_tpdo3([[maybe_unused]] const ucanopen::can_payload& payload) {}
    void _handle_tpdo4([[maybe_unused]] const ucanopen::can_payload& payload) {};

    ucanopen::can_payload _create_rpdo1();

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

    void set_pdm_contactor_states(const std::array<bool, pdm_contactor_count>& states) {
        std::copy(states.begin(), states.end(), _pdm_contactor_refstates.begin());
    }

    std::array<bool, pdm_contactor_count> pdm_contactor_states() const {
        std::array<bool, pdm_contactor_count> ret;
        std::copy(_pdm_contactor_states.begin(), _pdm_contactor_states.end(), ret.begin());
        return ret;
    }

    void set_client_value(ucanopen::CobTpdo tpdo_type, double value) { _client_values[static_cast<size_t>(tpdo_type)] = value; }
    void set_server_value(ucanopen::CobRpdo rpdo_type, double value) { _server_values[static_cast<size_t>(rpdo_type)] = value; }

    uint32_t errors() const { return _errors; }
    uint16_t warnings() const { return _warnings; }

    //bool pdm_contactor_state(PdmContactor contactor) const { return _pdm_contactor_state[std::to_underlying(contactor)]; }
};

} // namespace atvvcu

