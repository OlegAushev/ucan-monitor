#pragma once


#include "brkdrive_def.h"
#include "ucanopen/server/server.h"
#include "bsclog/bsclog.h"
#include <algorithm>
#include <atomic>


namespace brkdrive {

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
    struct {
        std::atomic<float> ref_angle{0};
        std::atomic<uint16_t> track_speed{0};
        std::atomic<bool> master_bad{false};
        std::atomic<bool> wakeup{false};
        std::atomic<OperatingStatus> cmd_opstatus{OperatingStatus::inoperable};
    } _rpdo1;

    struct {
        std::atomic<OperatingMode> opmode{OperatingMode::normal};
        std::atomic<ControlMode> ctlmode{ControlMode::torque};
        std::atomic<ControlLoop> ctlloop{ControlLoop::closed};
        std::atomic<float> ref_torque{0};
        std::atomic<int16_t> ref_speed{0};
    } _rpdo2;

    struct {
        std::atomic<float> ref_dcurr{0};
        std::atomic<int16_t> openloop_ref_angle{0};
    } _rpdo3;

    struct {
        std::atomic<float> angle{0};
        std::atomic<OperatingStatus> opstatus{OperatingStatus::inoperable};
        std::atomic<DriveState> drive_state{DriveState::waiting};

        std::atomic<bool> pwm_on{false};
        std::atomic<bool> error{false};
        std::atomic<bool> warning{false};
        std::atomic<OperatingMode> opmode{OperatingMode::normal};
        std::atomic<ControlMode> ctlmode{ControlMode::torque};
        std::atomic<ControlLoop> ctlloop{ControlLoop::closed};
    } _tpdo1;

    struct {
        std::atomic<float> ref_angle{0};
        std::atomic<float> ref_brake{0};
    } _tpdo2;

    struct {
        std::atomic<int16_t> speed{0};
    } _tpdo3;

    struct {
        std::atomic<uint32_t> errors{0};
        std::atomic<uint16_t> warnings{0};
    } _tpdo4;

public:
    void set_ref_angle(float value) { _rpdo1.ref_angle.store(value); }
    void set_track_speed(uint16_t value) { _rpdo1.track_speed.store(value); }
    void toggle_master_bad(bool value) { _rpdo1.master_bad = value; }
    void toggle_wakeup(bool value) { _rpdo1.wakeup.store(value); }
    void set_ref_status(OperatingStatus status) { _rpdo1.cmd_opstatus.store(status); }

    void set_opmode(OperatingMode mode) { _rpdo2.opmode.store(mode); }
    void set_ctlmode(ControlMode mode) { _rpdo2.ctlmode.store(mode); }
    void set_ctlloop(ControlLoop loop) { _rpdo2.ctlloop.store(loop); }
    void set_ref_torque(float value) { _rpdo2.ref_torque.store(std::clamp(value, -1.0f, 1.0f)); }
    void set_ref_speed(int16_t value) { _rpdo2.ref_speed.store(value); }

    void set_ref_dcurr(float value) { _rpdo3.ref_dcurr.store(std::clamp(value, -1.0f, 1.0f)); }
    void set_openloop_ref_angle(int16_t value) { _rpdo3.openloop_ref_angle.store(value); }

    float angle() const { return _tpdo1.angle.load(); }
    OperatingStatus status() const { return _tpdo1.opstatus.load(); }
    DriveState drive_state() const { return _tpdo1.drive_state.load(); }
    bool is_pwm_on() const { return _tpdo1.pwm_on.load(); }
    bool has_error() const { return _tpdo1.error.load(); }
    bool has_warning() const { return _tpdo1.warning.load(); }
    OperatingMode opmode() const { return _tpdo1.opmode.load(); }
    ControlMode ctlmode() const { return _tpdo1.ctlmode.load(); }
    ControlLoop ctlloop() const { return _tpdo1.ctlloop.load(); }
    
    float ref_angle() const { return _tpdo2.ref_angle.load(); }
    float ref_brake() const { return _tpdo2.ref_brake.load(); }
    
    int16_t speed() const { return _tpdo3.speed.load(); }

    uint32_t errors() const { return _tpdo4.errors.load(); }
    uint16_t warnings() const { return _tpdo4.warnings.load(); }

    const auto& error_list() const { return ::brkdrive::error_list; }
    const auto& warning_list() const { return ::brkdrive::warning_list; }

private:
    void _handle_tpdo1(const ucanopen::can_payload& payload);
    void _handle_tpdo2(const ucanopen::can_payload& payload);
    void _handle_tpdo3(const ucanopen::can_payload& payload);
    void _handle_tpdo4(const ucanopen::can_payload& payload);

    ucanopen::can_payload _create_rpdo1();
    ucanopen::can_payload _create_rpdo2();
    ucanopen::can_payload _create_rpdo3();
};

} // namespace brkdrive
