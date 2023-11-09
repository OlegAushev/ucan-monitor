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
    std::array<std::atomic_bool, pdm_contactor_count> _pdm_contactor_states{};
    std::array<std::atomic_bool, pdm_contactor_count> _pdm_contactor_refstates{};

public:
    std::array<std::atomic_bool, 4> motordrive_refmode{};
    std::array<std::atomic_bool, 4> motordrive_refstatus{};
    std::array<std::atomic_bool, 4> motordrive_refdischarge{};
    std::array<std::atomic_bool, 4> motordrive_refrelay{};
    std::array<std::atomic_bool, 4> motordrive_reffootbrake{};
    std::array<std::atomic_bool, 4> motordrive_refhandbrake{};
    std::array<std::atomic_bool, 4> motordrive_reffaultreset{};
    std::array<std::atomic_uint8_t, 4> motordrive_refgear{};
    std::array<std::atomic_int16_t, 4> motordrive_refspeed{};
    std::array<std::atomic_int16_t, 4> motordrive_reftorque{};


    uint32_t _errors = 0;
    uint16_t _warnings = 0;
protected:
    void _handle_tpdo1(const ucanopen::can_payload& payload);
    void _handle_tpdo2([[maybe_unused]] const ucanopen::can_payload& payload) {}
    void _handle_tpdo3([[maybe_unused]] const ucanopen::can_payload& payload) {}
    void _handle_tpdo4([[maybe_unused]] const ucanopen::can_payload& payload) {};

    ucanopen::can_payload _create_rpdo1();

    ucanopen::can_payload _create_rpdo2();

    ucanopen::can_payload _create_rpdo3() {
        return {};
    }

    ucanopen::can_payload _create_rpdo4() {
        return {};
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



    uint32_t errors() const { return _errors; }
    uint16_t warnings() const { return _warnings; }

    //bool pdm_contactor_state(PdmContactor contactor) const { return _pdm_contactor_state[std::to_underlying(contactor)]; }
};

} // namespace atvvcu

