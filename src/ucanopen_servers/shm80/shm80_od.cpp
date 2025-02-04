#include "shm80_server.h"

namespace shm80 {

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
{{0x1011, 0x04}, {"ctl", "sys", "restore_default_parameter", "", OD_ACCESS_WO, OD_UINT32}},

{{0x1018, 0x04}, {"info", "sys", "serial_number", "", OD_ACCESS_CONST, OD_UINT32}},

{{0x2000, 0x01}, {"ctl", "sys", "reset_device", "", OD_ACCESS_WO, OD_EXEC}},
{{0x2000, 0x02}, {"ctl", "sys", "clear_errors", "", OD_ACCESS_WO, OD_EXEC}},

{{0x2001, 0x01}, {"ctl", "drive", "reset_driver_fault",     "", OD_ACCESS_WO, OD_EXEC}},
{{0x2001, 0x02}, {"ctl", "drive", "calibrate_resolver",     "", OD_ACCESS_WO, OD_EXEC}},
{{0x2001, 0x03}, {"ctl", "drive", "save_resolver_config",   "", OD_ACCESS_WO, OD_EXEC}},

//##########################################################################################################################################################################################################################################################################################################
{{0x5000, 0x01}, {"watch", "sys",       "uptime",       "s",    OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x02}, {"watch", "sys",       "syslogmsg",    "",     OD_ACCESS_RO, OD_UINT32}},

{{0x5000, 0x11}, {"watch", "converter", "vDC",  "V",    OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x12}, {"watch", "converter", "iA",   "A",    OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x13}, {"watch", "converter", "iB",   "A",    OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x14}, {"watch", "converter", "iC",   "A",    OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x15}, {"watch", "converter", "iF",   "A",    OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x16}, {"watch", "converter", "tA",   "o",    OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x17}, {"watch", "converter", "tB",   "o",    OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x18}, {"watch", "converter", "tC",   "o",    OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x19}, {"watch", "converter", "tF",   "o",    OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x1A}, {"watch", "converter", "tPCB", "o",    OD_ACCESS_RO, OD_FLOAT32}},

{{0x5000, 0x21}, {"watch", "model",     "iS",       "A",    OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x22}, {"watch", "model",     "iD",       "A",    OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x23}, {"watch", "model",     "iDRef",    "A",    OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x24}, {"watch", "model",     "iQ",       "A",    OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x25}, {"watch", "model",     "iQRef",    "A",    OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x26}, {"watch", "model",     "iFRef",    "A",    OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x27}, {"watch", "model",     "vD",       "A",    OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x28}, {"watch", "model",     "vQ",       "A",    OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x29}, {"watch", "model",     "vS",       "A",    OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x2A}, {"watch", "model",     "angD",     "o",    OD_ACCESS_RO, OD_FLOAT32}},

{{0x5000, 0x31}, {"watch", "motor",     "tAW",  "o",    OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x32}, {"watch", "motor",     "tFW",  "o",    OD_ACCESS_RO, OD_FLOAT32}},

{{0x5000, 0x41}, {"watch", "drive",     "torqueRef",    "%",    OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x42}, {"watch", "drive",     "torque",       "%",    OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x43}, {"watch", "drive",     "speedRef",     "rpm",  OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x44}, {"watch", "drive",     "speed",        "rpm",  OD_ACCESS_RO, OD_FLOAT32}},

{{0x5000, 0x51}, {"watch", "resolver",  "speed",    "rpm",  OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x52}, {"watch", "resolver",  "angle",    "o",    OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x53}, {"watch", "resolver",  "sinmax",   "V",    OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x54}, {"watch", "resolver",  "sinmin",   "V",    OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x55}, {"watch", "resolver",  "cosmax",   "V",    OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x56}, {"watch", "resolver",  "cosmin",   "V",    OD_ACCESS_RO, OD_FLOAT32}},

//############################################################################################
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
//############################################################################################
{{0x3001, 0x01}, {"config", "syslog", "sys_enabled_errors",         "",     OD_ACCESS_RW, OD_UINT32}},
{{0x3001, 0x02}, {"config", "syslog", "settings_enabled_errors",    "",     OD_ACCESS_RW, OD_UINT32}},
{{0x3001, 0x03}, {"config", "syslog", "can_enabled_errors",         "",     OD_ACCESS_RW, OD_UINT32}},
{{0x3001, 0x04}, {"config", "syslog", "sensors_enabled_errors",     "",     OD_ACCESS_RW, OD_UINT32}},
{{0x3001, 0x05}, {"config", "syslog", "motor_enabled_errors",       "",     OD_ACCESS_RW, OD_UINT32}},
{{0x3001, 0x06}, {"config", "syslog", "converter_enabled_errors",   "",     OD_ACCESS_RW, OD_UINT32}},
{{0x3001, 0x07}, {"config", "syslog", "model_enabled_errors",       "",     OD_ACCESS_RW, OD_UINT32}},
{{0x3001, 0x08}, {"config", "syslog", "resolver_enabled_errors",    "",     OD_ACCESS_RW, OD_UINT32}},
{{0x3001, 0x09}, {"config", "syslog", "pdm_enabled_errors",         "",     OD_ACCESS_RW, OD_UINT32}},
{{0x3001, 0x0A}, {"config", "syslog", "isolation_enabled_errors",   "",     OD_ACCESS_RW, OD_UINT32}},
//##############################################################################
{{0x3002, 0x01}, {"config", "drive", "forward_direction",   "",     OD_ACCESS_RO, OD_INT16}},
{{0x3002, 0x02}, {"config", "drive", "runout_speed",        "rpm",  OD_ACCESS_RW, OD_FLOAT32}},
{{0x3002, 0x03}, {"config", "drive", "stopping_torque",     "pu",   OD_ACCESS_RW, OD_FLOAT32}},
{{0x3002, 0x04}, {"config", "drive", "stopping_timeout",    "ms",   OD_ACCESS_RW, OD_UINT32}},
{{0x3002, 0x05}, {"config", "drive", "torque_slope",        "pu/s", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3002, 0x06}, {"config", "drive", "speed_slope",         "rpm/s",OD_ACCESS_RW, OD_FLOAT32}},
{{0x3002, 0x07}, {"config", "drive", "watchdog_timeout",    "ms",   OD_ACCESS_RW, OD_UINT32}},
//##############################################################################
{{0x3003, 0x01}, {"config", "motor", "parameters_specified","",     OD_ACCESS_RO, OD_BOOL}},
{{0x3003, 0x02}, {"config", "motor", "pole_pairs",          "",     OD_ACCESS_RW, OD_INT16}},
{{0x3003, 0x03}, {"config", "motor", "R",                   "Ohm",  OD_ACCESS_RW, OD_FLOAT32}},
{{0x3003, 0x04}, {"config", "motor", "Ld",                  "H",    OD_ACCESS_RW, OD_FLOAT32}},
{{0x3003, 0x05}, {"config", "motor", "kLd",                 "",     OD_ACCESS_RW, OD_FLOAT32}},
{{0x3003, 0x06}, {"config", "motor", "Lq",                  "H",    OD_ACCESS_RW, OD_FLOAT32}},
{{0x3003, 0x07}, {"config", "motor", "kLq",                 "",     OD_ACCESS_RW, OD_FLOAT32}},
{{0x3003, 0x08}, {"config", "motor", "psi",                 "Wb",   OD_ACCESS_RW, OD_FLOAT32}},
{{0x3003, 0x09}, {"config", "motor", "otp_aw",              "oC",   OD_ACCESS_RW, OD_FLOAT32}},
{{0x3003, 0x0A}, {"config", "motor", "otp_fw",              "oC",   OD_ACCESS_RW, OD_FLOAT32}},
{{0x3003, 0x0B}, {"config", "motor", "fan_temp_on",         "oC",   OD_ACCESS_RW, OD_FLOAT32}},
{{0x3003, 0x0C}, {"config", "motor", "fan_temp_off",        "oC",   OD_ACCESS_RW, OD_FLOAT32}},
//##############################################################################
{{0x3004, 0x01}, {"config", "converter", "uvp_dc",          "V",    OD_ACCESS_RW, OD_FLOAT32}},
{{0x3004, 0x02}, {"config", "converter", "ovp_dc",          "V",    OD_ACCESS_RW, OD_FLOAT32}},
{{0x3004, 0x03}, {"config", "converter", "ocp_phase",       "A",    OD_ACCESS_RW, OD_FLOAT32}},
{{0x3004, 0x04}, {"config", "converter", "ocp_field",       "A",    OD_ACCESS_RW, OD_FLOAT32}},
{{0x3004, 0x05}, {"config", "converter", "otp_phase",       "oC",   OD_ACCESS_RW, OD_FLOAT32}},
{{0x3004, 0x06}, {"config", "converter", "otp_field",       "oC",   OD_ACCESS_RW, OD_FLOAT32}},
{{0x3004, 0x07}, {"config", "converter", "otp_pcb",         "oC",   OD_ACCESS_RW, OD_FLOAT32}},
{{0x3004, 0x08}, {"config", "converter", "fan_temp_on",     "oC",   OD_ACCESS_RW, OD_FLOAT32}},
{{0x3004, 0x09}, {"config", "converter", "fan_temp_off",    "oC",   OD_ACCESS_RW, OD_FLOAT32}},
{{0x3004, 0x0A}, {"config", "converter", "kp_field_current","",     OD_ACCESS_RW, OD_FLOAT32}},
{{0x3004, 0x0B}, {"config", "converter", "ki_field_current","",     OD_ACCESS_RW, OD_FLOAT32}},
{{0x3004, 0x0C}, {"config", "converter", "active_discharge","",     OD_ACCESS_RW, OD_BOOL}},
//##############################################################################
{{0x3005, 0x01}, {"config", "model_basic", "iSMot_limit",       "A",    OD_ACCESS_RW, OD_FLOAT32}},
{{0x3005, 0x02}, {"config", "model_basic", "iSGen_limit",       "A",    OD_ACCESS_RW, OD_FLOAT32}},
{{0x3005, 0x03}, {"config", "model_basic", "iF_limit",          "A",    OD_ACCESS_RW, OD_FLOAT32}},
{{0x3005, 0x04}, {"config", "model_basic", "speed_limit",       "rpm",  OD_ACCESS_RW, OD_FLOAT32}},
{{0x3005, 0x05}, {"config", "model_basic", "iDFluxWeak_limit",  "A",    OD_ACCESS_RW, OD_FLOAT32}},
{{0x3005, 0x06}, {"config", "model_basic", "iF_coeff",          "",     OD_ACCESS_RW, OD_FLOAT32}},
{{0x3005, 0x07}, {"config", "model_basic", "torque_max_speed_limit", "rpm", OD_ACCESS_RW, OD_FLOAT32}},
//##############################################################################
{{0x3006, 0x01}, {"config", "model_controllers", "speed_kP",    "", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3006, 0x02}, {"config", "model_controllers", "speed_kI",    "", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3006, 0x03}, {"config", "model_controllers", "iD_kP",       "", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3006, 0x04}, {"config", "model_controllers", "iD_kI",       "", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3006, 0x05}, {"config", "model_controllers", "iQ_kP",       "", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3006, 0x06}, {"config", "model_controllers", "iQ_kI",       "", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3006, 0x07}, {"config", "model_controllers", "fluxweak_kP", "", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3006, 0x08}, {"config", "model_controllers", "fluxweak_kI", "", OD_ACCESS_RW, OD_FLOAT32}},
//##############################################################################
{{0x3007, 0x01}, {"config", "mras", "kP", "", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3007, 0x02}, {"config", "mras", "kI", "", OD_ACCESS_RW, OD_FLOAT32}},
//##############################################################################
{{0x3008, 0x01}, {"config", "pdm", "dclink_charge_threshold",       "V",    OD_ACCESS_RW, OD_FLOAT32}},
{{0x3008, 0x02}, {"config", "pdm", "bypass_voltage_threshold",      "V",    OD_ACCESS_RW, OD_FLOAT32}},
{{0x3008, 0x03}, {"config", "pdm", "dclink_charge_timeout_ms",      "ms",   OD_ACCESS_RW, OD_UINT32}},
{{0x3008, 0x04}, {"config", "pdm", "charging_contactor_holdup_ms",  "ms",   OD_ACCESS_RW, OD_UINT32}},
{{0x3008, 0x05}, {"config", "pdm", "dclink_discharge_threshold",    "V",    OD_ACCESS_RW, OD_FLOAT32}},
{{0x3008, 0x06}, {"config", "pdm", "dclink_discharge_timeout_ms",   "ms",   OD_ACCESS_RW, OD_UINT32}},
{{0x3008, 0x07}, {"config", "pdm", "switch_timeout_ms",             "ms",   OD_ACCESS_RW, OD_UINT32}},
{{0x3008, 0x08}, {"config", "pdm", "pause_ms",                      "ms",   OD_ACCESS_RW, OD_UINT32}},
//##############################################################################
{{0x3009, 0x01}, {"config", "phase_current_sensor", "gain_a",           "", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3009, 0x02}, {"config", "phase_current_sensor", "gain_b",           "", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3009, 0x03}, {"config", "phase_current_sensor", "gain_c",           "", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3009, 0x04}, {"config", "phase_current_sensor", "offset_a",         "A",OD_ACCESS_RO, OD_FLOAT32}},
{{0x3009, 0x05}, {"config", "phase_current_sensor", "offset_b",         "A",OD_ACCESS_RO, OD_FLOAT32}},
{{0x3009, 0x06}, {"config", "phase_current_sensor", "offset_c",         "A",OD_ACCESS_RO, OD_FLOAT32}},
{{0x3009, 0x07}, {"config", "phase_current_sensor", "offset_threshold", "A",OD_ACCESS_RW, OD_FLOAT32}},
//##############################################################################
{{0x300A, 0x01}, {"config", "field_current_sensor", "gain",             "", OD_ACCESS_RW, OD_FLOAT32}},
{{0x300A, 0x02}, {"config", "field_current_sensor", "offset",           "", OD_ACCESS_RO, OD_FLOAT32}},
{{0x300A, 0x03}, {"config", "field_current_sensor", "offset_threshold", "", OD_ACCESS_RW, OD_FLOAT32}},
//##############################################################################
{{0x300B, 0x01}, {"config", "dc_voltage_sensor",    "gain",             "", OD_ACCESS_RW, OD_FLOAT32}},
{{0x300B, 0x02}, {"config", "dc_voltage_sensor",    "offset",           "", OD_ACCESS_RW, OD_FLOAT32}},
//##############################################################################
{{0x300C, 0x01}, {"config", "resolver", "pole_pairs",               "",     OD_ACCESS_RW, OD_INT16}},
{{0x300C, 0x02}, {"config", "resolver", "install_direction",        "",     OD_ACCESS_RO, OD_INT16}},
{{0x300C, 0x03}, {"config", "resolver", "install_offset",           "rad",  OD_ACCESS_RO, OD_FLOAT32}},
{{0x300C, 0x04}, {"config", "resolver", "observer_natural_freq",    "rad/s",OD_ACCESS_RW, OD_FLOAT32}},
{{0x300C, 0x05}, {"config", "resolver", "observer_damping_factor",  "",     OD_ACCESS_RW, OD_FLOAT32}},
{{0x300C, 0x06}, {"config", "resolver", "exc_factor",               "",     OD_ACCESS_RW, OD_FLOAT32}},
{{0x300C, 0x07}, {"config", "resolver", "sin_offset",               "V",    OD_ACCESS_RO, OD_FLOAT32}},
{{0x300C, 0x08}, {"config", "resolver", "cos_offset",               "V",    OD_ACCESS_RO, OD_FLOAT32}},
//##############################################################################
{{0x300D, 0x01}, {"config", "resolver_calibration", "phase_current",        "pu",   OD_ACCESS_RW, OD_FLOAT32}},
{{0x300D, 0x02}, {"config", "resolver_calibration", "field_current ",       "pu",   OD_ACCESS_RW, OD_FLOAT32}},
{{0x300D, 0x03}, {"config", "resolver_calibration", "dir_detection_angle",  "o",    OD_ACCESS_RW, OD_FLOAT32}},
{{0x300D, 0x04}, {"config", "resolver_calibration", "phase_slope",          "pu/s", OD_ACCESS_RW, OD_FLOAT32}},
{{0x300D, 0x05}, {"config", "resolver_calibration", "field_slope",          "pu/s", OD_ACCESS_RW, OD_FLOAT32}},
{{0x300D, 0x06}, {"config", "resolver_calibration", "angle_slope",          "o",    OD_ACCESS_RW, OD_FLOAT32}},
//##############################################################################
{{0x300E, 0x01}, {"config", "insulation", "enabled",                "",     OD_ACCESS_RW, OD_BOOL}},
{{0x300E, 0x02}, {"config", "insulation", "min_batt_voltage",       "V",    OD_ACCESS_RW, OD_FLOAT32}},
{{0x300E, 0x03}, {"config", "insulation", "testing_period",         "ms",   OD_ACCESS_RW, OD_UINT32}},
{{0x300E, 0x04}, {"config", "insulation", "delay_before_adc",       "ms",   OD_ACCESS_RW, OD_UINT32}},
{{0x300E, 0x05}, {"config", "insulation", "insulation_low_limit",   "Ohm",  OD_ACCESS_RW, OD_FLOAT32}},
{{0x300E, 0x06}, {"config", "insulation", "insulation_pos",         "Ohm",  OD_ACCESS_RO, OD_FLOAT32}},
{{0x300E, 0x07}, {"config", "insulation", "insulation_neg",         "Ohm",  OD_ACCESS_RO, OD_FLOAT32}},
//##############################################################################
{{0x300F, 0x01}, {"config", "throttle_controller", "enabled",           "",     OD_ACCESS_RW, OD_BOOL}},
{{0x300F, 0x02}, {"config", "throttle_controller", "can_id",            "",     OD_ACCESS_RW, OD_UINT32}},
{{0x300F, 0x03}, {"config", "throttle_controller", "timeout",           "ms",   OD_ACCESS_RW, OD_UINT32}},
{{0x300F, 0x04}, {"config", "throttle_controller", "control_mode",      "",     OD_ACCESS_RW, OD_UINT32}},
{{0x300F, 0x05}, {"config", "throttle_controller", "speed_ref_posmin",  "rpm",  OD_ACCESS_RW, OD_FLOAT32}},
{{0x300F, 0x06}, {"config", "throttle_controller", "speed_ref_posmax",  "rpm",  OD_ACCESS_RW, OD_FLOAT32}},
{{0x300F, 0x07}, {"config", "throttle_controller", "speed_ref_negmin",  "rpm",  OD_ACCESS_RW, OD_FLOAT32}},
{{0x300F, 0x08}, {"config", "throttle_controller", "speed_ref_negmax",  "rpm",  OD_ACCESS_RW, OD_FLOAT32}},

}
};

} // namespace shm80
