#pragma once


#include <cstdint>
#include <string>
#include <vector>


namespace atvvcu {


static constexpr int pdm_contactor_count = 7;
enum class PdmContactor : unsigned int {
    battery_p,
    battery_n,
    front_bypass,
    back_bypass,
    aux_bypass,
    charge_allow,
    charge_mode,
};


inline const std::vector<std::string> vcu_opmodes {
    "normal",
    "debug "
};


inline const std::vector<std::string> vcu_states = {
    "idle    ",
    "wait    ",
    "standby ",
    "powerup ",
    "ready   ",
    "start   ",
    "run     ",
    "stop    ",
    "shutdown"
};


struct CobTpdo1 {
    uint32_t vcu_state : 4;
    uint32_t vcu_opmode : 1;
    uint32_t _reserved1 : 27;

    uint32_t _reserved2 : 24;
    uint32_t counter : 2;
    uint32_t _reserved3 : 6;
};


struct CobTpdo2 {
    uint32_t pdm_battery_p : 1;
    uint32_t pdm_battery_n : 1;
    uint32_t pdm_front_bypass : 1;
    uint32_t pdm_back_bypass : 1;
    uint32_t pdm_aux_bypass : 1;
    uint32_t pdm_charge_allow : 1;
    uint32_t pdm_charge_mode : 1;
    uint32_t _reserved1 : 25;
    uint32_t _reserved2 : 24;
    uint32_t counter : 2;
    uint32_t _reserved3 : 6;
};


struct CobTpdo3 {
    uint32_t controller_errors : 24;
    uint32_t _reserved1 : 8;
    uint32_t ctlmode : 1;
    uint32_t controller_enabled : 1;
    uint32_t discharge : 2;
    uint32_t controller_fault_level : 4;
    uint32_t fault_code : 8;
    
    uint32_t wheel : 2;
    uint32_t _reserved2 : 6;

    uint32_t counter : 4;
    uint32_t _reserved3 : 4;
};


struct CobRpdo1 {
    uint32_t debug : 1;
    uint32_t power : 1;
    uint32_t run : 1;
    uint32_t _reserved1 : 29;
    uint32_t _reserved2 : 32;
};


struct CobRpdo2 {
    uint32_t pdm_battery_p : 1;
    uint32_t pdm_battery_n : 1;
    uint32_t pdm_front_bypass : 1;
    uint32_t pdm_back_bypass : 1;
    uint32_t pdm_aux_bypass : 1;
    uint32_t pdm_charge_allow : 1;
    uint32_t pdm_charge_mode : 1;
    uint32_t _reserved1 : 25;
    uint32_t _reserved2 : 24;
    uint32_t counter : 2;
    uint32_t _reserved3 : 6;
};


struct CobRpdo3 {
    uint32_t ctlmode : 1;
    uint32_t enable_controller : 1;
    uint32_t discharge : 1;
    uint32_t mainrelay : 1;
    uint32_t vehicle_fault_level : 4;
    uint32_t gear : 2;
    uint32_t footbrake : 1;
    uint32_t handbrake : 1;
    uint32_t fault_reset : 1;
    uint32_t _reserved1 : 3;
    int16_t speed_cmd : 16;
    int16_t torque_cmd : 16;

    uint32_t wheel : 2;
    uint32_t _reserved2 : 6;

    uint32_t counter : 4;
    uint32_t _reserved3 : 4;
};


inline const std::vector<std::string> syslog_messages = {
    "",
    "         @syslog: Boot CPU1...",
    "[  OK  ] @syslog: CPU1 ready.",
    "         @syslog: Boot CPU2...",
    "[  OK  ] @syslog: CPU2 ready.",
    "[  OK  ] @syslog: Configured CPU1 periphery.",
    "[  OK  ] @syslog: Configured CPU2 periphery.",
    "[  OK  ] @syslog: Device ready.",
    "[FAILED] @syslog: Device busy.",
    "[FAILED] @syslog: Error occurred.",
    "         @syslog: Resetting device...",
    "[ WARN ] @syslog: Lost SDO request.",
    "[ WARN ] @syslog: Cannot communicate with EEPROM: disabled.",
    "[  OK  ] @syslog: Read settings.",
    "[FAILED] @syslog: Failed to read settings.",
    "[  OK  ] @syslog: Applied settings.",
    "[FAILED] @syslog: Failed to apply settings.",
    "[  OK  ] @syslog: Restored settings.",
    "[FAILED] @syslog: Failed to restore settings.",
    "[  OK  ] @syslog: Loaded default settings.",
};


inline const std::vector<std::string_view> error_list = {
    "can_bus_connection_lost",
};


inline const std::vector<std::string_view> warning_list = {
    "can_bus_error",
    "can_bus_overrun",
    "can_bus_connection_lost",
};


} // namespace atvvcu
