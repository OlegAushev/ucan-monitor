#pragma once


#include <cstdint>
#include <cstring>
#include <vector>
#include <string_view>
#include <string>


namespace srmdrive {

struct CobTpdo1 {
    uint16_t run : 1;
    uint16_t error : 1;
    uint16_t warning : 1;
    uint16_t overheat : 1;
    uint16_t ctlmode : 1;
    uint16_t ctlloop : 1;
    uint16_t _reserved : 2;
    uint16_t drive_state : 8;
    int16_t torque : 16;
    int16_t speed : 16;
    uint16_t counter : 2;
    uint16_t _reserved2 : 6;
    uint16_t checksum : 8;
    CobTpdo1() { memset(this, 0, sizeof(CobTpdo1)); static_assert(sizeof(CobTpdo1) == 8); }
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
    CobTpdo2() { memset(this, 0, sizeof(CobTpdo2)); static_assert(sizeof(CobTpdo2) == 8); }
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
    CobTpdo3() { memset(this, 0, sizeof(CobTpdo3)); static_assert(sizeof(CobTpdo3) == 8); }
};


struct CobTpdo4 {
    uint32_t errors;
    uint16_t warnings;
    uint16_t counter : 2;
    uint16_t _reserved : 6;
    uint16_t checksum : 8;
    CobTpdo4() { memset(this, 0, sizeof(CobTpdo4)); static_assert(sizeof(CobTpdo4) == 8); }
};


//----------------------------------------------------------------------------------------------------------------------
struct CobRpdo1 {
    uint16_t power : 1;
    uint16_t run : 1;
    uint16_t ctlmode : 1;
    uint16_t disable_isotest : 1;
    uint16_t emergency_stop : 1;
    uint16_t _reserved1 : 11;
    int16_t torque_ref;
    int16_t speed_ref;
    uint16_t counter : 2;
    uint16_t _reserved2 : 6;
    uint16_t checksum : 8;
    CobRpdo1() { memset(this, 0, sizeof(CobTpdo1)); static_assert(sizeof(CobRpdo1) == 8); }
};


struct CobRpdo2 {
    uint16_t manual_fieldctl : 1;
    uint16_t ctlloop : 1;
    uint16_t _reserved1 : 14;
    uint16_t field_current_ref;
    uint16_t stator_current_ref;
    uint16_t counter : 2;
    uint16_t _reserved2 : 6;
    uint16_t checksum : 8;
    CobRpdo2() { memset(this, 0, sizeof(CobTpdo2)); static_assert(sizeof(CobRpdo2) == 8); }
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
    "idle",
    "wait",
    "standby",
    "startup",
    "ready",
    "prepare",
    "start",
    "run",
    "stop",
    "shutdown",
    "cal_start",
    "cal_stage1",
    "cal_stage2",
    "cal_stage3",
    "cal_stop"
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


enum class ControlMode {
    torque,
    speed
};


enum class ControlLoopType {
    closed,
    open
};


}
