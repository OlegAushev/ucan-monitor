#include "loco_server.h"


namespace loco {

using namespace ucanopen;


extern const ucanopen::ObjectDictionary object_dictionary = {
.config = {
    .watch_category = "watch",
    .config_category = "config"
},
.entries = {
{{0x1008, 0x00}, {"info", "sys", "device_name", "", OD_ACCESS_CONST, OD_STRING}},
{{0x1009, 0x00}, {"info", "sys", "hardware_version", "", OD_ACCESS_CONST, OD_STRING}},
{{0x100A, 0x00}, {"info", "sys", "firmware_version", "", OD_ACCESS_CONST, OD_STRING}},

{{0x1010, 0x01}, {"ctl", "sys", "save_all_parameters", "", OD_ACCESS_WO, OD_EXEC}},
{{0x1011, 0x01}, {"ctl", "sys", "restore_all_default_parameters", "", OD_ACCESS_WO, OD_EXEC}},
{{0x1011, 0x04}, {"ctl", "sys", "restore_default_parameter", "", OD_ACCESS_WO, OD_UINT32}},

{{0x1018, 0x04}, {"info", "sys", "serial_number", "", OD_ACCESS_CONST, OD_UINT32}},

{{0x2000, 0x01}, {"ctl", "sys", "reset_device", "", OD_ACCESS_WO, OD_EXEC}},
{{0x2000, 0x02}, {"ctl", "sys", "clear_errors", "", OD_ACCESS_WO, OD_EXEC}},


{{0x2001, 0x01}, {"ctl", "drive", "phi_1", "rad", OD_ACCESS_RO, OD_FLOAT32}},
{{0x2001, 0x02}, {"ctl", "drive", "phi_2", "rad", OD_ACCESS_RO, OD_FLOAT32}},
{{0x2001, 0x03}, {"ctl", "drive", "phi_3", "rad", OD_ACCESS_RO, OD_FLOAT32}},


{{0x5000, 0x01}, {"watch", "sys", "uptime", "s", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x02}, {"watch", "sys", "syslog_message", "", OD_ACCESS_RO, OD_UINT32}},
{{0x5000, 0x03}, {"watch", "converter", "vDC",          "V",    OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x04}, {"watch", "converter", "iA1",          "A",    OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x05}, {"watch", "converter", "iB1",          "A",    OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x06}, {"watch", "converter", "iC1",          "A",    OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x07}, {"watch", "converter", "iA2",          "A",    OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x08}, {"watch", "converter", "iB2",          "A",    OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x09}, {"watch", "converter", "iC2",          "A",    OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x0A}, {"watch", "converter", "iA3",          "A",    OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x0B}, {"watch", "converter", "iB3",          "A",    OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x0C}, {"watch", "converter", "iC3",          "A",    OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x0D}, {"watch", "converter", "iF",           "A",    OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x0E}, {"watch", "model",     "iS",           "A",    OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x0F}, {"watch", "model",     "iD",           "A",    OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x10}, {"watch", "model",     "iDRef",        "A",    OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x11}, {"watch", "model",     "iQ",           "A",    OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x12}, {"watch", "model",     "iQRef",        "A",    OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x13}, {"watch", "possens",   "speed",        "rpm",  OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x14}, {"watch", "possens",   "angle",        "rad",  OD_ACCESS_RO, OD_FLOAT32}},

{{0x5000, 0xF1}, {"watch",  "ref",      "torque",       "",     OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0xF2}, {"watch",  "ref",      "speed",        "",     OD_ACCESS_RO, OD_FLOAT32}},


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
{{0x3001, 0x01}, {"config", "syslog", "enabled_errors",     "",     OD_ACCESS_RW, OD_UINT32}},
{{0x3001, 0x02}, {"config", "syslog", "fatal_errors",       "",     OD_ACCESS_RW, OD_UINT32}},
//#####################################################################################################
{{0x3002, 0x01}, {"config", "drive", "fwd_rotation_direction",  "",         OD_ACCESS_RW, OD_INT32}},
// {{0x3002, 0x02}, {"config", "drive", "brk_braking_speed",       "rpm",      OD_ACCESS_RW, OD_FLOAT32}},
// {{0x3002, 0x03}, {"config", "drive", "brk_start_current",       "A",        OD_ACCESS_RW, OD_FLOAT32}},
// {{0x3002, 0x04}, {"config", "drive", "brk_speed_threshold",     "rad/s",    OD_ACCESS_RW, OD_FLOAT32}},
// {{0x3002, 0x05}, {"config", "drive", "brk_duration_threshold",  "ms",       OD_ACCESS_RW, OD_UINT32}},
// {{0x3002, 0x06}, {"config", "drive", "rls_speed_limit",         "rpm",      OD_ACCESS_RW, OD_FLOAT32}},
// {{0x3002, 0x07}, {"config", "drive", "rls_angle_kp",            "",         OD_ACCESS_RW, OD_FLOAT32}},
// {{0x3002, 0x08}, {"config", "drive", "rls_current_limit",       "A",        OD_ACCESS_RW, OD_FLOAT32}},
// {{0x3002, 0x09}, {"config", "drive", "rls_release_diffangle",   "rad",      OD_ACCESS_RW, OD_FLOAT32}},
{{0x3002, 0x0A}, {"config", "drive", "trk_speed_limit",         "rpm",      OD_ACCESS_RW, OD_FLOAT32}},
{{0x3002, 0x0B}, {"config", "drive", "trk_angle_kp",            "",         OD_ACCESS_RW, OD_FLOAT32}},
{{0x3002, 0x0C}, {"config", "drive", "trk_angle_threshold",     "rad",      OD_ACCESS_RW, OD_FLOAT32}},
{{0x3002, 0x0D}, {"config", "drive", "trk_rad_per_mm",          "rad/mm",   OD_ACCESS_RW, OD_FLOAT32}},
{{0x3002, 0x0E}, {"config", "drive", "watchdog_timeout",        "ms",       OD_ACCESS_RW, OD_UINT32}},
{{0x3002, 0x0F}, {"config", "drive", "torque_slope",            "pu/s",     OD_ACCESS_RW, OD_FLOAT32}},
{{0x3002, 0x10}, {"config", "drive", "speed_slope",             "rpm/s",    OD_ACCESS_RW, OD_FLOAT32}},
{{0x3002, 0x11}, {"config", "drive", "angle_slope",             "rad/s",    OD_ACCESS_RW, OD_FLOAT32}},
//#################################################################################################
{{0x3003, 0x01}, {"config", "motor", "parameters_specified",    "",     OD_ACCESS_RO, OD_BOOL}},
{{0x3003, 0x02}, {"config", "motor", "pole_pairs",              "",     OD_ACCESS_RW, OD_INT32}},
{{0x3003, 0x03}, {"config", "motor", "R",                       "Ohm",  OD_ACCESS_RW, OD_FLOAT32}},
{{0x3003, 0x04}, {"config", "motor", "Ld",                      "H",    OD_ACCESS_RW, OD_FLOAT32}},
{{0x3003, 0x05}, {"config", "motor", "Lq",                      "H",    OD_ACCESS_RW, OD_FLOAT32}},
{{0x3003, 0x06}, {"config", "motor", "psi",                     "Wb",   OD_ACCESS_RW, OD_FLOAT32}},
{{0x3003, 0x07}, {"config", "motor", "otp",                     "oC",   OD_ACCESS_RW, OD_FLOAT32}},
//#####################################################################################
{{0x3004, 0x01}, {"config", "converter", "uvp_dc",  "V",    OD_ACCESS_RW, OD_FLOAT32}},
{{0x3004, 0x02}, {"config", "converter", "ovp_dc",  "V",    OD_ACCESS_RW, OD_FLOAT32}},
{{0x3004, 0x03}, {"config", "converter", "ocp_ac",  "A",    OD_ACCESS_RW, OD_FLOAT32}},
{{0x3004, 0x04}, {"config", "converter", "otp_ph",  "oC",   OD_ACCESS_RW, OD_FLOAT32}},
{{0x3004, 0x05}, {"config", "converter", "otp_pcb", "oC",   OD_ACCESS_RW, OD_FLOAT32}},
//#####################################################################################################
{{0x3005, 0x01}, {"config", "model_basic", "motor_current_limit",   "A",    OD_ACCESS_RW, OD_FLOAT32}},
{{0x3005, 0x02}, {"config", "model_basic", "gener_current_limit",   "A",    OD_ACCESS_RW, OD_FLOAT32}},
{{0x3005, 0x03}, {"config", "model_basic", "speed_limit",           "rpm",  OD_ACCESS_RW, OD_FLOAT32}},
{{0x3005, 0x04}, {"config", "model_basic", "flux_weakening_limit",  "A",    OD_ACCESS_RW, OD_FLOAT32}},
{{0x3005, 0x05}, {"config", "model_basic", "mras_enable_speed",     "rpm",  OD_ACCESS_RW, OD_FLOAT32}},
//#############################################################################################
{{0x3006, 0x01}, {"config", "model_controllers", "speed_kp",        "", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3006, 0x02}, {"config", "model_controllers", "speed_ki",        "", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3006, 0x03}, {"config", "model_controllers", "speed_limit_kp",  "", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3006, 0x04}, {"config", "model_controllers", "id_kp",           "", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3006, 0x05}, {"config", "model_controllers", "id_ki",           "", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3006, 0x06}, {"config", "model_controllers", "iq_kp",           "", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3006, 0x07}, {"config", "model_controllers", "iq_ki",           "", OD_ACCESS_RW, OD_FLOAT32}},
//#######################################################################
{{0x3007, 0x01}, {"config", "mras", "kp", "", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3007, 0x02}, {"config", "mras", "ki", "", OD_ACCESS_RW, OD_FLOAT32}},
//#############################################################################################
{{0x3008, 0x01}, {"config", "ac_current_sensors", "phase_a_gain",      "",     OD_ACCESS_RW, OD_FLOAT32}},
{{0x3008, 0x02}, {"config", "ac_current_sensors", "phase_b_gain",      "",     OD_ACCESS_RW, OD_FLOAT32}},
{{0x3008, 0x03}, {"config", "ac_current_sensors", "phase_c_gain",      "",     OD_ACCESS_RW, OD_FLOAT32}},
//##########################################################################################################################################################################################################################################################################################################
{{0x3009, 0x01}, {"config", "angle_sensor", "install_direction",                "",     OD_ACCESS_RW, OD_INT32}},
{{0x3009, 0x02}, {"config", "angle_sensor", "install_offset",                   "rad",  OD_ACCESS_RW, OD_FLOAT32}},
{{0x3009, 0x03}, {"config", "angle_sensor", "spi_speed_filter_time_constant",   "",     OD_ACCESS_RW, OD_FLOAT32}},
{{0x3009, 0x04}, {"config", "angle_sensor", "adc_speed_filter_time_constant",   "",     OD_ACCESS_RW, OD_FLOAT32}},
//##########################################################################################################################################################################################################################################################################################################
{{0x300A, 0x01}, {"config", "calibration", "touch_current",                 "A",        OD_ACCESS_RW, OD_FLOAT32}},
{{0x300A, 0x02}, {"config", "calibration", "clamp_current",                 "A",        OD_ACCESS_RW, OD_FLOAT32}},
{{0x300A, 0x03}, {"config", "calibration", "brk_speed",                     "rpm",      OD_ACCESS_RW, OD_FLOAT32}},
{{0x300A, 0x04}, {"config", "calibration", "rls_speed",                     "rpm",      OD_ACCESS_RW, OD_FLOAT32}},
{{0x300A, 0x05}, {"config", "calibration", "brk_start_current",             "A",        OD_ACCESS_RW, OD_FLOAT32}},
{{0x300A, 0x06}, {"config", "calibration", "brk_start_duration",            "ms",       OD_ACCESS_RW, OD_UINT32}},
{{0x300A, 0x07}, {"config", "calibration", "brk_touch_detection_speed",     "rpm",      OD_ACCESS_RW, OD_FLOAT32}},
{{0x300A, 0x08}, {"config", "calibration", "brk_touch_detection_start",     "ms",       OD_ACCESS_RW, OD_UINT32}},
{{0x300A, 0x09}, {"config", "calibration", "rls_distance",                  "mm",       OD_ACCESS_RW, OD_FLOAT32}},
{{0x300A, 0x0A}, {"config", "calibration", "stage1_max_duration",           "ms",       OD_ACCESS_RW, OD_UINT32}},
{{0x300A, 0x0B}, {"config", "calibration", "stage2_max_duration",           "ms",       OD_ACCESS_RW, OD_UINT32}},
{{0x300A, 0x0C}, {"config", "calibration", "stage3_max_duration",           "ms",       OD_ACCESS_RW, OD_UINT32}},
{{0x300A, 0x0D}, {"config", "calibration", "stage4_max_duration",           "ms",       OD_ACCESS_RW, OD_UINT32}},
{{0x300A, 0x0E}, {"config", "calibration", "stage5_max_duration",           "ms",       OD_ACCESS_RW, OD_UINT32}},


// {{0x1A00, 0x00}, {}},
// {{0x1A01, 0x00}, {}},
// {{0x1A01, 0x01}, {"tpdo2", "converter", "vDC", "V", OD_ACCESS_RO, OD_UINT16}},
// {{0x1A01, 0x02}, {"tpdo2", "model", "iS", "A", OD_ACCESS_RO, OD_UINT16}},

}
};


} // namespace brkdrive