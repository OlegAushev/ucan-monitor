#pragma once


#include <array>
#include <cstdint>
#include <string>
#include <vector>


namespace atvvcu {


enum class VcuOperationMode : unsigned int {
    normal,
    ctlemu,
    debug,
};


inline const std::vector<std::string> vcu_opmodes {
    "normal",
    "ctlemu",
    "debug "
};


inline const std::vector<std::string> vcu_states = {
    "idle    ",
    "wait    ",
    "standby ",
    "poweron ",
    "ready   ",
    "start   ",
    "run     ",
    "stop    ",
    "poweroff"
};


struct CobTpdo1 {
    uint32_t vcu_state : 4;
    uint32_t vcu_dbg : 1;
    uint32_t _reserved1 : 3;

    uint32_t dash_dbg : 1;
    uint32_t pdm_dbg : 1;
    uint32_t drive_dbg : 1;
    uint32_t gear_dbg : 1;
    uint32_t accl_dbg : 1;
    uint32_t steer_dbg : 1;
    uint32_t _reserved2 : 2;

    uint32_t emergency : 1;
    uint32_t faultreset : 1;
    uint32_t power : 1;
    uint32_t run : 1;
    uint32_t gear : 2;
    uint32_t _reserved3 : 2;

    uint32_t pdm_battery_p : 1;
    uint32_t pdm_battery_n : 1;
    uint32_t pdm_front_bypass : 1;
    uint32_t pdm_back_bypass : 1;
    uint32_t pdm_aux_bypass : 1;
    uint32_t pdm_charge_allow : 1;
    uint32_t pdm_charge_mode : 1;
    uint32_t _reserved4 : 1;

    uint32_t accl : 8;

    uint32_t _reserved5 : 16;

    uint32_t counter : 2;
    uint32_t _reserved6 : 6;
};


struct CobTpdo2 {
    uint32_t bms_voltage : 16;

    uint32_t bms_charge_pct : 8;

    uint32_t _reserved1 : 8;

    uint32_t _reserved2 : 16;
    
    uint32_t _reserved3 : 8;
    
    uint32_t counter : 2;
    uint32_t _reserved4 : 6;
};


struct CobTpdo3 {
    uint32_t controller_errors : 24;
    uint32_t _reserved1 : 8;
    uint32_t ctlmode : 1;
    uint32_t controller_enabled : 1;
    uint32_t discharge : 2;
    uint32_t controller_faultlevel : 4;
    uint32_t faultcode : 8;
    
    uint32_t wheel : 2;
    uint32_t _reserved2 : 6;

    uint32_t counter : 4;
    uint32_t _reserved3 : 4;
};


struct CobTpdo4 {
    uint32_t errors;
    uint16_t warnings;
    uint16_t counter : 2;
    uint16_t domain : 6;
    uint16_t _reserved : 8;
};


struct CobRpdo1 {
    uint32_t dash_dbg : 1;
    uint32_t pdm_dbg : 1;
    uint32_t drive_dbg : 1;
    uint32_t gear_dbg : 1;
    uint32_t accl_dbg : 1;
    uint32_t steer_dbg : 1;
    uint32_t _reserved1 : 2;

    uint32_t emergency : 1;
    uint32_t faultreset : 1;
    uint32_t power : 1;
    uint32_t run : 1;
    uint32_t gear : 2;
    uint32_t _reserved2 : 2;

    uint32_t pdm_battery_p : 1;
    uint32_t pdm_battery_n : 1;
    uint32_t pdm_front_bypass : 1;
    uint32_t pdm_back_bypass : 1;
    uint32_t pdm_aux_bypass : 1;
    uint32_t pdm_charge_allow : 1;
    uint32_t pdm_charge_mode : 1;
    uint32_t _reserved3 : 1;

    uint32_t accl : 8;

    uint32_t _reserved4 : 24;

