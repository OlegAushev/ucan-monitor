#pragma once


#include "srmdrive_def.h"
#include "ucanopen/server/server.h"
#include "log/log.h"
#include <algorithm>


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
    uint32_t _errors = 0;
    uint16_t _warnings = 0;

    bool _power_enabled = false;
    bool _run_enabled = false;
    ControlMode _ctlmode = ControlMode::torque;
    bool _emergency_enabled = false;
    double _torque_perunit_ref = 0;
    double _speed_rpm_ref = 0;

    bool _manual_fieldctl = false;
    ControlLoopType _ctlloop = ControlLoopType::closed;
    double _field_current_ref = 0;
    double _stator_current_perunit_ref = 0;

public:
    uint32_t errors() const { return _errors; }
    uint16_t warnings() const { return _warnings; }

    void set_power_enabled(bool enabled) { _power_enabled = enabled; }
    void set_run_enabled(bool enabled) { _run_enabled = enabled; }
    void set_ctlmode(ControlMode mode) { _ctlmode = mode; }
    void set_emergency_enabled(bool enabled) { _emergency_enabled = enabled; }
    void set_torque(double value_perunit) { _torque_perunit_ref = std::clamp(value_perunit, -1.0, 1.0); }
    void set_speed(double value_rpm) { _speed_rpm_ref = value_rpm; }

    void set_manual_fieldctl_enabled(bool enabled) { _manual_fieldctl = enabled; }
    void set_ctlloop(ControlLoopType ctlloop) { _ctlloop = ctlloop; }
    void set_field_current(double val) { _field_current_ref = std::clamp(val, 0.0, 100.0); }
    void set_stator_current(double val_perunit) { _stator_current_perunit_ref = std::clamp(val_perunit, 0.0, 1.0); }
private:
    void _handle_tpdo1(const ucanopen::can_payload& payload);
    void _handle_tpdo2(const ucanopen::can_payload& payload);
    void _handle_tpdo3(const ucanopen::can_payload& payload);
    void _handle_tpdo4(const ucanopen::can_payload& payload);

    ucanopen::can_payload _create_rpdo1();
    ucanopen::can_payload _create_rpdo2();

    struct Tpdo1 {
        bool run;
        bool error;
        bool warning;
        bool overheat;
        std::string ctlmode;
        std::string ctlloop;
        std::string drive_state;
        int torque;
        int speed;
    } _tpdo1{};

    struct Tpdo2 {
        unsigned int dc_voltage;
        unsigned int stator_current;
        float field_current;
        unsigned int mech_power;
        bool manual_field_current;
    } _tpdo2{};

    struct Tpdo3 {
        int pwrmodule_temp;
        int excmodule_temp;
        int pcb_temp;
        int aw_temp;
        int fw_temp;
    } _tpdo3{};
public:
    const Tpdo1& tpdo1() const { return _tpdo1; }
    const Tpdo2& tpdo2() const { return _tpdo2; }
    const Tpdo3& tpdo3() const { return _tpdo3; }
};

} // namespace srmdrive

