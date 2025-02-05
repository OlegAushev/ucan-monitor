#pragma once

#include <array>
#include <bit>
#include <cstdint>
#include <cstring>
#include <string_view>
#include <vector>

namespace shm80 {

class syslog {
private:
    static constexpr size_t domain_count = 10;
public:
    static constexpr std::array<std::string_view, domain_count> domains = {
        "sys",
        "settings",
        "can",
        "sensors",
        "motor",
        "converter",
        "model",
        "resolver",
        "pdm",
        "insulation"};
    static constexpr std::array<std::string_view, 6> levels =
            {"      ", "[info]", "[ ok ]", "[warn]", "[fail]", "[crit]"};
private:
    static inline const std::vector<std::string_view> sys_errors_ = {
        "emergency_stop",
        "watchdog_timeout",
        "invalid_config"};
    static inline const std::vector<std::string_view> settings_errors_ = {
        "no_nonvolatile_memory",
        "read_error",
        "write_error"};
    static inline const std::vector<std::string_view> can_errors_ = {
        "vcu_connection_lost",
        "throttle_connection_lost"};
    static inline const std::vector<std::string_view> sensors_errors_ = {
        "vdc_sensor_failure",
        "iph_sensor_failure",
        "if_sensor_failure"};
    static inline const std::vector<std::string_view> motor_errors_ = {
        "overtemp"
        "aw_overtemp",
        "fw_overtemp"};
    static inline const std::vector<std::string_view> converter_errors_ = {
        "dc_undervoltage",
        "dc_overvoltage",
        "phase_overcurrent",
        "phase_a_overcurrent",
        "phase_b_overcurrent",
        "phase_c_overcurrent",
        "field_overcurrent",
        "phase_module_fault",
        "phase_a_fault",
        "phase_b_fault",
        "phase_c_fault",
        "field_module_fault",
        "phase_module_overtemp",
        "phase_a_overtemp",
        "phase_b_overtemp",
        "phase_c_overtemp",
        "field_overtemp",
        "pcb_overtemp"};
    static inline const std::vector<std::string_view> model_errors_{};
    static inline const std::vector<std::string_view> resolver_errors_{
        "invalid_configuration",
        "observer_failure"};
    static inline const std::vector<std::string_view> pdm_errors_{
        "dclink_charge_failure",
        "dclink_charge_timeout",
        "dclink_discharge_timeout",
        "incorrect_switch_state",
        "error_state"};
    static inline const std::vector<std::string_view> insulation_errors_{};
public:
    static inline const std::array<std::vector<std::string_view>,
                                   domains.size()>
            errors = {sys_errors_,
                      settings_errors_,
                      can_errors_,
                      sensors_errors_,
                      motor_errors_,
                      converter_errors_,
                      model_errors_,
                      resolver_errors_,
                      pdm_errors_,
                      insulation_errors_};
private:
    static inline const std::vector<std::string_view> sys_warnings_ = {};
    static inline const std::vector<std::string_view> settings_warnings_ = {};
    static inline const std::vector<std::string_view> can_warnings_ = {
        "error",
        "overrun",
        "vcu_connection_lost",
        "checksum_mismatch",
        "counter_freeze",
        "throttle_invalid_signal"};
    static inline const std::vector<std::string_view> sensors_warnings_ = {};
    static inline const std::vector<std::string_view> motor_warnings_ = {
        "overheat",
        "aw_overheat",
        "fw_overheat"};
    static inline const std::vector<std::string_view> converter_warnings_ = {
        "phase_module_overheat",
        "phase_a_overheat",
        "phase_b_overheat",
        "phase_c_overheat",
        "field_module_overheat",
        "pcb_overheat"};
    static inline const std::vector<std::string_view> model_warnings_{};
    static inline const std::vector<std::string_view> resolver_warnings_{
        "calibration_stage1",
        "calibration_stage2",
        "calibration_stage3"};
    static inline const std::vector<std::string_view> pdm_warnings_{
        "dclink_disconnected",
        "dclink_charging",
        "dclink_holdup",
        "dclink_discharging"};
    static inline const std::vector<std::string_view> insulation_warnings_{
        "low_insulation",
        "invalid_result"};
public:
    static inline const std::array<std::vector<std::string_view>,
                                   domains.size()>
            warnings = {sys_warnings_,
                        settings_warnings_,
                        can_warnings_,
                        sensors_warnings_,
                        motor_warnings_,
                        converter_warnings_,
                        model_warnings_,
                        resolver_warnings_,
                        pdm_warnings_,
                        insulation_warnings_};
private:
    static inline const std::vector<std::string_view> sys_messages_ = {
        "Boot CPU1...",
        "Boot CPU2...",
        "CPU1 is ready.",
        "CPU2 is ready.",
        "Device is ready.",
        "Cannot perform operation - device is busy.",
        "Device is in error state.",
        "Device is being rebooted..."};
    static inline const std::vector<std::string_view> settings_messages_ = {
        "No non-volatile memory detected.",
        "Settings read timeout.",
        "Settings read error occurred",
        "Read all configs - ok.",
        "Apply all configs - ok.",
        "Apply all configs - fail.",
        "Restore all configs -ok.",
        "Restore all configs - fail.",
        "Load all default configs.",

        "Read ucanopen server config - ok.",
        "Read ucanopen server config - fail.",
        "Load ucanopen server default config.",
        "Write ucanopen server config - ok.",
        "Write ucanopen server config - fail.",

        "Read syslog config - ok.",
        "Read syslog config - fail.",
        "Load syslog default config.",
        "Write syslog config - ok.",
        "Write syslog config - fail.",

        "Read drive config - ok.",
        "Read drive config - fail.",
        "Load drive default config.",
        "Write drive config - ok.",
        "Write drive config - fail.",

        "Read motor config - ok.",
        "Read motor config - fail.",
        "Load motor default config.",
        "Write motor config - ok.",
        "Write motor config - fail.",

        "Read converter config - ok.",
        "Read converter config - fail.",
        "Load converter default config.",
        "Write converter config - ok.",
        "Write converter config - fail.",

        "Read model basic config - ok.",
        "Read model basic config - fail.",
        "Load model basic default config.",
        "Write model basic config - ok.",
        "Write model basic config - fail.",

        "Read model controllers config - ok.",
        "Read model controllers config - fail.",
        "Load model controllers default config.",
        "Write model controllers config - ok.",
        "Write model controllers config - fail.",

        "Read mras config - ok.",
        "Read mras config - fail.",
        "Load mras default config.",
        "Write mras config - ok.",
        "Write mras config - fail.",

        "Read pdm config - ok.",
        "Read pdm config - fail.",
        "Load pdm default config.",
        "Write pdm config - ok.",
        "Write pdm config - fail.",

        "Read phase current sensor config - ok.",
        "Read phase current sensor config - fail.",
        "Load phase current sensor default config.",
        "Write phase current sensor config - ok.",
        "Write phase current sensor config - fail.",

        "Read field current sensor config - ok.",
        "Read field current sensor config - fail.",
        "Load field current sensor default config.",
        "Write field current sensor config - ok.",
        "Write field current sensor config - fail.",

        "Read dc voltage sensor config - ok.",
        "Read dc voltage sensor config - fail.",
        "Load dc voltage sensor default config.",
        "Write dc voltage sensor config - ok.",
        "Write dc voltage sensor config - fail.",

        "Read resolver config - ok.",
        "Read resolver config - fail.",
        "Load resolver default config.",
        "Write resolver config - ok.",
        "Write resolver config - fail.",

        "Read resolver calibration config - ok.",
        "Read resolver calibration config - fail.",
        "Load resolver calibration default config.",
        "Write resolver calibration config - ok.",
        "Write resolver calibration config - fail.",

        "Read isolation controller config - ok.",
        "Read isolation controller config - fail.",
        "Load isolation controller default config.",
        "Write isolation controller config - ok.",
        "Write isolation controller config - fail.",

        "Read throttle controller config - ok.",
        "Read throttle controller config - fail.",
        "Load throttle controller default config.",
        "Write throttle controller config - ok.",
        "Write throttle controller config - fail.",
    };
    static inline const std::vector<std::string_view> can_messages_ = {
        "CAN SDO-request is lost."};
    static inline const std::vector<std::string_view> sensors_messages_ = {};
    static inline const std::vector<std::string_view> motor_messages_ = {};
    static inline const std::vector<std::string_view> converter_messages_ = {};
    static inline const std::vector<std::string_view> model_messages_{};
    static inline const std::vector<std::string_view> resolver_messages_{};
    static inline const std::vector<std::string_view> pdm_messages_{};
    static inline const std::vector<std::string_view> isolation_messages_{};
public:
    static inline const std::array<std::vector<std::string_view>,
                                   domains.size()>
            messages = {sys_messages_,
                        settings_messages_,
                        can_messages_,
                        sensors_messages_,
                        motor_messages_,
                        converter_messages_,
                        model_messages_,
                        resolver_messages_,
                        pdm_messages_,
                        isolation_messages_};
private:
    syslog();
public:
    static syslog& instance() {
        static syslog s;
        return s;
    }
};

class SyslogMessage {
private:
    uint8_t domain_;
    uint8_t level_;
    uint16_t id_;
public:
    SyslogMessage(uint32_t data) { *this = std::bit_cast<SyslogMessage>(data); }

    bool valid() const {
        if (domain_ == UINT8_MAX && level_ == UINT8_MAX && id_ == UINT16_MAX) {
            return false;
        }
        if (domain_ >= syslog::domains.size() ||
            level_ >= syslog::levels.size()) {
            return false;
        }
        if (id_ >= syslog::messages[domain_].size()) {
            return false;
        }
        return true;
    }

    std::string_view domain() const { return syslog::domains[domain_]; }
    std::string_view level() const { return syslog::levels[level_]; }
    std::string_view message() const { return syslog::messages[domain_][id_]; }
};

} // namespace shm80
