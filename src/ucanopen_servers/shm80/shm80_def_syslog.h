#pragma once

#include <array>
#include <string_view>
#include <vector>

namespace shm80 {

inline constexpr size_t syslog_domain_count = 9;
inline const std::array<std::string_view, syslog_domain_count> syslog_domains =
        {"sys",
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
            {{"emergency_stop",
              "watchdog_timeout",
              "ext_memory_error",
              "invalid_config"},
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
             {"observer_failure"},
             {"dclink_charge_failure",
              "dclink_charge_timeout",
              "dclink_discharge_timeout",
              "incorrect_switch_state",
              "error_state"},
             {/* no  isotest errors */}}};

inline const std::array<std::vector<std::string_view>, syslog_domain_count>
        warning_list = {
            {{/* no sys warnings */},
             {"error",
              "overrun",
              "connection_lost",
              "checksum_mismatch",
              "counter_freeze"},
             {/* no sensors warnings */},
             {"overheat"},
             {"overheat"},
             {/* no model warnings */},
             {/* no resolver warnings*/},
             {"dclink_disconnected",
              "dclink_charging",
              "dclink_holdup",
              "dclink_discharging"},
             {/* no isotest warnings */}}};

} // namespace shm80
