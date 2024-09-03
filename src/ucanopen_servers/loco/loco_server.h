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
        std::atomic<ControlMode> ctlmode{ControlMode::torque};
        std::atomic<float> ref_torque{0};
        std::atomic<int16_t> ref_speed{0};
    } _rpdo1;

    struct {
        std::atomic<OperatingMode> opmode{OperatingMode::normal};
        std::atomic<ControlLoop> ctlloop{ControlLoop::closed};
        std::atomic<int16_t> ref_d_angle{0};
        std::atomic<float> ref_d_current{0};
    } _rpdo2;

    struct {
        std::atomic<bool> manual_field{false};
        std::atomic<float> ref_f_current{0};
    } _rpdo3;

    struct {
        std::atomic<DriveState> drive_state{DriveState::init};
        std::atomic<bool> pwm_on{false};
        std::atomic<bool> error{false};
        std::atomic<bool> warning{false};
        std::atomic<OperatingMode> opmode{OperatingMode::normal};
        std::atomic<ControlMode> ctlmode{ControlMode::torque};
        std::atomic<ControlLoop> ctlloop{ControlLoop::closed};
        std::atomic<float> torque{0};
        std::atomic<int16_t> speed{0};
        std::atomic<bool> manual_field{false};
    } _tpdo1;

    struct {
      std::atomic<uint32_t> errors{0};
      std::atomic<uint16_t> warnings{0};
    } _tpdo4;

public:
    void toggle_power(bool v) { _rpdo1.power.store(v); }
    void toggle_start(bool v) { _rpdo1.start.store(v); }
    void set_ctlmode(ControlMode v) { _rpdo1.ctlmode.store(v); }
    void set_torque(float v_pu) { _rpdo1.ref_torque.store(std::clamp(v_pu, 0.f, 1.f)); }
    void set_speed(int16_t v) { _rpdo1.ref_speed.store(v); }
    
    void set_opmode(OperatingMode v) { _rpdo2.opmode.store(v); }
    void set_ctlloop(ControlLoop v) { _rpdo2.ctlloop.store(v); }
    void set_d_angle(int16_t v_deg) { _rpdo2.ref_d_angle.store(v_deg); }
    void set_d_current(float v_pu) { _rpdo2.ref_d_current.store(std::clamp(v_pu, 0.f, 1.f)); }
    
    void toggle_manual_field(bool v) { _rpdo3.manual_field.store(v); }
    void set_f_current(float v_pu) { _rpdo3.ref_f_current.store(std::clamp(v_pu, 0.f, 1.f)); }
public:
    DriveState drive_state() const { return _tpdo1.drive_state.load(); }
    bool pwm_on() const { return _tpdo1.pwm_on.load(); }
    bool error() const { return _tpdo1.error.load(); }
    bool warning() const { return _tpdo1.warning.load(); }
    OperatingMode opmode() const { return _tpdo1.opmode.load(); }
    ControlMode ctlmode() const { return _tpdo1.ctlmode.load(); }
    ControlLoop ctlloop() const { return _tpdo1.ctlloop.load(); }
    float torque() const { return _tpdo1.torque.load(); }
    int16_t speed() const { return _tpdo1.speed.load(); }
    bool manual_field() const { return _tpdo1.manual_field.load(); }

    uint32_t errors() const { return _tpdo4.errors.load(); }
    uint16_t warnings() const { return _tpdo4.warnings.load(); }
    const auto& error_list() const { return ::loco::error_list; }
    const auto& warning_list() const { return ::loco::warning_list; }

};


} // namespace loco
