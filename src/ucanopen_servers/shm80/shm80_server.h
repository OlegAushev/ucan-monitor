#pragma once

#include "shm80_def.h"
#include <algorithm>
#include <atomic>
#include <bsclog/bsclog.h>
#include <ucanopen/server/server.h>

namespace shm80 {

extern const ucanopen::ObjectDictionary object_dictionary;
extern const ucanopen::ObjectDictionaryConfig object_dictionary_config;

class Server : public ucanopen::Server, public ucanopen::SdoSubscriber {
public:
    Server(std::shared_ptr<can::Socket> socket,
           ucanopen::NodeId node_id,
           const std::string& name);
protected:
    virtual ucanopen::FrameHandlingStatus
    handle_sdo(ucanopen::ODEntryIter entry,
               ucanopen::SdoType sdo_type,
               ucanopen::ExpeditedSdoData data) override final;
private:
    struct {
        std::atomic<bool> emergency_stop{false};
        std::atomic<bool> power{false};
        std::atomic<bool> start{false};
        std::atomic<ControlMode> ctlmode{ControlMode::torque};
        std::atomic<bool> isotest_dis{false};
        std::atomic<float> ref_torque{0};
        std::atomic<int16_t> ref_speed{0};
    } _rpdo1;

    struct {
        std::atomic<int16_t> ref_angle{0};
        std::atomic<float> ref_current{0};
        std::atomic<float> ref_voltage{0};
        std::atomic<float> ref_field{0};
        std::atomic<OperatingMode> opmode{OperatingMode::normal};
        std::atomic<ControlLoop> ctlloop{ControlLoop::closed};
        std::atomic<bool> manual_field{false};
    } _rpdo2;

    struct {
        std::atomic<DriveState> drive_state{DriveState::init};
        std::atomic<bool> pwm_on{false};
        std::atomic<bool> error{false};
        std::atomic<bool> warning{false};
        std::atomic<OperatingMode> opmode{OperatingMode::normal};
        std::atomic<ControlMode> ctlmode{ControlMode::torque};
        std::atomic<ControlLoop> ctlloop{ControlLoop::closed};

        std::atomic<std::string_view> pdu_precharge_state{"n/a"};
        std::atomic<std::string_view> calibration_state{"n/a"};
        std::atomic<std::string_view> insulation_tester_state{"n/a"};

        std::atomic<bool> manual_field{false};

        std::atomic<bool> pdu_main_contactor{false};
        std::atomic<bool> pdu_charging_contactor{false};
        std::atomic<bool> pdu_bypassing_contactor{false};

        std::atomic<bool> din_ship_failure_warning{false};
        std::atomic<bool> din_ship_failure{false};
        std::atomic<bool> din_start{false};
        std::atomic<bool> dout_power_request{false};
        std::atomic<bool> dout_drive_ready{false};
        std::atomic<bool> dout_drive_started{false};
        std::atomic<bool> dout_drive_failure{false};
    } _tpdo1;

    struct {
        std::atomic<float> dc_voltage{0};
        std::atomic<float> stator_current{0};
        std::atomic<float> field_current{0};
    } _tpdo2;

    struct {
        std::atomic<int16_t> speed{0};
        std::atomic<int16_t> angle{0};
    } _tpdo3;

    std::array<std::atomic_uint32_t, syslog::domains.size()> _errors{};
    std::array<std::atomic_uint16_t, syslog::domains.size()> _warnings{};
public:
    // RPDO
    void toggle_emergency(bool v) { _rpdo1.emergency_stop.store(v); }
    void toggle_power(bool v) { _rpdo1.power.store(v); }
    void toggle_start(bool v) { _rpdo1.start.store(v); }
    void set_ctlmode(ControlMode v) { _rpdo1.ctlmode.store(v); }
    void toggle_isotest_dis(bool v) { _rpdo1.isotest_dis.store(v); }
    void set_ref_torque(float v) {
        _rpdo1.ref_torque.store(std::clamp(v, -1.0f, 1.0f));
    }
    void set_ref_speed(int16_t value) { _rpdo1.ref_speed.store(value); }

