#pragma once

#include <cstdint>
#include <array>
#include <string_view>
#include <vector>

namespace shm80 {

inline constexpr size_t syslog_domain_count = 10;
inline const std::array<std::string_view, syslog_domain_count> syslog_domains =
        {"sys",
         "settings",
         "can",
         "sensors",
         "motor",
         "converter",
         "model",
         "resolver",
         "pdm",
         "isolation"};

inline const std::array<std::vector<std::string_view>, syslog_domain_count>
        error_list = {
            {{"emergency_stop", "watchdog_timeout", "invalid_config"},
             {"no_nonvolatile_memory", "read_error", "write_error"},
             {"connection_lost"},
             {"vdc_sensor_failure", "iph_sensor_failure", "if_sensor_failure"},
             {"aw_overtemp", "fw_overtemp"},
             {"dc_undervoltage",
              "dc_overvoltage",
              "phase_overcurrent",
              "field_overcurrent",
              "phase_module_fault",
              "field_module_fault",
              "phase_overtemp",
              "field_overtemp",
              "pcb_overtemp"},
             {/* no model errors */},
             {"invalid_configuration", "observer_failure"},
             {"dclink_charge_failure",
              "dclink_charge_timeout",
              "dclink_discharge_timeout",
              "incorrect_switch_state",
              "error_state"},
             {/* no  isotest errors */}}};

inline const std::array<std::vector<std::string_view>, syslog_domain_count>
        warning_list = {
            {{/* no sys warnings */},
             {/* no settings warnings */},
             {"error",
              "overrun",
              "connection_lost",
              "checksum_mismatch",
              "counter_freeze"},
             {/* no sensors warnings */},
             {"overheat"},
             {"overheat"},
             {/* no model warnings */},
             {"calibration_stage1", "calibration_stage2", "calibration_stage3"},
             {"dclink_disconnected",
              "dclink_charging",
              "dclink_holdup",
              "dclink_discharging"},
             {/* no isotest warnings */}}};

inline const std::array<std::vector<std::string_view>, syslog_domain_count>
        syslog_messages = {
            {{  "cpu1_booted_up",
                "cpu2_booted_up",
                "cpu1_ready",
                "cpu2_ready",
                "device_ready",
                "device_busy",
                "device_error",
                "device_resetting"},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            {},
        }};

struct SyslogMessage {
    uint16_t domain;
    uint16_t id;
    SyslogMessage(uint32_t data)
            : domain(data & 0x0000FFFF),
              id(data >> 16) {}
    bool valid() const {
        if (domain == 0xFFFF || id == 0xFFFF) {
            return false;
        }
        if (domain >= syslog_domain_count) {
            return false;
        }
        if (id >= syslog_messages[domain].size()) {
            return false;
        }
        return true;
    }
};

} // namespace shm80
