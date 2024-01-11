#pragma once


#include "srmdrive_def.h"
#include "ucanopen/server/server.h"
#include "bsclog/bsclog.h"
#include <algorithm>
#include <atomic>


namespace srmdrive {

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
    std::atomic<uint32_t> _errors = 0;
    std::atomic<uint16_t> _warnings = 0;

    std::atomic<bool> _power_enabled{false};
    std::atomic<bool> _run_enabled{false};
    std::atomic<ControlMode> _ctlmode{ControlMode::torque};
    std::atomic<bool> _emergency_enabled{false};
    std::atomic<float> _torque_perunit_ref{0};
    std::atomic<float> _speed_rpm_ref{0};

    std::atomic<bool> _manual_fieldctl{false};
    std::atomic<ControlLoop> _ctlloop{ControlLoop::closed};
    std::atomic<uint16_t> _openloop_angle{0};
    std::atomic<float> _field_current_ref{0};
    std::atomic<float> _d_current_perunit_ref{0};

public:
    uint32_t errors() const { return _errors; }
    uint16_t warnings() const { return _warnings; }

    const auto& error_list() const { return ::srmdrive::error_list; }
    const auto& warning_list() const { return ::srmdrive::warning_list; }

    void set_power_enabled(bool enabled) { _power_enabled = enabled; }
    void set_run_enabled(bool enabled) { _run_enabled = enabled; }
    void set_ctlmode(ControlMode mode) { _ctlmode = mode; }
    void set_emergency_enabled(bool enabled) { _emergency_enabled = enabled; }
    void set_torque(float value_perunit) { _torque_perunit_ref = std::clamp(value_perunit, -1.0f, 1.0f); }
    void set_speed(float value_rpm) { _speed_rpm_ref = value_rpm; }

    void set_manual_fieldctl_enabled(bool enabled) { _manual_fieldctl = enabled; }
    void set_ctlloop(ControlLoop ctlloop) { _ctlloop = ctlloop; }
    void set_openloop_angle(uint16_t value) { _openloop_angle = value % 360; }
    void set_field_current(float val) { _field_current_ref = std::clamp(val, 0.0f, 100.0f); }
    void set_d_current(float val_perunit) { _d_current_perunit_ref = std::clamp(val_perunit, -1.0f, 1.0f); }
private:
    void _handle_tpdo1(const ucanopen::can_payload& payload);
    void _handle_tpdo2(const ucanopen::can_payload& payload);
    void _handle_tpdo3(const ucanopen::can_payload& payload);
    void _handle_tpdo4(const ucanopen::can_payload& payload);

    ucanopen::can_payload _create_rpdo1();
    ucanopen::can_payload _create_rpdo2();

public:
    struct Tpdo1 {
        bool run;
        bool error;
        bool warning;
        bool overheat;
        std::string_view ctlmode;
        std::string_view ctlloop;
        std::string_view drive_state;
        int torque;
        int speed;
    };

    struct Tpdo2 {
        unsigned int dc_voltage;
        unsigned int stator_current;
        float field_current;
        unsigned int mech_power;
        bool manual_field_current;
    };

    struct Tpdo3 {
        int pwrmodule_temp;
        int excmodule_temp;
        int pcb_temp;
        int aw_temp;
        int fw_temp;
    };

private:
    std::atomic<Tpdo1> _tpdo1{};
    std::atomic<Tpdo2> _tpdo2{};
    std::atomic<Tpdo3> _tpdo3{};

public:
    Tpdo1 tpdo1() const { return _tpdo1.load(); }
    Tpdo2 tpdo2() const { return _tpdo2.load(); }
    Tpdo3 tpdo3() const { return _tpdo3.load(); }
};

} // namespace srmdrive

