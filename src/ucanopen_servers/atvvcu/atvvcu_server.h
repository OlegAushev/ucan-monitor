#pragma once


#include "ucanopen/server/server.h"
#include "atvvcu_def.h"
#include "bsclog/bsclog.h"
#include <array>
#include <atomic>
#include <cstdint>
#include <string_view>

#include "util/accl.h"
#include "util/bms.h"
#include "util/brakes.h"
#include "util/dash.h"
#include "util/drive.h"
#include "util/gear.h"
#include "util/pdm.h"
#include "util/esp.h"


namespace atvvcu {


extern const ucanopen::ObjectDictionary object_dictionary;
extern const ucanopen::ObjectDictionaryConfig object_dictionary_config;


class Server : public ucanopen::Server, public ucanopen::SdoSubscriber {
private:
    std::atomic<std::string_view> _vcu_state{"n/a     "};
    std::atomic_bool _vcu_debug_mode;
public:
    bms::Controller bms;
    pdm::Controller pdm;
    drive::Controller drive;
    dash::Dashboard dash;
    gear::GearSelector gear_selector;
    accl::AcceleratorPedal accl_pedal;
    brk::Brakes brakes;
    esp::EspSystem esp_system;

    std::array<std::atomic_uint32_t, error_domain_count> errors{};
    std::array<std::atomic_uint16_t, error_domain_count> warnings{};
protected:
    void _handle_tpdo1(const ucanopen::can_payload& payload);
    void _handle_tpdo2(const ucanopen::can_payload& payload);
    void _handle_tpdo3(const ucanopen::can_payload& payload);
    void _handle_tpdo4(const ucanopen::can_payload& payload);

    ucanopen::can_payload _create_rpdo1();
    ucanopen::can_payload _create_rpdo2();

    virtual ucanopen::FrameHandlingStatus handle_sdo(ucanopen::ODEntryIter entry,
                                                     ucanopen::SdoType sdo_type,
                                                     ucanopen::ExpeditedSdoData data) override;
public:
    Server(std::shared_ptr<can::Socket> socket, ucanopen::NodeId node_id, const std::string& name);
    
    std::string_view vcu_state() const { return _vcu_state; }
    bool vcu_debug_mode() const { return _vcu_debug_mode; }
};


} // namespace atvvcu
