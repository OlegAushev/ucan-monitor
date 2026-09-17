#include "chss_server.hpp"

namespace chss {

using namespace ucanopen;

// Mirrors object_dictionary_storage in
// src/app/chss/comm/can/canopen/od.cpp of the h2-hess firmware.
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
{{0x3001, 0x01}, {"config", "system",  "startup_delay",            "s",    OD_ACCESS_RW, OD_FLOAT32}},
//##########################################################################################################################################################################################################################################################################################################
{{0x3002, 0x01}, {"config", "sensor",  "pressure_min",             "atm",  OD_ACCESS_RW, OD_FLOAT32}},
{{0x3002, 0x02}, {"config", "sensor",  "pressure_max",             "atm",  OD_ACCESS_RW, OD_FLOAT32}},
{{0x3002, 0x03}, {"config", "sensor",  "flow_min",                 "m3/h", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3002, 0x04}, {"config", "sensor",  "flow_max",                 "m3/h", OD_ACCESS_RW, OD_FLOAT32}},
//##########################################################################################################################################################################################################################################################################################################
{{0x3002, 0x05}, {"config", "sensor",  "receiver_pressure_installed",       "",     OD_ACCESS_RW, OD_BOOL}},
{{0x3002, 0x06}, {"config", "sensor",  "fill_line_pressure_installed",      "",     OD_ACCESS_RW, OD_BOOL}},
{{0x3002, 0x07}, {"config", "sensor",  "pressure_before_reducer_installed", "",     OD_ACCESS_RW, OD_BOOL}},
{{0x3002, 0x08}, {"config", "sensor",  "pressure_after_reducer_installed",  "",     OD_ACCESS_RW, OD_BOOL}},
{{0x3002, 0x09}, {"config", "sensor",  "inflow_rate_installed",             "",     OD_ACCESS_RW, OD_BOOL}},
//##########################################################################################################################################################################################################################################################################################################
{{0x3003, 0x01}, {"config", "fill",    "start_pressure_max",       "atm",  OD_ACCESS_RW, OD_FLOAT32}},
{{0x3003, 0x02}, {"config", "fill",    "pressure_max",             "atm",  OD_ACCESS_RW, OD_FLOAT32}},
//##########################################################################################################################################################################################################################################################################################################
{{0x3004, 0x01}, {"config", "storage", "leak_pressure_drop",       "atm",  OD_ACCESS_RW, OD_FLOAT32}},
//##########################################################################################################################################################################################################################################################################################################
{{0x3005, 0x01}, {"config", "supply",  "filter_dp_warn_th",        "atm",  OD_ACCESS_RW, OD_FLOAT32}},
{{0x3005, 0x02}, {"config", "supply",  "reducer_in_pressure_min",  "atm",  OD_ACCESS_RW, OD_FLOAT32}},
{{0x3005, 0x03}, {"config", "supply",  "reducer_in_pressure_max",  "atm",  OD_ACCESS_RW, OD_FLOAT32}},
{{0x3005, 0x04}, {"config", "supply",  "reducer_out_pressure_min", "atm",  OD_ACCESS_RW, OD_FLOAT32}},
{{0x3005, 0x05}, {"config", "supply",  "reducer_out_pressure_max", "atm",  OD_ACCESS_RW, OD_FLOAT32}},
//##########################################################################################################################################################################################################################################################################################################
{{0x4000, 0x01}, {"debug", "substitute", "receiver_pressure",       "atm",  OD_ACCESS_RW, OD_FLOAT32}},
{{0x4000, 0x02}, {"debug", "substitute", "fill_line_pressure",      "atm",  OD_ACCESS_RW, OD_FLOAT32}},
{{0x4000, 0x03}, {"debug", "substitute", "pressure_before_reducer", "atm",  OD_ACCESS_RW, OD_FLOAT32}},
{{0x4000, 0x04}, {"debug", "substitute", "pressure_after_reducer",  "atm",  OD_ACCESS_RW, OD_FLOAT32}},
{{0x4000, 0x05}, {"debug", "substitute", "inflow_rate",             "m3/h", OD_ACCESS_RW, OD_FLOAT32}},
//##########################################################################################################################################################################################################################################################################################################
{{0x5000, 0x01}, {"watch", "sys", "uptime", "s", OD_ACCESS_RO, OD_FLOAT32}},
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
}
};
// clang-format on

} // namespace chss
