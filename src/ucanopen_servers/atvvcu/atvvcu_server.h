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

    std::atomic<float> bms_voltage{0.0f};
    std::atomic<float> bms_charge_pct{0.0f};

    std::array<std::atomic_bool, pdm_contactor_count> pdm_contactor_state{};
    std::array<std::atomic_bool, pdm_contactor_count> pdm_contactor_ref_state{};

public:
    struct MotorDriveData {
        uint32_t errors;
        std::string_view ctlmode{"..."};
        bool enabled;
        std::string_view discharge{"..."};
        std::string_view faultlevel{"..."};
        uint32_t faultcode;
    };

    std::array<std::atomic<MotorDriveData>, 4> motordrive_data{};

    struct SystemData {
        std::string_view vcu_state{"..."};
        std::string_view vcu_opmode{"..."};
    };

    std::atomic<SystemData> system_data{};

    std::array<std::atomic_uint32_t, error_domain_count> errors{};
    std::array<std::atomic_uint16_t, error_domain_count> warnings{};

protected:
    void _handle_tpdo1(const ucanopen::can_payload& payload);
    void _handle_tpdo2(const ucanopen::can_payload& payload);
    void _handle_tpdo3(const ucanopen::can_payload& payload);
    void _handle_tpdo4(const ucanopen::can_payload& payload);

    ucanopen::can_payload _create_rpdo1();
    ucanopen::can_payload _create_rpdo2();
    ucanopen::can_payload _create_rpdo3();


    virtual ucanopen::FrameHandlingStatus handle_sdo(ucanopen::ODEntryIter entry,
                                                     ucanopen::SdoType sdo_type,
                                                     ucanopen::ExpeditedSdoData data) override;
public:
    Server(std::shared_ptr<can::Socket> socket, ucanopen::NodeId node_id, const std::string& name);

private:
    std::array<std::atomic_bool, 4> _motordrive_ref_ctlmode{};
    std::array<std::atomic_bool, 4> _motordrive_ref_enable{};
    std::array<std::atomic_bool, 4> _motordrive_ref_discharge{};
    std::array<std::atomic_bool, 4> _motordrive_ref_mainrelay{};
    std::array<std::atomic_bool, 4> _motordrive_ref_footbrake{};
    std::array<std::atomic_bool, 4> _motordrive_ref_handbrake{};
    std::array<std::atomic_bool, 4> _motordrive_ref_faultreset{};
    std::array<std::atomic_uint8_t, 4> _motordrive_ref_gear{};
    std::array<std::atomic_int16_t, 4> _motordrive_ref_speed{};
    std::array<std::atomic_int16_t, 4> _motordrive_ref_torque{};

public:
    void set_motordrive_ref_ctlmode(const std::array<int, 4>& ref) {
        std::copy(ref.begin(), ref.end(), _motordrive_ref_ctlmode.begin());
    }

    void set_motordrive_ref_enable(const std::array<bool, 4>& ref) {
        std::copy(ref.begin(), ref.end(), _motordrive_ref_enable.begin());
    }

    void set_motordrive_ref_discharge(const std::array<bool, 4>& ref) {
        std::copy(ref.begin(), ref.end(), _motordrive_ref_discharge.begin());
    }

    void set_motordrive_ref_mainrelay(const std::array<bool, 4>& ref) {
        std::copy(ref.begin(), ref.end(), _motordrive_ref_mainrelay.begin());
    }

    void set_motordrive_ref_footbrake(const std::array<bool, 4>& ref) {
        std::copy(ref.begin(), ref.end(), _motordrive_ref_footbrake.begin());
    }

    void set_motordrive_ref_handbrake(const std::array<bool, 4>& ref) {
        std::copy(ref.begin(), ref.end(), _motordrive_ref_handbrake.begin());
    }

    void set_motordrive_ref_faultreset(const std::array<bool, 4>& ref) {
        std::copy(ref.begin(), ref.end(), _motordrive_ref_faultreset.begin());
    }

    void set_motordrive_ref_gear(const std::array<int, 4>& ref) {
        std::copy(ref.begin(), ref.end(), _motordrive_ref_gear.begin());
    }

    void set_motordrive_ref_speed(const std::array<float, 4>& ref) {
        for (auto i = 0uz; i < ref.size(); ++i) {
            _motordrive_ref_speed[i] = std::clamp(ref[i], -10000.0f, 10000.0f);
        }
    }

    void set_motordrive_ref_torque(const std::array<float, 4>& ref) {
        for (auto i = 0uz; i < ref.size(); ++i) {
            _motordrive_ref_torque[i] = std::clamp(ref[i] * 10.0f, -4000.0f, 4000.0f);
        }
    }
};


} // namespace atvvcu
