#pragma once


#include "moyka_def.h"
#include "ucanopen/server/server.h"
#include "bsclog/bsclog.h"
#include <algorithm>
#include <atomic>


namespace moyka {

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
        std::atomic<bool> pwm_on{false};
        std::atomic<bool> error{false};
        std::atomic<int16_t> speed{0};
        std::atomic<uint16_t> dc_voltage{0};
    } _tpdo1;
    
    struct {
        std::atomic<float> throttle{0.5f};
    } _tpdo2;

    struct {
        std::atomic<uint32_t> errors{0};
        std::atomic<uint16_t> warnings{0};
    } _tpdo4;

    std::atomic<DriveState> _drive_state{DriveState::STANDBY};
public:
    DriveState drive_state() const { return _drive_state.load(); }
    float throttle() const { return _tpdo2.throttle.load(); }
 /*    void set_ref_angle(float value) { _rpdo1.ref_angle.store(value); }
    void set_track_speed(uint16_t value) { _rpdo1.track_speed.store(value); }
    void toggle_master_bad(bool value) { _rpdo1.master_bad = value; }
    void toggle_wakeup(bool value) { _rpdo1.wakeup.store(value); }
    void set_cmd_status(OperatingStatus status) { _rpdo1.cmd_opstatus.store(status); }

    void set_opmode(OperatingMode mode) { _rpdo2.opmode.store(mode); }
    void set_ctlmode(ControlMode mode) { _rpdo2.ctlmode.store(mode); }
    void set_ctlloop(ControlLoop loop) { _rpdo2.ctlloop.store(loop); }
    void set_ref_torque(float value) { _rpdo2.ref_torque.store(std::clamp(value, -1.0f, 1.0f)); }
    void set_ref_speed(int16_t value) { _rpdo2.ref_speed.store(value); }

    void set_ref_dcurr(float value) { _rpdo3.ref_dcurr.store(std::clamp(value, -1.0f, 1.0f)); }
    void set_openloop_ref_angle(int16_t value) { _rpdo3.openloop_ref_angle.store(value); }

    float angle() const { return _tpdo1.angle.load(); }
    OperatingStatus opstatus() const { return _tpdo1.opstatus.load(); }
    bool is_calibrated() const { return _tpdo1.calibrated; }
    CalibrationState calstatus() const { return _tpdo1.calstatus; }
    bool is_pwm_on() const { return _tpdo1.pwm_on.load(); }
    bool has_error() const { return _tpdo1.error.load(); }
    bool has_warning() const { return _tpdo1.warning.load(); }
    OperatingMode opmode() const { return _tpdo1.opmode.load(); }
    ControlMode ctlmode() const { return _tpdo1.ctlmode.load(); }
    ControlLoop ctlloop() const { return _tpdo1.ctlloop.load(); }
    bool is_trk_completed() const { return _tpdo1.trkcompleted; }
    
    float ref_angle() const { return _tpdo2.ref_angle.load(); }
    float ref_brake() const { return _tpdo2.ref_brake.load(); }
    
    int16_t speed() const { return _tpdo3.speed.load(); }

    uint32_t errors() const { return _tpdo4.errors.load(); }
    uint16_t warnings() const { return _tpdo4.warnings.load(); }

    const auto& error_list() const { return ::brkdrive::error_list; }
    const auto& warning_list() const { return ::brkdrive::warning_list; } */

private:
    void _handle_tpdo1(const ucanopen::can_payload& payload);
    void _handle_tpdo2(const ucanopen::can_payload& payload);
    void _handle_tpdo4(const ucanopen::can_payload& payload);
};

} // namespace brkdrive
