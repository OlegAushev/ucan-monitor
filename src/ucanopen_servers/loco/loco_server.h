#pragma once


#include "loco_def.h"
#include "ucanopen/server/server.h"
#include "bsclog/bsclog.h"
#include <algorithm>
#include <atomic>


namespace loco {


extern const ucanopen::ObjectDictionary object_dictionary;
extern const ucanopen::ObjectDictionaryConfig object_dictionary_config;


class Server : public ucanopen::Server, public ucanopen::SdoSubscriber {
public:
    Server(std::shared_ptr<can::Socket> socket, ucanopen::NodeId node_id, const std::string& name);
protected:
    virtual ucanopen::FrameHandlingStatus handle_sdo(ucanopen::ODEntryIter entry,
                                                     ucanopen::SdoType sdo_type,
                                                     ucanopen::ExpeditedSdoData data) override final;
private:
    void _handle_tpdo1(const ucanopen::can_payload& payload);
    void _handle_tpdo2(const ucanopen::can_payload& payload);
    void _handle_tpdo3(const ucanopen::can_payload& payload);
    void _handle_tpdo4(const ucanopen::can_payload& payload);

    ucanopen::can_payload _create_rpdo1();
    ucanopen::can_payload _create_rpdo2();
    ucanopen::can_payload _create_rpdo3();
private:
    struct {
        std::atomic<bool> power{false};
        std::atomic<bool> start{false}; 
        std::atomic<OperatingMode> opmode{OperatingMode::normal};
        std::atomic<ControlMode> ctlmode{ControlMode::torque};
        std::atomic<ControlLoop> ctlloop{ControlLoop::closed};
        std::atomic<float> ref_torque{0};
        std::atomic<int16_t> ref_speed{0};
    } _rpdo1;

    struct {

    } _rpdo2;

    struct {

    } _rpdo3;


};


} // namespace loco
