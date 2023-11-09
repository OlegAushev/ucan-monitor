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

public:
    std::array<std::atomic_bool, pdm_contactor_count> pdm_contactor_state{};
    std::array<std::atomic_bool, pdm_contactor_count> pdm_contactor_ref_state{};

    std::array<std::atomic_bool, 4> motordrive_ref_ctlmode{};
    std::array<std::atomic_bool, 4> motordrive_ref_enable{};
    std::array<std::atomic_bool, 4> motordrive_ref_discharge{};
    std::array<std::atomic_bool, 4> motordrive_ref_mainrelay{};
    std::array<std::atomic_bool, 4> motordrive_ref_footbrake{};
    std::array<std::atomic_bool, 4> motordrive_ref_handbrake{};
    std::array<std::atomic_bool, 4> motordrive_ref_faultreset{};
    std::array<std::atomic_uint8_t, 4> motordrive_ref_gear{};
    std::array<std::atomic_int16_t, 4> motordrive_ref_speed{};
    std::array<std::atomic_int16_t, 4> motordrive_ref_torque{};
protected:
    void _handle_tpdo1(const ucanopen::can_payload& payload);
    void _handle_tpdo2([[maybe_unused]] const ucanopen::can_payload& payload) {}

    ucanopen::can_payload _create_rpdo1();
    ucanopen::can_payload _create_rpdo2();


    virtual ucanopen::FrameHandlingStatus handle_sdo(ucanopen::ODEntryIter entry,
                                                     ucanopen::SdoType sdo_type,
                                                     ucanopen::ExpeditedSdoData data) override;
public:
    Server(std::shared_ptr<can::Socket> socket, ucanopen::NodeId node_id, const std::string& name);

    // void set_pdm_contactor_states(const std::array<bool, pdm_contactor_count>& states) {
    //     std::copy(states.begin(), states.end(), pdm_contactor_ref_state.begin());
    // }

    // std::array<bool, pdm_contactor_count> pdm_contactor_states() const {
    //     std::array<bool, pdm_contactor_count> ret;
    //     std::copy(pdm_contactor_state.begin(), pdm_contactor_state.end(), ret.begin());
    //     return ret;
    // }
};

} // namespace atvvcu

