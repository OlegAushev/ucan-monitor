#pragma once


#include "ucanopen/server/server.h"
#include "atvvcu_def.h"
#include "bsclog/bsclog.h"
#include <array>
#include <atomic>
#include <cstdint>
#include <string_view>


namespace atvvcu {


extern const ucanopen::ObjectDictionary object_dictionary;
extern const ucanopen::ObjectDictionaryConfig object_dictionary_config;


class Server : public ucanopen::Server, public ucanopen::SdoSubscriber {
private:

public:
    std::atomic<VcuOperationMode> vcu_opmode{VcuOperationMode::normal};
    std::atomic<bool> power_enabled{false};
    std::atomic<bool> run_enabled{false};

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

    struct MotorDriveData {
        uint32_t errors;
        std::string_view ctlmode;
        bool enabled;
        std::string_view discharge;
        std::string_view faultlevel;
        uint32_t faultcode;
    };

    std::array<std::atomic<MotorDriveData>, 4> motordrive_data;

    struct SystemData {
        std::string_view vcu_state;
        std::string_view vcu_opmode;
    };

    std::atomic<SystemData> system_data;

protected:
    void _handle_tpdo1(const ucanopen::can_payload& payload);
    void _handle_tpdo2(const ucanopen::can_payload& payload);
    void _handle_tpdo3(const ucanopen::can_payload& payload);

    ucanopen::can_payload _create_rpdo1();
    ucanopen::can_payload _create_rpdo2();
    ucanopen::can_payload _create_rpdo3();


    virtual ucanopen::FrameHandlingStatus handle_sdo(ucanopen::ODEntryIter entry,
                                                     ucanopen::SdoType sdo_type,
                                                     ucanopen::ExpeditedSdoData data) override;
public:
    Server(std::shared_ptr<can::Socket> socket, ucanopen::NodeId node_id, const std::string& name);
};


} // namespace atvvcu
