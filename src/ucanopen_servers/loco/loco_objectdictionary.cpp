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
{{0x5000, 0x13}, {"watch", "model",     "iFRef",        "A",    OD_ACCESS_RO, OD_FLOAT32}},

{{0x5000, 0x20}, {"watch", "possens",   "speed",        "rpm",  OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x21}, {"watch", "possens",   "angle",        "rad",  OD_ACCESS_RO, OD_FLOAT32}},

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
{{0x3002, 0x01}, {"config", "drive", "fwd_dir",                 "",         OD_ACCESS_RO, OD_INT16}},
{{0x3002, 0x02}, {"config", "drive", "watchdog_timeout_ms",     "ms",       OD_ACCESS_RW, OD_UINT32}},
{{0x3002, 0x03}, {"config", "drive", "torque_slope",            "1/s",      OD_ACCESS_RW, OD_FLOAT32}},
{{0x3002, 0x04}, {"config", "drive", "speed_slope",             "rpm/s",    OD_ACCESS_RW, OD_FLOAT32}},
{{0x3002, 0x05}, {"config", "drive", "runout_speed",            "rpm",      OD_ACCESS_RW, OD_FLOAT32}},
//##########################################################################################################################################################################################################################################################################################################
{{0x3003, 0x01}, {"config", "motor", "pole_pairs",              "",         OD_ACCESS_RW, OD_INT16}},
{{0x3003, 0x02}, {"config", "motor", "R",                       "Ohm",      OD_ACCESS_RW, OD_FLOAT32}},
{{0x3003, 0x03}, {"config", "motor", "Ld",                      "H",        OD_ACCESS_RW, OD_FLOAT32}},
{{0x3003, 0x04}, {"config", "motor", "kLd",                     "",         OD_ACCESS_RW, OD_FLOAT32}},
{{0x3003, 0x05}, {"config", "motor", "Lq",                      "H",        OD_ACCESS_RW, OD_FLOAT32}},
{{0x3003, 0x06}, {"config", "motor", "kLq",                     "",         OD_ACCESS_RW, OD_FLOAT32}},
{{0x3003, 0x07}, {"config", "motor", "psi",                     "Wb",       OD_ACCESS_RW, OD_FLOAT32}},
{{0x3003, 0x08}, {"config", "motor", "otp_aw",                  "oC",       OD_ACCESS_RW, OD_FLOAT32}},
{{0x3003, 0x09}, {"config", "motor", "otp_fw",                  "oC",       OD_ACCESS_RW, OD_FLOAT32}},
//##########################################################################################################################################################################################################################################################################################################
{{0x3004, 0x01}, {"config", "converter", "uvp_dc",              "V",        OD_ACCESS_RW, OD_FLOAT32}},
{{0x3004, 0x02}, {"config", "converter", "ovp_dc",              "V",        OD_ACCESS_RW, OD_FLOAT32}},
{{0x3004, 0x03}, {"config", "converter", "ocp_pwrmodule",       "A",        OD_ACCESS_RW, OD_FLOAT32}},
{{0x3004, 0x04}, {"config", "converter", "ocp_excmodule",       "A",        OD_ACCESS_RW, OD_FLOAT32}},
{{0x3004, 0x05}, {"config", "converter", "otp_pwrmodule",       "oC",       OD_ACCESS_RW, OD_FLOAT32}},
{{0x3004, 0x06}, {"config", "converter", "otp_excmodule",       "oC",       OD_ACCESS_RW, OD_FLOAT32}},
{{0x3004, 0x07}, {"config", "converter", "otp_pcb",             "oC",       OD_ACCESS_RW, OD_FLOAT32}},
{{0x3004, 0x08}, {"config", "converter", "field_current_kP",    "",         OD_ACCESS_RW, OD_FLOAT32}},
{{0x3004, 0x09}, {"config", "converter", "field_current_kI",    "",         OD_ACCESS_RW, OD_FLOAT32}},
//##########################################################################################################################################################################################################################################################################################################
{{0x3005, 0x02}, {"config", "model_basic", "iSMaxMot",          "A",        OD_ACCESS_RW, OD_FLOAT32}},
{{0x3005, 0x03}, {"config", "model_basic", "iSMaxGen",          "A",        OD_ACCESS_RW, OD_FLOAT32}},
{{0x3005, 0x04}, {"config", "model_basic", "iFMax",             "A",        OD_ACCESS_RW, OD_FLOAT32}},
{{0x3005, 0x05}, {"config", "model_basic", "torque_posmax",     "pu",       OD_ACCESS_RW, OD_FLOAT32}},
{{0x3005, 0x06}, {"config", "model_basic", "torque_negmax",     "pu",       OD_ACCESS_RW, OD_FLOAT32}},
{{0x3005, 0x07}, {"config", "model_basic", "speed_max",         "rpm",      OD_ACCESS_RW, OD_FLOAT32}},
{{0x3005, 0x08}, {"config", "model_basic", "iDMax_fluxweak",    "A",        OD_ACCESS_RW, OD_FLOAT32}},
{{0x3005, 0x09}, {"config", "model_basic", "iFCoeff",           "",         OD_ACCESS_RW, OD_FLOAT32}},
//##########################################################################################################################################################################################################################################################################################################
{{0x3006, 0x01}, {"config", "model_controllers", "speed_kP", "",            OD_ACCESS_RW, OD_FLOAT32}},
{{0x3006, 0x02}, {"config", "model_controllers", "speed_kI", "",            OD_ACCESS_RW, OD_FLOAT32}},
{{0x3006, 0x03}, {"config", "model_controllers", "iD_kP", "",               OD_ACCESS_RW, OD_FLOAT32}},
{{0x3006, 0x04}, {"config", "model_controllers", "iD_kI", "",               OD_ACCESS_RW, OD_FLOAT32}},
{{0x3006, 0x05}, {"config", "model_controllers", "iQ_kP", "",               OD_ACCESS_RW, OD_FLOAT32}},
{{0x3006, 0x06}, {"config", "model_controllers", "iQ_kI", "",               OD_ACCESS_RW, OD_FLOAT32}},
{{0x3006, 0x07}, {"config", "model_controllers", "fluxweak_kP", "",         OD_ACCESS_RW, OD_FLOAT32}},
{{0x3006, 0x08}, {"config", "model_controllers", "fluxweak_kI", "",         OD_ACCESS_RW, OD_FLOAT32}},
//##########################################################################################################################################################################################################################################################################################################
{{0x3007, 0x01}, {"config", "mras", "kP", "", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3007, 0x02}, {"config", "mras", "kI", "", OD_ACCESS_RW, OD_FLOAT32}},
//##########################################################################################################################################################################################################################################################################################################
{{0x3008, 0x01}, {"config", "precharge", "dclink_charge_threshold", "V", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3008, 0x02}, {"config", "precharge", "dclink_charge_timeout_ms", "ms", OD_ACCESS_RW, OD_UINT32}},
{{0x3008, 0x03}, {"config", "precharge", "charging_contactor_holdup_ms", "ms", OD_ACCESS_RW, OD_UINT32}},
{{0x3008, 0x04}, {"config", "precharge", "dclink_discharge_threshold", "V", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3008, 0x05}, {"config", "precharge", "dclink_discharge_timeout_ms", "ms", OD_ACCESS_RW, OD_UINT32}},
//##########################################################################################################################################################################################################################################################################################################
{{0x3009, 0x01}, {"config", "phase_current_sensor", "phase_a_gain", "", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3009, 0x02}, {"config", "phase_current_sensor", "phase_b_gain", "", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3009, 0x03}, {"config", "phase_current_sensor", "phase_c_gain", "", OD_ACCESS_RW, OD_FLOAT32}},
//##########################################################################################################################################################################################################################################################################################################
{{0x300A, 0x01}, {"config", "field_current_sensor", "gain", "", OD_ACCESS_RW, OD_FLOAT32}},
//##########################################################################################################################################################################################################################################################################################################
{{0x300B, 0x01}, {"config", "dc_voltage_sensor", "gain", "", OD_ACCESS_RW, OD_FLOAT32}},
{{0x300B, 0x02}, {"config", "dc_voltage_sensor", "offset", "", OD_ACCESS_RW, OD_FLOAT32}},

}
};


} // namespace brkdrive
