#pragma once


#include "ucanopen/server/server.h"
#include "atvvcu_def.h"
#include "bsclog/bsclog.h"
#include <array>
#include <atomic>
#include <cstdint>
#include <string_view>

#include "util/pdm.h"
#include "util/drive.h"
#include "util/bms.h"
#include "util/accl.h"
#include "util/gear.h"


namespace atvvcu {


extern const ucanopen::ObjectDictionary object_dictionary;
extern const ucanopen::ObjectDictionaryConfig object_dictionary_config;


class Server : public ucanopen::Server, public ucanopen::SdoSubscriber {
public:
    bms::Controller bms;
    pdm::Controller pdm;
    drive::Controller drive;
    gear::GearSelector gear_selector;
    accl::AcceleratorPedal accl_pedal;

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
    //ucanopen::can_payload _create_rpdo3();


    virtual ucanopen::FrameHandlingStatus handle_sdo(ucanopen::ODEntryIter entry,
                                                     ucanopen::SdoType sdo_type,
                                                     ucanopen::ExpeditedSdoData data) override;
public:
    Server(std::shared_ptr<can::Socket> socket, ucanopen::NodeId node_id, const std::string& name);

private:
    std::atomic<VcuOperationMode> _vcu_opmode{VcuOperationMode::normal};
    std::atomic<bool> _power_enabled{false};
    std::atomic<bool> _run_enabled{false};

public:
    void set_vcu_opmode(VcuOperationMode mode) { _vcu_opmode = mode; }
    void toggle_power(bool is_enabled) { _power_enabled = is_enabled; }
    void toggle_run(bool is_enabled) { _run_enabled = is_enabled; 
    }
private:
    std::atomic<float> _bms_voltage{0.0f};
    std::atomic<float> _bms_charge_pct{0.0f};

private:

};


} // namespace atvvcu
