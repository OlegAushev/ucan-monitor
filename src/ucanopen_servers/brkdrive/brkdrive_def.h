#pragma once


#include <cstdint>
#include <cstring>
#include <map>
#include <string>
#include <string_view>
#include <utility>
#include <vector>


namespace brkdrive {

struct CobTpdo1 {
    uint8_t run : 1;
    uint8_t error : 1;
    uint8_t warning : 1;
    uint8_t opmode : 2;
    uint8_t ctlmode : 1;
    uint8_t ctlloop : 2;
    
    uint8_t drive_state;
    
    int16_t torque;
    
    int16_t speed;
    
    uint8_t _reserved1;
    
    uint8_t counter : 2;
    uint8_t _reserved2 : 6;
};


struct CobTpdo2 {
    uint16_t braking;

    uint8_t _reserved1;

    uint8_t _reserved2;

    uint8_t _reserved3;

    uint8_t _reserved4;

    uint8_t _reserved5;

    uint8_t counter : 2;
    uint8_t _reserved6 : 6;
};


struct CobTpdo3 {
    uint16_t pwrmodule_temp : 8;
    uint16_t excmodule_temp : 8;
    uint16_t pcb_temp : 8;
    uint16_t aw_temp : 8;
    uint16_t fw_temp : 8;
    uint16_t _reserved : 6;
    uint16_t counter : 2;
    uint16_t _reserved2 : 6;
    uint16_t checksum : 8;
};


struct CobTpdo4 {
    uint32_t errors;
    uint16_t warnings;
    uint8_t _reserved1;
    uint8_t counter : 2;
    uint8_t _reserved2 : 6;
};


//----------------------------------------------------------------------------------------------------------------------
struct CobRpdo1 {
    uint8_t wakeup : 1;
    uint8_t _reserved1 : 7;
    
    uint8_t _reserved2;
    
    uint16_t brake_ref;
    
    uint8_t _reserved3;
    
    uint8_t _reserved4;
    
    uint8_t _reserved5;
    
    uint8_t counter : 2;
    uint8_t _reserved6 : 6;
};


struct CobRpdo2 {
    int16_t torque_ref;
    int16_t speed_ref;
    int16_t dcurr_ref;
    uint8_t _reserved1;
    uint8_t counter : 2;
    uint8_t opmode : 2;
    uint8_t ctlmode : 1;
    uint8_t ctlloop : 2;
    uint8_t run : 1;
};


struct CobRpdo3 {
    int16_t openloop_angle_ref;
    int16_t angle_ref;
    uint16_t track_speed;
    uint8_t _reserved1;
    uint8_t counter : 2;
    uint8_t _reserved2 : 6;
};


//----------------------------------------------------------------------------------------------------------------------
inline const std::vector<std::string> syslog_messages = {
    "",
    "@syslog: [ info ] Boot CPU1...",
    "@syslog: [  ok  ] CPU1 is ready.",
    "@syslog: [ info ] Boot CPU2...",
    "@syslog: [  ok  ] CPU2 is ready.",
    "@syslog: [  ok  ] CPU1 periphery is configured.",
    "@syslog: [  ok  ] CPU2 periphery is configured.",
    "@syslog: [  ok  ] Device is ready.",
    "@syslog: [ fail ] Device is busy.",
    "@syslog: [ fail ] Error occurred.",
    "@syslog: [ info ] Device is resetting...",
    "@syslog: [ warn ] SDO request is lost.",
    "@syslog: [ warn ] Cannot communicate with external memory: disabled.",
    "@syslog: [  ok  ] Load settings - ok.",
    "@syslog: [ fail ] Load settings - fail.",
    "@syslog: [  ok  ] Apply settings - ok.",
    "@syslog: [ fail ] Apply settings - fail.",
    "@syslog: [  ok  ] Restore settings - ok.",
    "@syslog: [ fail ] Restore settings - fail.",
    "@syslog: [  ok  ] Load default settings - ok.",

    "@syslog: [  ok  ] Load ucanopen server config - ok.",
    "@syslog: [ fail ] Load ucanopen server config - fail.",
    "@syslog: [  ok  ] Load syslog config - ok",
    "@syslog: [ fail ] Load syslog config - fail",
    "@syslog: [  ok  ] Load drive config - ok",
    "@syslog: [ fail ] Load drive config - fail",
    "@syslog: [  ok  ] Load motor config - ok",
    "@syslog: [ fail ] Load motor config - fail",
    "@syslog: [  ok  ] Load converter config - ok",
    "@syslog: [ fail ] Load converter config - fail",
    "@syslog: [  ok  ] Load model basic config - ok",
    "@syslog: [ fail ] Load model basic config - fail",
    "@syslog: [  ok  ] Load model controllers config - ok",
    "@syslog: [ fail ] Load model controllers config - fail",
    "@syslog: [  ok  ] Load mras config - ok",
    "@syslog: [ fail ] Load mras config - fail",
    "@syslog: [  ok  ] Load ac currsens config - ok",
    "@syslog: [ fail ] Load ac currsens config - fail",

    "@syslog: [  ok  ] Calibration completed successfully."	
};


inline const std::vector<std::string> drive_states = {
    "wait",
    "standby",
    "powerup",
    "ready",
    "braking",
    "release",
    "run",
    "tracking",
    "powerdown",
    "hwtest"
};


inline const std::vector<std::string> error_list = {
    "emergency_stop",
    "can_bus_no_conn",
    "dc_undervoltage",
    "dc_overvoltage",
    "dc_overcurrent",
    "module_fault",
    "ac_overcurrent",
    "module_overtemp",
    "motor_overtemp",
    "vdc_sensor_fault",
    "iph_sensor_fault",
    "eeprom_error",
    "invalid_config",
};


inline const std::vector<std::string> warning_list = {
    "can_bus_error",
    "can_bus_overrun",
    "can_bus_connection_lost",
    "can_bus_checksum_mismatch",
    "can_bus_counter_freeze",
    "converter_overheat",
    "motor_overheat",
    "flux_weakening",
};


enum class OperationMode {
    normal,
    run,
    track,
    hwtest
};


inline constexpr bool is_opmode(int v) {
    switch (static_cast<OperationMode>(v)) {
    case OperationMode::normal:
    case OperationMode::run:
    case OperationMode::track:
    case OperationMode::hwtest:
        return true;
    }
    return false;
}


inline const std::map<OperationMode, std::string_view> opmode_string_map = {
    {OperationMode::normal, "normal"},
    {OperationMode::run, "run"},
    {OperationMode::track, "track"},
    {OperationMode::hwtest, "hwtest"},
};


enum class ControlMode {
    torque,
    speed
};


inline const std::map<int, std::string_view> ctlmode_map = {
    {std::to_underlying(ControlMode::torque), "torque"},
    {std::to_underlying(ControlMode::speed), "speed"}
};


enum class ControlLoop {
    closed,
    open,
    semiclosed
};


inline const std::map<int, std::string_view> ctlloop_map = {
    {std::to_underlying(ControlLoop::closed), "closed"},
    {std::to_underlying(ControlLoop::open), "open"},
    {std::to_underlying(ControlLoop::semiclosed), "semiclosed"}
};


} // namespace brkdrive
