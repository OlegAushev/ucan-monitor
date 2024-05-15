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
    
    uint8_t drive_state : 8;
    
    int16_t torque : 16;
    
    int16_t speed : 16;
    
    uint8_t _reserved1 : 8;
    
    uint16_t counter : 2;
    uint16_t _reserved2 : 6;
};


struct CobTpdo2 {
    uint16_t dc_voltage : 16;
    uint16_t stator_current : 16;
    uint16_t field_current : 8;
    uint16_t mech_power : 8;
    uint16_t counter : 2;
    uint16_t manual_field_current : 1;
    uint16_t _reserved : 5;
    uint16_t checksum : 8;
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
    uint16_t counter : 2;
    uint16_t _reserved : 6;
    uint16_t checksum : 8;
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
    "@syslog: [  ok  ] CPU1 ready.",
    "@syslog: [ info ] Boot CPU2...",
    "@syslog: [  ok  ] CPU2 ready.",
    "@syslog: [  ok  ] Configured CPU1 periphery.",
    "@syslog: [  ok  ] Configured CPU2 periphery.",
    "@syslog: [  ok  ] Device ready.",
    "@syslog: [ fail ] Device busy.",
    "@syslog: [ fail ] Error occurred.",
    "@syslog: [ info ] Resetting device...",
    "@syslog: [ warn ] Lost SDO request.",
    "@syslog: [ warn ] Cannot communicate with EEPROM: disabled.",
    "@syslog: [  ok  ] Read settings.",
    "@syslog: [ fail ] Failed to read settings.",
    "@syslog: [  ok  ] Applied settings.",
    "@syslog: [ fail ] Failed to apply settings.",
    "@syslog: [  ok  ] Restored settings.",
    "@syslog: [ fail ] Failed to restore settings.",
    "@syslog: [  ok  ] Loaded default settings.",
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
    "powerdown"
};


inline const std::vector<std::string> error_list = {
    "emergency_stop",
    "can_bus_connection_lost",
    "dclink_charge_failure",
    "dclink_discharge_failure",
    "dc_undervoltage",
    "dc_overvoltage",
    "pwr_module_fault",
    "exc_module_fault",
    "pwr_overcurrent",
    "exc_overcurrent",
    "pwr_overtemp",
    "exc_overtemp",
    "pcb_overtemp",
    "aw_overtemp",
    "fw_overtemp",
    "ia_sensor_fault",
    "if_sensor_fault",
    "eeprom_error",
    "resolver_failure"
};


inline const std::vector<std::string> warning_list = {
    "can_bus_error",
    "can_bus_overrun",
    "can_bus_connection_lost",
    "can_bus_checksum_mismatch",
    "can_bus_counter_freeze",
    "dclink_disconnected",
    "dclink_charging",
    "converter_overheat",
    "motor_overheat",
    "flux_weakening",
    "insulation_low",
    "insulation_na",
};


enum class OperationMode {
    normal,
    run,
    track,
    hwtest
};


inline const std::map<int, std::string_view> opmode_map = {
    {std::to_underlying(OperationMode::normal), "normal"},
    {std::to_underlying(OperationMode::run), "run"},
    {std::to_underlying(OperationMode::track), "track"},
    {std::to_underlying(OperationMode::hwtest), "hwtest"},
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