    uint32_t counter : 2;
    uint32_t _reserved5 : 6;
};

struct CobRpdo2 {
    uint32_t ctlmode : 1;
    uint32_t enable_controller : 1;
    uint32_t discharge : 1;
    uint32_t mainrelay : 1;
    uint32_t vehicle_faultlevel : 4;
    uint32_t gear : 2;
    uint32_t footbrake : 1;
    uint32_t handbrake : 1;
    uint32_t faultreset : 1;
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


inline constexpr size_t error_domain_count = 4;
inline const std::array<std::string_view, error_domain_count> error_domains = {
    "sys", "ucanopen", "pdm", "accl"
};


inline const std::array<std::vector<std::string_view>, error_domain_count> error_list = {{
    {
        "emergency_stop",
        "eeprom_error",
    },
    {
        "can_bus_error",
        "can_bus_overrun",
        "can_bus_connection_lost",
        "can_bus_checksum_mismatch",
        "can_bus_counter_freeze",
    },
    {
        "battery_p_failure",
        "battery_n_failure",
        "front_bypass_failure",
        "back_bypass_failure",
        "aux_bypass_failure",
        "charge_allow_failure",
        "charge_mode_failure",
        "poweron_failed",
        "poweroff_failed"
    },
    {
        "accl1_low",
        "accl1_high",
        "accl2_low",
        "accl2_high",
        "accl12_ratio"
    }
}};


inline const std::array<std::vector<std::string_view>, error_domain_count> warning_list = {{
    {

    },
    {
        "can_bus_error",
        "can_bus_overrun",
        "can_bus_connection_lost",
        "can_bus_checksum_mismatch",
        "can_bus_counter_freeze",     
    },
    {

    },
    {
        
    }
}};


inline const std::vector<std::string_view> bms_internal_states = {
    "low_soc",
    "high_ch_current",
    "ch_contactor_state",
    "allow_charging",
    "ch_current_present",
    "dch_contactor_state",
    "dch_current_present",
    "increased_voltage",
    "high_dch_temp_state",
    "cooler_state",
    "hyg_shutdown",
    "init",
    "prech_contactor_state",
    "combilift_shutdown",
    "cell_analysis",
    "balancing_series_1",
    "balancing_series_2",
    "aux_dch_contactor_state",
    "power_down_ack",
    "crown_ews",
    "main_contactor_state",
    "service_reset",
    "ch_dch_contactor_state",
    "ready_to_charge",
    "ready_to_discharge",   
};


inline const std::vector<std::string_view> bms_error_register_1 = {
    "overcurrent",
    "undervoltage",
    "overvoltage",
    "low_dch_temp",
    "high_dch_temp",
    "battery_cover",
    "high_humidity",
    "water",
    "high_logic_temp",
    "logic_offline",
    "critical_error",
    "crown_error",
    "cell_count_error",
    "hyg_offline",
    "need_ack",
    "combilift_offline",
    "short_circuit",
    "high_contactor_temp",
    "logic_count_error",
    "adc_error",
    "current_sensor_error",
    "ch_contactor_cycles_error",
    "dch_contactor_cycles_error",
    "shunt_offline",
    "shunt_error",
    "settings_error",
    "wdt_reset",
    "no_temp_sensors",
    "temp_sensor_shorted",
    "spirit_offline",
};


inline const std::vector<std::string_view> bms_error_register_2 = {
    "low_ch_temp",
    "high_ch_temp",
    "sd_mount_error",
    "sd_rw_error",
    "unallowable_charging",
    "stuck_contactor",
    "ch_contactor_feedback_error",
    "dch_contactor_feedback_error",
    "insulation_fault",
    "pch_contactor_feedback_error",
    "ch_dch_contactor_feedback_error",
    "main_contactor_feedback_error",
};


inline const std::vector<std::string_view> bms_discrete_inputs_1 = {
    "battery_cover",
    "charger_connected",
    "power_down_request",
    "inhibit_charging",
    "inhibit_discharging",
    "ch_contactor_feedback",
    "dch_contactor_feedback",
    "insulation_status",  
};


inline const std::vector<std::string_view> bms_discrete_inputs_2 = {
    "charge_request",
    "precharge_request",
    "discharge_request",
    "pch_contactor_feedback",
    "ch_dch_contactor_feedback",
    "main_contactor_feedback",
};


} // namespace atvvcu
