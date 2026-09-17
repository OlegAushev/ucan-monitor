#include "pdu_server.hpp"

namespace pdu {

using namespace ucanopen;

// Mirrors object_dictionary_storage in
// src/app/pdu/comm/can/canopen/od.cpp of the h2-hess firmware.
// Entry order follows the firmware table so the two can be diffed line by line.

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
{{0x1010, 0x01}, {"ctl", "sys", "save_all_parameters",            "", OD_ACCESS_WO, OD_EXEC}},
{{0x1011, 0x01}, {"ctl", "sys", "restore_all_default_parameters", "", OD_ACCESS_WO, OD_EXEC}},
{{0x1011, 0x03}, {"ctl", "sys", "erase_all_parameters",           "", OD_ACCESS_WO, OD_EXEC}},
//##########################################################################################################################################################################################################################################################################################################
{{0x1018, 0x04}, {"info", "sys", "serial_number", "", OD_ACCESS_CONST, OD_UINT32}},
//##########################################################################################################################################################################################################################################################################################################
{{0x2000, 0x01}, {"ctl", "sys", "reset_device", "", OD_ACCESS_WO, OD_EXEC}},
{{0x2000, 0x02}, {"ctl", "sys", "clear_errors", "", OD_ACCESS_WO, OD_EXEC}},
{{0x2000, 0x03}, {"ctl", "sys", "emergency",    "", OD_ACCESS_WO, OD_EXEC}},
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
//##########################################################################################################################################################################################################################################################################################################
{{0x3001, 0x01}, {"config", "power", "precharge_delta",     "V", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3001, 0x02}, {"config", "power", "battery_voltage_min", "V", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3001, 0x03}, {"config", "power", "precharge_timeout",   "s", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3001, 0x04}, {"config", "power", "main_settle_time",    "s", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3001, 0x05}, {"config", "power", "contactor_fb_time",   "s", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3001, 0x06}, {"config", "power", "startup_delay",       "s", OD_ACCESS_RW, OD_FLOAT32}},
//##########################################################################################################################################################################################################################################################################################################
{{0x3002, 0x01}, {"config", "sensor", "battery_voltage_installed",  "", OD_ACCESS_RW, OD_BOOL}},
{{0x3002, 0x02}, {"config", "sensor", "fuelcell_voltage_installed", "", OD_ACCESS_RW, OD_BOOL}},
{{0x3002, 0x03}, {"config", "sensor", "inverter_voltage_installed", "", OD_ACCESS_RW, OD_BOOL}},
//##########################################################################################################################################################################################################################################################################################################
{{0x4000, 0x01}, {"debug", "substitute", "battery_voltage",  "V", OD_ACCESS_RW, OD_FLOAT32}},
{{0x4000, 0x02}, {"debug", "substitute", "fuelcell_voltage", "V", OD_ACCESS_RW, OD_FLOAT32}},
{{0x4000, 0x03}, {"debug", "substitute", "inverter_voltage", "V", OD_ACCESS_RW, OD_FLOAT32}},
//##########################################################################################################################################################################################################################################################################################################
{{0x5000, 0x01}, {"watch", "sys", "uptime", "s", OD_ACCESS_RO, OD_FLOAT32}},
//##########################################################################################################################################################################################################################################################################################################
{{0x5000, 0xF1}, {"watch", "logger", "ch0", "", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0xF2}, {"watch", "logger", "ch1", "", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0xF3}, {"watch", "logger", "ch2", "", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0xF4}, {"watch", "logger", "ch3", "", OD_ACCESS_RO, OD_FLOAT32}},
//##########################################################################################################################################################################################################################################################################################################
}
};
// clang-format on

} // namespace pdu
