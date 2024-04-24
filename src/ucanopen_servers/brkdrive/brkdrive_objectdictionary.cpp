#include "brkdrive_server.h"


namespace brkdrive {

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

// {{0x2001, 0x01}, {"ctl", "drive", "startup", "", OD_ACCESS_WO, OD_EXEC}},


{{0x5000, 0x01}, {"watch", "sys", "uptime", "s", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x02}, {"watch", "sys", "syslog_message", "", OD_ACCESS_RO, OD_UINT32}},

{{0x5000, 0x10}, {"watch", "converter", "vDC", "", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x11}, {"watch", "converter", "iA", "", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x12}, {"watch", "converter", "iB", "", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x13}, {"watch", "converter", "iC", "", OD_ACCESS_RO, OD_FLOAT32}},

{{0x5000, 0x20}, {"watch", "model", "iS", "", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x21}, {"watch", "model",     "iD",           "A",    OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x22}, {"watch", "model",     "iQ",           "A",    OD_ACCESS_RO, OD_FLOAT32}},

{{0x5000, 0x30}, {"watch", "angsens",  "mech_angel_d",      "o",    OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x31}, {"watch", "angsens",  "mech_absangle_d",   "o",    OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x32}, {"watch", "angsens",  "elec_angle_d",      "o",    OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x33}, {"watch", "angsens",  "elec_absangle_d",   "o",    OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x34}, {"watch", "angsens",  "speed_d",           "rpm",  OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x35}, {"watch", "angsens",  "turns_d",           "",     OD_ACCESS_RO, OD_INT32}},
{{0x5000, 0x36}, {"watch", "angsens",  "mech_angle_a",      "o",    OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x37}, {"watch", "angsens",  "mech_absangle_a",   "o",    OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x38}, {"watch", "angsens",  "elec_angle_a",      "o",    OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x39}, {"watch", "angsens",  "elec_absangle_a",   "o",    OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x3A}, {"watch", "angsens",  "speed_a",           "rpm",  OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x3B}, {"watch", "angsens",  "turns_a",           "",     OD_ACCESS_RO, OD_INT32}},


{{0x3000, 0x01}, {"config", "ucanopen", "node_id", "", OD_ACCESS_RW, OD_UINT32}},
{{0x3000, 0x02}, {"config", "ucanopen", "heartbeat_period", "ms", OD_ACCESS_RW, OD_UINT32}},
{{0x3000, 0x03}, {"config", "ucanopen", "tpdo1_period", "ms", OD_ACCESS_RW, OD_UINT32}},
{{0x3000, 0x04}, {"config", "ucanopen", "tpdo2_period", "ms", OD_ACCESS_RW, OD_UINT32}},
{{0x3000, 0x05}, {"config", "ucanopen", "tpdo3_period", "ms", OD_ACCESS_RW, OD_UINT32}},
{{0x3000, 0x06}, {"config", "ucanopen", "tpdo4_period", "ms", OD_ACCESS_RW, OD_UINT32}},
{{0x3000, 0x07}, {"config", "ucanopen", "rpdo1_timeout", "ms", OD_ACCESS_RW, OD_UINT32}},
{{0x3000, 0x08}, {"config", "ucanopen", "rpdo2_timeout", "ms", OD_ACCESS_RW, OD_UINT32}},
{{0x3000, 0x09}, {"config", "ucanopen", "rpdo3_timeout", "ms", OD_ACCESS_RW, OD_UINT32}},
{{0x3000, 0x0A}, {"config", "ucanopen", "rpdo4_timeout", "ms", OD_ACCESS_RW, OD_UINT32}},
{{0x3000, 0x0B}, {"config", "ucanopen", "rpdo1_id", "", OD_ACCESS_RW, OD_UINT32}},
{{0x3000, 0x0C}, {"config", "ucanopen", "rpdo2_id", "", OD_ACCESS_RW, OD_UINT32}},
{{0x3000, 0x0D}, {"config", "ucanopen", "rpdo3_id", "", OD_ACCESS_RW, OD_UINT32}},
{{0x3000, 0x0E}, {"config", "ucanopen", "rpdo4_id", "", OD_ACCESS_RW, OD_UINT32}},

{{0x3001, 0x01}, {"config", "syslog", "enabled_errors", "", OD_ACCESS_RW, OD_UINT32}},
{{0x3001, 0x02}, {"config", "syslog", "fatal_errors", "", OD_ACCESS_RW, OD_UINT32}},

{{0x3002, 0x01}, {"config", "drive", "fwd_rotation_direction", "", OD_ACCESS_RO, OD_INT16}},
{{0x3002, 0x02}, {"config", "drive", "cal_stator_current", "A", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3002, 0x03}, {"config", "drive", "cal_field_current", "A", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3002, 0x04}, {"config", "drive", "cal_speed_rpm", "rpm", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3002, 0x05}, {"config", "drive", "cal_di_dt", "", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3002, 0x06}, {"config", "drive", "cal_accel", "", OD_ACCESS_RW, OD_FLOAT32}},

{{0x3003, 0x01}, {"config", "motor", "parameters_specified", "", OD_ACCESS_RO, OD_BOOL}},
{{0x3003, 0x02}, {"config", "motor", "pole_pairs", "", OD_ACCESS_RW, OD_INT16}},
{{0x3003, 0x03}, {"config", "motor", "R", "Ω", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3003, 0x04}, {"config", "motor", "Ld", "H", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3003, 0x05}, {"config", "motor", "kLd", "", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3003, 0x06}, {"config", "motor", "Lq", "H", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3003, 0x07}, {"config", "motor", "kLq", "", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3003, 0x08}, {"config", "motor", "psi", "Wb", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3003, 0x09}, {"config", "motor", "otp_aw", "°C", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3003, 0x0A}, {"config", "motor", "otp_fw", "°C", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3003, 0x0B}, {"config", "motor", "fan_temp_on", "°C", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3003, 0x0C}, {"config", "motor", "fan_temp_off", "°C", OD_ACCESS_RW, OD_FLOAT32}},

{{0x3004, 0x01}, {"config", "converter", "uvp_dc", "V", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3004, 0x02}, {"config", "converter", "ovp_dc", "V", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3004, 0x03}, {"config", "converter", "ocp_pwrmodule", "A", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3004, 0x04}, {"config", "converter", "ocp_excmodule", "A", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3004, 0x05}, {"config", "converter", "otp_pwrmodule", "°C", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3004, 0x06}, {"config", "converter", "otp_excmodule", "°C", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3004, 0x07}, {"config", "converter", "otp_pcb", "°C", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3004, 0x08}, {"config", "converter", "fan_temp_on", "°C", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3004, 0x09}, {"config", "converter", "fan_temp_off", "°C", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3004, 0x0A}, {"config", "converter", "kp_exc_current", "", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3004, 0x0B}, {"config", "converter", "ki_exc_current", "", OD_ACCESS_RW, OD_FLOAT32}},

{{0x3005, 0x01}, {"config", "model_basic", "reference", "", OD_ACCESS_RO, OD_UINT32}},
{{0x3005, 0x02}, {"config", "model_basic", "i_smax_motor", "A", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3005, 0x03}, {"config", "model_basic", "i_smax_gener", "A", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3005, 0x04}, {"config", "model_basic", "i_fmax", "A", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3005, 0x05}, {"config", "model_basic", "torque_posmax", "pu", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3005, 0x06}, {"config", "model_basic", "torque_negmax", "pu", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3005, 0x07}, {"config", "model_basic", "speed_rpmmax", "rpm", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3005, 0x08}, {"config", "model_basic", "i_dmax_fluxweak", "A", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3005, 0x09}, {"config", "model_basic", "i_f_coeff", "", OD_ACCESS_RW, OD_FLOAT32}},

{{0x3006, 0x01}, {"config", "model_controllers", "kp_speed", "", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3006, 0x02}, {"config", "model_controllers", "ki_speed", "", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3006, 0x03}, {"config", "model_controllers", "kp_id", "", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3006, 0x04}, {"config", "model_controllers", "ki_id", "", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3006, 0x05}, {"config", "model_controllers", "kp_iq", "", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3006, 0x06}, {"config", "model_controllers", "ki_iq", "", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3006, 0x07}, {"config", "model_controllers", "kp_fluxweak", "", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3006, 0x08}, {"config", "model_controllers", "ki_fluxweak", "", OD_ACCESS_RW, OD_FLOAT32}},

{{0x3007, 0x01}, {"config", "mras", "kp", "", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3007, 0x02}, {"config", "mras", "ki", "", OD_ACCESS_RW, OD_FLOAT32}},

{{0x3008, 0x01}, {"config", "resolver", "pole_pairs", "", OD_ACCESS_RW, OD_INT16}},
{{0x3008, 0x02}, {"config", "resolver", "install_direction", "", OD_ACCESS_RO, OD_INT16}},
{{0x3008, 0x03}, {"config", "resolver", "install_offset", "rad", OD_ACCESS_RO, OD_FLOAT32}},
{{0x3008, 0x04}, {"config", "resolver", "observer_natural_freq", "rad/s", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3008, 0x05}, {"config", "resolver", "observer_damping_factor", "", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3008, 0x06}, {"config", "resolver", "exc_factor", "", OD_ACCESS_RW, OD_FLOAT32}},

{{0x3009, 0x01}, {"config", "precharge", "dclink_charge_threshold", "V", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3009, 0x02}, {"config", "precharge", "dclink_charge_timeout_ms", "ms", OD_ACCESS_RW, OD_UINT32}},
{{0x3009, 0x03}, {"config", "precharge", "charging_contactor_holdup_ms", "ms", OD_ACCESS_RW, OD_UINT32}},
{{0x3009, 0x04}, {"config", "precharge", "dclink_discharge_threshold", "V", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3009, 0x05}, {"config", "precharge", "dclink_discharge_timeout_ms", "ms", OD_ACCESS_RW, OD_UINT32}},

{{0x300A, 0x01}, {"config", "insulation", "min_batt_voltage", "V", OD_ACCESS_RW, OD_FLOAT32}},
{{0x300A, 0x02}, {"config", "insulation", "testing_period_ms", "ms", OD_ACCESS_RW, OD_UINT32}},
{{0x300A, 0x03}, {"config", "insulation", "delay_before_adc_ms", "ms", OD_ACCESS_RW, OD_UINT32}},
{{0x300A, 0x04}, {"config", "insulation", "insulation_low_limit", "Ω", OD_ACCESS_RW, OD_FLOAT32}},

{{0x1A00, 0x00}, {}},
{{0x1A01, 0x00}, {}},
{{0x1A01, 0x01}, {"tpdo2", "converter", "vDC", "V", OD_ACCESS_RO, OD_UINT16}},
{{0x1A01, 0x02}, {"tpdo2", "model", "iS", "A", OD_ACCESS_RO, OD_UINT16}},

}
};


} // namespace brkdrive
