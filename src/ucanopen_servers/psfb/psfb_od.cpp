#include "psfb_server.hpp"

namespace psfb {

using namespace ucanopen;

// Mirrors object_dictionary_storage in
// src/app/converter/comm/can/canopen/od.cpp of the adpt-etk-psfb-converter
// firmware. Entry order follows the firmware table so the two can be diffed
// line by line.

// clang-format off
extern const ucanopen::ObjectDictionary object_dictionary = {
.config = {
    .watch_category = "watch",
    .config_category = "config"
},
.entries = {
{{0x1008, 0x00}, {"info", "sys", "device_name", "", OD_ACCESS_CONST, OD_STRING}},
{{0x1009, 0x00}, {"info", "sys", "hardware_version", "", OD_ACCESS_CONST, OD_STRING}},
{{0x100A, 0x00}, {"info", "sys", "firmware_version", "", OD_ACCESS_CONST, OD_STRING}},
{{0x100A, 0x01}, {"info", "sys", "firmware_commitdate", "", OD_ACCESS_CONST, OD_STRING}},
{{0x100A, 0x02}, {"info", "sys", "firmware_branch", "", OD_ACCESS_CONST, OD_STRING}},
//##########################################################################################################################################################################################################################################################################################################
{{0x1010, 0x01}, {"ctl", "sys", "save_all_parameters", "", OD_ACCESS_WO, OD_EXEC}},
{{0x1011, 0x01}, {"ctl", "sys", "restore_all_default_parameters", "", OD_ACCESS_WO, OD_EXEC}},
{{0x1011, 0x03}, {"ctl", "sys", "erase_all_parameters", "", OD_ACCESS_WO, OD_EXEC}},
{{0x1011, 0x04}, {"ctl", "sys", "restore_default_parameter",        "", OD_ACCESS_WO, OD_EXEC}},
//##########################################################################################################################################################################################################################################################################################################
{{0x1018, 0x04}, {"info", "sys", "serial_number", "", OD_ACCESS_CONST, OD_UINT32}},
//##########################################################################################################################################################################################################################################################################################################
{{0x2000, 0x01}, {"ctl", "sys", "reset_device", "", OD_ACCESS_WO, OD_EXEC}},
{{0x2000, 0x02}, {"ctl", "sys", "clear_errors", "", OD_ACCESS_WO, OD_EXEC}},
{{0x2000, 0x03}, {"ctl", "sys", "emergency",    "", OD_ACCESS_WO, OD_EXEC}},
//##########################################################################################################################################################################################################################################################################################################
{{0x5000, 0x01}, {"watch", "sys", "uptime",     "s",  OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x02}, {"watch", "sys", "state",      "",   OD_ACCESS_RO, OD_UINT32}},
//##########################################################################################################################################################################################################################################################################################################
{{0x5000, 0x11}, {"watch", "elec", "Vhv", "V", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x12}, {"watch", "elec", "Vlv", "V", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x13}, {"watch", "elec", "I1",  "A", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x14}, {"watch", "elec", "I2",  "A", OD_ACCESS_RO, OD_FLOAT32}},
//##########################################################################################################################################################################################################################################################################################################
{{0x5000, 0x21}, {"watch", "temp", "Tmcu", "°C", OD_ACCESS_RO, OD_FLOAT32}},
//##########################################################################################################################################################################################################################################################################################################
{{0x5000, 0x31}, {"watch", "pwm", "ch1_active",      "", OD_ACCESS_RO, OD_BOOL}},
{{0x5000, 0x32}, {"watch", "pwm", "ch2_active",      "", OD_ACCESS_RO, OD_BOOL}},
{{0x5000, 0x33}, {"watch", "pwm", "ch1_overlap",     "", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x34}, {"watch", "pwm", "ch2_overlap",     "", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x35}, {"watch", "pwm", "ch1_overlap_ref", "", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x36}, {"watch", "pwm", "ch2_overlap_ref", "", OD_ACCESS_RO, OD_FLOAT32}},
//##########################################################################################################################################################################################################################################################################################################
{{0x5000, 0xF1}, {"watch", "logger", "ch0", "", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0xF2}, {"watch", "logger", "ch1", "", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0xF3}, {"watch", "logger", "ch2", "", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0xF4}, {"watch", "logger", "ch3", "", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0xF5}, {"watch", "logger", "ch4", "", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0xF6}, {"watch", "logger", "ch5", "", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0xF7}, {"watch", "logger", "ch6", "", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0xF8}, {"watch", "logger", "ch7", "", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0xF9}, {"watch", "logger", "ch8", "", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0xFA}, {"watch", "logger", "ch9", "", OD_ACCESS_RO, OD_FLOAT32}},
//##########################################################################################################################################################################################################################################################################################################
{{0x3000, 0x01}, {"config", "nvm", "magic",            "",  OD_ACCESS_RO, OD_UINT32}},
{{0x3000, 0x02}, {"config", "nvm", "slot_count",       "",  OD_ACCESS_RO, OD_UINT32}},
{{0x3000, 0x03}, {"config", "nvm", "slot_capacity",    "B", OD_ACCESS_RO, OD_UINT32}},
{{0x3000, 0x04}, {"config", "nvm", "slots_per_block",  "",  OD_ACCESS_RO, OD_UINT32}},
{{0x3000, 0x05}, {"config", "nvm", "sequence",         "",  OD_ACCESS_RO, OD_UINT32}},
{{0x3000, 0x06}, {"config", "nvm", "erase_cycles",     "",  OD_ACCESS_RO, OD_UINT32}},
{{0x3000, 0x07}, {"config", "nvm", "loaded_slot",      "",  OD_ACCESS_RO, OD_UINT32}},
{{0x3000, 0x08}, {"config", "nvm", "loaded_sequence",  "",  OD_ACCESS_RO, OD_UINT32}},
{{0x3000, 0x09}, {"config", "nvm", "valid",            "",  OD_ACCESS_RO, OD_BOOL}},
{{0x3000, 0x0A}, {"config", "nvm", "schema_matched",   "",  OD_ACCESS_RO, OD_BOOL}},
{{0x3000, 0x0B}, {"config", "nvm", "read_failed",      "",  OD_ACCESS_RO, OD_BOOL}},
{{0x3000, 0x0C}, {"config", "nvm", "stored",           "",  OD_ACCESS_RO, OD_UINT32}},
{{0x3000, 0x0D}, {"config", "nvm", "loaded",           "",  OD_ACCESS_RO, OD_UINT32}},
{{0x3000, 0x0E}, {"config", "nvm", "unknown",          "",  OD_ACCESS_RO, OD_UINT32}},
{{0x3000, 0x0F}, {"config", "nvm", "rejected",         "",  OD_ACCESS_RO, OD_UINT32}},
{{0x3000, 0x10}, {"config", "nvm", "missing",          "",  OD_ACCESS_RO, OD_UINT32}},
{{0x3000, 0x11}, {"config", "nvm", "restart_required", "",  OD_ACCESS_RO, OD_BOOL}},
{{0x3000, 0x12}, {"config", "nvm", "changes_pending",  "",  OD_ACCESS_RO, OD_BOOL}},
//##########################################################################################################################################################################################################################################################################################################
{{0x3003, 0x01}, {"config", "protection", "otp_mcu",          "°C", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3003, 0x02}, {"config", "protection", "watchdog_timeout", "ms", OD_ACCESS_RW, OD_UINT32}},
//##########################################################################################################################################################################################################################################################################################################
{{0x3008, 0x01}, {"config", "output_current_sensor", "gain", "A/V", OD_ACCESS_CONST, OD_FLOAT32}},
//##########################################################################################################################################################################################################################################################################################################
{{0x3009, 0x01}, {"config", "hvdc_voltage_sensor", "gain",   "V/V", OD_ACCESS_CONST, OD_FLOAT32}},
{{0x3009, 0x02}, {"config", "hvdc_voltage_sensor", "offset", "V",   OD_ACCESS_CONST, OD_FLOAT32}},
{{0x300A, 0x01}, {"config", "lvdc_voltage_sensor", "gain",   "V/V", OD_ACCESS_CONST, OD_FLOAT32}},
{{0x300A, 0x02}, {"config", "lvdc_voltage_sensor", "offset", "V",   OD_ACCESS_CONST, OD_FLOAT32}},
//##########################################################################################################################################################################################################################################################################################################
}
};
// clang-format on

} // namespace psfb