    void set_ref_angle(int16_t v) { _rpdo2.ref_angle.store(v); }
    void set_ref_current(float v) {
        _rpdo2.ref_current.store(std::clamp(v, -1.0f, 1.0f));
    }
    void set_ref_voltage(float v) {
        _rpdo2.ref_voltage.store(std::clamp(v, 0.0f, 1.0f));
    }
    void set_ref_field(float v) {
        _rpdo2.ref_field.store(std::clamp(v, 0.0f, 1.0f));
    }
    void set_opmode(OperatingMode v) { _rpdo2.opmode.store(v); }
    void set_ctlloop(ControlLoop v) { _rpdo2.ctlloop.store(v); }
    void toggle_manual_field(bool v) { _rpdo2.manual_field.store(v); }

    // TPDO
    DriveState drive_state() const { return _tpdo1.drive_state.load(); }
    std::string_view drive_state_str() const {
        auto it = drive_state_names.find(drive_state());
        if (it == drive_state_names.end()) {
            return "n/a";
        } else {
            return it->second;
        }
    }

    bool is_pwm_on() const { return _tpdo1.pwm_on.load(); }
    bool has_error() const { return _tpdo1.error.load(); }
    bool has_any_warning() const { return _tpdo1.warning.load(); }
    OperatingMode opmode() const { return _tpdo1.opmode.load(); }
    std::string_view opmode_str() const {
        auto it = opmode_names.find(opmode());
        if (it == opmode_names.end()) {
            return "n/a";
        } else {
            return it->second;
        }
    }

    ControlMode ctlmode() const { return _tpdo1.ctlmode.load(); }
    std::string_view ctlmode_str() const {
        auto it = ctlmode_names.find(ctlmode());
        if (it == ctlmode_names.end()) {
            return "n/a";
        } else {
            return it->second;
        }
    }

    ControlLoop ctlloop() const { return _tpdo1.ctlloop.load(); }
    std::string_view ctlloop_str() const {
        auto it = ctlloop_names.find(ctlloop());
        if (it == ctlloop_names.end()) {
            return "n/a";
        } else {
            return it->second;
        }
    }

    std::string_view pdu_precharge_state() const {
        return _tpdo1.pdu_precharge_state.load();
    }
    std::string_view insulation_tester_state() const {
        return _tpdo1.insulation_tester_state.load();
    }

    bool manual_field_enabled() const { return _tpdo1.manual_field.load(); }

    bool pdu_main_contactor() const { return _tpdo1.pdu_main_contactor.load(); }
    bool pdu_charging_contactor() const {
        return _tpdo1.pdu_charging_contactor.load();
    }
    bool pdu_bypassing_contactor() const {
        return _tpdo1.pdu_bypassing_contactor.load();
    }

    bool din_ship_failure_warning() const {
        return _tpdo1.din_ship_failure_warning.load();
    };
    bool din_ship_failure() const { return _tpdo1.din_ship_failure.load(); };
    bool din_start() const { return _tpdo1.din_start.load(); };
    bool dout_power_request() const {
        return _tpdo1.dout_power_request.load();
    };
    bool dout_drive_ready() const { return _tpdo1.dout_drive_ready.load(); };
    bool dout_drive_started() const {
        return _tpdo1.dout_drive_started.load();
    };
    bool dout_drive_failure() const {
        return _tpdo1.dout_drive_failure.load();
    };

    float dc_voltage() const { return _tpdo2.dc_voltage.load(); }
    float stator_current() const { return _tpdo2.stator_current.load(); }
    float field_current() const { return _tpdo2.field_current.load(); }

    int16_t speed() const { return _tpdo3.speed.load(); }
    int16_t angle() const { return _tpdo3.angle.load(); }

    bool has_any_error() const {
        for (auto& entry : _errors) {
            if (entry.load() != 0) {
                return true;
            }
        }
        return false;
    }
    const auto& errors() const { return _errors; }
    const auto& warnings() const { return _warnings; }
private:
    void _handle_tpdo1(const ucanopen::can_payload& payload);
    void _handle_tpdo2(const ucanopen::can_payload& payload);
    void _handle_tpdo3(const ucanopen::can_payload& payload);
    void _handle_tpdo4(const ucanopen::can_payload& payload);

    ucanopen::can_payload _create_rpdo1();
    ucanopen::can_payload _create_rpdo2();
};

} // namespace shm80
