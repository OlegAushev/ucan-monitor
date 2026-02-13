#include "sevpress_server.hpp"

namespace sevpress {

using namespace ucanopen;

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

{{0x1010, 0x01}, {"ctl", "sys", "save_all_parameters", "", OD_ACCESS_WO, OD_EXEC}},
{{0x1011, 0x01}, {"ctl", "sys", "restore_all_default_parameters", "", OD_ACCESS_WO, OD_EXEC}},
{{0x1011, 0x04}, {"ctl", "sys", "restore_default_parameter",        "", OD_ACCESS_WO, OD_EXEC}},

{{0x1018, 0x04}, {"info", "sys", "serial_number", "", OD_ACCESS_CONST, OD_UINT32}},

{{0x2000, 0x01}, {"ctl", "sys", "reset_device", "", OD_ACCESS_WO, OD_EXEC}},
{{0x2000, 0x02}, {"ctl", "sys", "clear_errors", "", OD_ACCESS_WO, OD_EXEC}},
{{0x2000, 0x03}, {"ctl", "sys", "emergency",    "", OD_ACCESS_WO, OD_EXEC}},

{{0x2001, 0x01}, {"ctl", "drive", "calibrate_angle_sensor",   "", OD_ACCESS_WO, OD_EXEC}},
{{0x2001, 0x02}, {"ctl", "drive", "save_angle_sensor_config", "", OD_ACCESS_WO, OD_EXEC}},
{{0x2001, 0x03}, {"ctl", "drive", "set_angle_correction",     "", OD_ACCESS_WO, OD_FLOAT32}},
{{0x2001, 0x04}, {"ctl", "drive", "set_vD_limit_factor",      "", OD_ACCESS_WO, OD_FLOAT32}},

{{0x5000, 0x01}, {"watch", "sys", "uptime",     "s",  OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x02}, {"watch", "sys", "syslogmsg",  "",   OD_ACCESS_RO, OD_UINT32}},

{{0x5000, 0x11}, {"watch", "converter", "vDC",  "V",  OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x12}, {"watch", "converter", "iA",   "A",  OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x13}, {"watch", "converter", "iB",   "A",  OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x14}, {"watch", "converter", "iC",   "A",  OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x15}, {"watch", "converter", "tPWR", "°C", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x16}, {"watch", "converter", "tMCU", "°C", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x17}, {"watch", "converter", "v5",   "V",  OD_ACCESS_RO, OD_FLOAT32}},

{{0x5000, 0x21}, {"watch", "model",     "iS",       "A",    OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x22}, {"watch", "model",     "iD",       "A",    OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x23}, {"watch", "model",     "iDRef",    "A",    OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x24}, {"watch", "model",     "iQ",       "A",    OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x25}, {"watch", "model",     "iQRef",    "A",    OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x26}, {"watch", "model",     "vD",       "A",    OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x27}, {"watch", "model",     "vQ",       "A",    OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x28}, {"watch", "model",     "vS",       "A",    OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x29}, {"watch", "model",     "angD",     "°",    OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x2A}, {"watch", "model",     "angCorr",  "°",    OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x2B}, {"watch", "model",     "vDLim",    "",     OD_ACCESS_RO, OD_FLOAT32}},

{{0x5000, 0x31}, {"watch", "motor",     "tAW",  "°C", OD_ACCESS_RO, OD_FLOAT32}},

{{0x5000, 0x41}, {"watch", "drive",     "torqueRef",    "%",    OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x42}, {"watch", "drive",     "torque",       "%",    OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x43}, {"watch", "drive",     "speedRef",     "rpm",  OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x44}, {"watch", "drive",     "speed",        "rpm",  OD_ACCESS_RO, OD_FLOAT32}},

{{0x5000, 0x51}, {"watch", "hall", "speed",   "rpm",  OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x52}, {"watch", "hall", "angle",   "°",    OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x53}, {"watch", "hall", "sector",  "",     OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x54}, {"watch", "hall", "inA",     "",     OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x55}, {"watch", "hall", "inB",     "",     OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x56}, {"watch", "hall", "inC",     "",     OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x57}, {"watch", "hall", "emu_dspeed", "rpm", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x58}, {"watch", "hall", "emu_dangle", "°",   OD_ACCESS_RO, OD_FLOAT32}},

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
{{0x3000, 0x01}, {"config", "ucanopen", "node_id",          "",     OD_ACCESS_RW, OD_UINT32}},
{{0x3000, 0x02}, {"config", "ucanopen", "heartbeat_period", "ms",   OD_ACCESS_RW, OD_UINT32}},
{{0x3000, 0x03}, {"config", "ucanopen", "sync_period",      "ms",   OD_ACCESS_RW, OD_UINT32}},
{{0x3000, 0x04}, {"config", "ucanopen", "tpdo1_period",     "ms",   OD_ACCESS_RW, OD_UINT32}},
{{0x3000, 0x05}, {"config", "ucanopen", "tpdo2_period",     "ms",   OD_ACCESS_RW, OD_UINT32}},
{{0x3000, 0x06}, {"config", "ucanopen", "tpdo3_period",     "ms",   OD_ACCESS_RW, OD_UINT32}},
{{0x3000, 0x07}, {"config", "ucanopen", "tpdo4_period",     "ms",   OD_ACCESS_RW, OD_UINT32}},
{{0x3000, 0x08}, {"config", "ucanopen", "rpdo1_timeout",    "ms",   OD_ACCESS_RW, OD_UINT32}},
{{0x3000, 0x09}, {"config", "ucanopen", "rpdo2_timeout",    "ms",   OD_ACCESS_RW, OD_UINT32}},
{{0x3000, 0x0A}, {"config", "ucanopen", "rpdo3_timeout",    "ms",   OD_ACCESS_RW, OD_UINT32}},
{{0x3000, 0x0B}, {"config", "ucanopen", "rpdo4_timeout",    "ms",   OD_ACCESS_RW, OD_UINT32}},
{{0x3000, 0x0C}, {"config", "ucanopen", "rpdo1_id",         "",     OD_ACCESS_RW, OD_UINT32}},
{{0x3000, 0x0D}, {"config", "ucanopen", "rpdo2_id",         "",     OD_ACCESS_RW, OD_UINT32}},
{{0x3000, 0x0E}, {"config", "ucanopen", "rpdo3_id",         "",     OD_ACCESS_RW, OD_UINT32}},
{{0x3000, 0x0F}, {"config", "ucanopen", "rpdo4_id",         "",     OD_ACCESS_RW, OD_UINT32}},
//##########################################################################################################################################################################################################################################################################################################
{{0x3001, 0x01}, {"config", "syslog", "general_enabled_errors",   "hex",  OD_ACCESS_RW, OD_UINT32}},
{{0x3001, 0x02}, {"config", "syslog", "settings_enabled_errors",  "hex",  OD_ACCESS_RW, OD_UINT32}},
{{0x3001, 0x03}, {"config", "syslog", "can_enabled_errors",       "hex",  OD_ACCESS_RW, OD_UINT32}},
{{0x3001, 0x04}, {"config", "syslog", "sensors_enabled_errors",   "hex",  OD_ACCESS_RW, OD_UINT32}},
{{0x3001, 0x05}, {"config", "syslog", "motor_enabled_errors",     "hex",  OD_ACCESS_RW, OD_UINT32}},
{{0x3001, 0x06}, {"config", "syslog", "converter_enabled_errors", "hex",  OD_ACCESS_RW, OD_UINT32}},
{{0x3001, 0x07}, {"config", "syslog", "hall_enabled_errors",      "hex",  OD_ACCESS_RW, OD_UINT32}},
//##########################################################################################################################################################################################################################################################################################################
{{0x3002, 0x01}, {"config", "drive", "forward_direction", "",       OD_ACCESS_RW, OD_INT32}},
{{0x3002, 0x02}, {"config", "drive", "torque_slope",      "pu/s",   OD_ACCESS_RW, OD_FLOAT32}},
{{0x3002, 0x03}, {"config", "drive", "speed_slope",       "rpm/s",  OD_ACCESS_RW, OD_FLOAT32}},
{{0x3002, 0x04}, {"config", "drive", "runout_speed",      "rpm",    OD_ACCESS_RW, OD_FLOAT32}},
{{0x3002, 0x05}, {"config", "drive", "stopping_torque",   "pu",     OD_ACCESS_RW, OD_FLOAT32}},
{{0x3002, 0x06}, {"config", "drive", "stopping_timeout",  "ms",     OD_ACCESS_RW, OD_UINT32}},
{{0x3002, 0x07}, {"config", "drive", "watchdog_timeout",  "ms",     OD_ACCESS_RW, OD_UINT32}},
//##########################################################################################################################################################################################################################################################################################################
{{0x3003, 0x01}, {"config", "motor", "pole_pairs",  "",     OD_ACCESS_RW, OD_INT32}},
{{0x3003, 0x02}, {"config", "motor", "R",           "Ohm",  OD_ACCESS_RW, OD_FLOAT32}},
{{0x3003, 0x03}, {"config", "motor", "Ld",          "H",    OD_ACCESS_RW, OD_FLOAT32}},
{{0x3003, 0x04}, {"config", "motor", "Lq",          "H",    OD_ACCESS_RW, OD_FLOAT32}},
{{0x3003, 0x05}, {"config", "motor", "psi",         "Wb",   OD_ACCESS_RW, OD_FLOAT32}},
{{0x3003, 0x06}, {"config", "motor", "otp",         "°C",   OD_ACCESS_RW, OD_FLOAT32}},
//##########################################################################################################################################################################################################################################################################################################
{{0x3004, 0x01}, {"config", "converter", "uvp_dc",            "V",  OD_ACCESS_RW, OD_FLOAT32}},
{{0x3004, 0x02}, {"config", "converter", "ovp_dc",            "V",  OD_ACCESS_RW, OD_FLOAT32}},
{{0x3004, 0x03}, {"config", "converter", "ocp_phase",         "A",  OD_ACCESS_RW, OD_FLOAT32}},
{{0x3004, 0x04}, {"config", "converter", "otp_pwr",           "°C", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3004, 0x05}, {"config", "converter", "otp_mcu",           "°C", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3004, 0x06}, {"config", "converter", "active_discharge",  "",   OD_ACCESS_RW, OD_BOOL}},
//##########################################################################################################################################################################################################################################################################################################
{{0x3005, 0x01}, {"config", "model_basic", "iSMot_limit",       "A",    OD_ACCESS_RW, OD_FLOAT32}},
{{0x3005, 0x02}, {"config", "model_basic", "iSGen_limit",       "A",    OD_ACCESS_RW, OD_FLOAT32}},
{{0x3005, 0x03}, {"config", "model_basic", "speed_limit",       "rpm",  OD_ACCESS_RW, OD_FLOAT32}},
{{0x3005, 0x04}, {"config", "model_basic", "angle_correction",  "°",    OD_ACCESS_RW, OD_FLOAT32}},
{{0x3005, 0x05}, {"config", "model_basic", "vD_limit_factor",   "",     OD_ACCESS_RW, OD_FLOAT32}},
//##########################################################################################################################################################################################################################################################################################################
{{0x3006, 0x01}, {"config", "model_controllers", "speed_kP",  "", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3006, 0x02}, {"config", "model_controllers", "speed_kI",  "", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3006, 0x03}, {"config", "model_controllers", "iD_kP",     "", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3006, 0x04}, {"config", "model_controllers", "iD_kI",     "", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3006, 0x05}, {"config", "model_controllers", "iQ_kP",     "", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3006, 0x06}, {"config", "model_controllers", "iQ_kI",     "", OD_ACCESS_RW, OD_FLOAT32}},
//##########################################################################################################################################################################################################################################################################################################
{{0x3007, 0x01}, {"config", "mras", "kP", "", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3007, 0x02}, {"config", "mras", "kI", "", OD_ACCESS_RW, OD_FLOAT32}},
//##########################################################################################################################################################################################################################################################################################################
{{0x3008, 0x01}, {"config", "phase_current_sensor", "gain_a",           "", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3008, 0x02}, {"config", "phase_current_sensor", "gain_b",           "", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3008, 0x03}, {"config", "phase_current_sensor", "gain_c",           "", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3008, 0x04}, {"config", "phase_current_sensor", "offset_a",         "A",OD_ACCESS_RO, OD_FLOAT32}},
{{0x3008, 0x05}, {"config", "phase_current_sensor", "offset_b",         "A",OD_ACCESS_RO, OD_FLOAT32}},
{{0x3008, 0x06}, {"config", "phase_current_sensor", "offset_c",         "A",OD_ACCESS_RO, OD_FLOAT32}},
{{0x3008, 0x07}, {"config", "phase_current_sensor", "offset_threshold", "A",OD_ACCESS_RW, OD_FLOAT32}},
//##########################################################################################################################################################################################################################################################################################################
{{0x3009, 0x01}, {"config", "dc_voltage_sensor",    "gain",             "", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3009, 0x02}, {"config", "dc_voltage_sensor",    "offset",           "", OD_ACCESS_RW, OD_FLOAT32}},
//##########################################################################################################################################################################################################################################################################################################
{{0x300A, 0x01}, {"config", "hall_anglesensor", "enabled",            "",  OD_ACCESS_RW, OD_BOOL}},
{{0x300A, 0x02}, {"config", "hall_anglesensor", "poll_num",           "",  OD_ACCESS_RW, OD_INT32}},
{{0x300A, 0x03}, {"config", "hall_anglesensor", "speed_timeconstant", "s", OD_ACCESS_RW, OD_FLOAT32}},
{{0x300A, 0x04}, {"config", "hall_anglesensor", "sector_a_pos",  "°",  OD_ACCESS_RW, OD_FLOAT32}},
{{0x300A, 0x05}, {"config", "hall_anglesensor", "sector_ab_pos", "°",  OD_ACCESS_RW, OD_FLOAT32}},
{{0x300A, 0x06}, {"config", "hall_anglesensor", "sector_b_pos",  "°",  OD_ACCESS_RW, OD_FLOAT32}},
{{0x300A, 0x07}, {"config", "hall_anglesensor", "sector_bc_pos", "°",  OD_ACCESS_RW, OD_FLOAT32}},
{{0x300A, 0x08}, {"config", "hall_anglesensor", "sector_c_pos",  "°",  OD_ACCESS_RW, OD_FLOAT32}},
{{0x300A, 0x09}, {"config", "hall_anglesensor", "sector_ca_pos", "°",  OD_ACCESS_RW, OD_FLOAT32}},
{{0x300A, 0x0A}, {"config", "hall_anglesensor", "sector_a_neg",  "°",  OD_ACCESS_RW, OD_FLOAT32}},
{{0x300A, 0x0B}, {"config", "hall_anglesensor", "sector_ab_neg", "°",  OD_ACCESS_RW, OD_FLOAT32}},
{{0x300A, 0x0C}, {"config", "hall_anglesensor", "sector_b_neg",  "°",  OD_ACCESS_RW, OD_FLOAT32}},
{{0x300A, 0x0D}, {"config", "hall_anglesensor", "sector_bc_neg", "°",  OD_ACCESS_RW, OD_FLOAT32}},
{{0x300A, 0x0E}, {"config", "hall_anglesensor", "sector_c_neg",  "°",  OD_ACCESS_RW, OD_FLOAT32}},
{{0x300A, 0x0F}, {"config", "hall_anglesensor", "sector_ca_neg", "°",  OD_ACCESS_RW, OD_FLOAT32}},
//##########################################################################################################################################################################################################################################################################################################
{{0x300B, 0x01}, {"config", "hall_emulator", "sector_a_pos",  "°",  OD_ACCESS_RW, OD_FLOAT32}},
{{0x300B, 0x02}, {"config", "hall_emulator", "sector_ab_pos", "°",  OD_ACCESS_RW, OD_FLOAT32}},
{{0x300B, 0x03}, {"config", "hall_emulator", "sector_b_pos",  "°",  OD_ACCESS_RW, OD_FLOAT32}},
{{0x300B, 0x04}, {"config", "hall_emulator", "sector_bc_pos", "°",  OD_ACCESS_RW, OD_FLOAT32}},
{{0x300B, 0x05}, {"config", "hall_emulator", "sector_c_pos",  "°",  OD_ACCESS_RW, OD_FLOAT32}},
{{0x300B, 0x06}, {"config", "hall_emulator", "sector_ca_pos", "°",  OD_ACCESS_RW, OD_FLOAT32}},
{{0x300B, 0x07}, {"config", "hall_emulator", "sector_a_neg",  "°",  OD_ACCESS_RW, OD_FLOAT32}},
{{0x300B, 0x08}, {"config", "hall_emulator", "sector_ab_neg", "°",  OD_ACCESS_RW, OD_FLOAT32}},
{{0x300B, 0x09}, {"config", "hall_emulator", "sector_b_neg",  "°",  OD_ACCESS_RW, OD_FLOAT32}},
{{0x300B, 0x0A}, {"config", "hall_emulator", "sector_bc_neg", "°",  OD_ACCESS_RW, OD_FLOAT32}},
{{0x300B, 0x0B}, {"config", "hall_emulator", "sector_c_neg",  "°",  OD_ACCESS_RW, OD_FLOAT32}},
{{0x300B, 0x0C}, {"config", "hall_emulator", "sector_ca_neg", "°",  OD_ACCESS_RW, OD_FLOAT32}},
//##########################################################################################################################################################################################################################################################################################################
{{0x300C, 0x01}, {"config", "hall_calibrator", "phase_current",       "pu",  OD_ACCESS_RW, OD_FLOAT32}},
{{0x300C, 0x02}, {"config", "hall_calibrator", "phase_current_slope", "pu",  OD_ACCESS_RW, OD_FLOAT32}},
{{0x300C, 0x03}, {"config", "hall_calibrator", "speed",               "rpm", OD_ACCESS_RW, OD_FLOAT32}},
{{0x300C, 0x04}, {"config", "hall_calibrator", "elrevolution_num",    "",    OD_ACCESS_RW, OD_INT32}},
//##########################################################################################################################################################################################################################################################################################################
}
};

} // namespace sevpress
