#include "srmdrive_server.h"


namespace srmdrive {

using namespace ucanopen;


extern const ucanopen::ObjectDictionary object_dictionary = {
.config = {
    .watch_category = "watch",
    .config_category = "config"
},
.entries = {
{{0x1008, 0x00}, {"info", "sys", "device_name", "", OD_STRING, OD_ACCESS_CONST}},
{{0x1009, 0x00}, {"info", "sys", "hardware_version", "", OD_STRING, OD_ACCESS_CONST}},
{{0x100A, 0x00}, {"info", "sys", "firmware_version", "", OD_STRING, OD_ACCESS_CONST}},

{{0x1010, 0x01}, {"ctl", "sys", "save_all_parameters", "", OD_EXEC, OD_ACCESS_WO}},
{{0x1011, 0x01}, {"ctl", "sys", "restore_all_default_parameters", "", OD_EXEC, OD_ACCESS_WO}},

{{0x1018, 0x04}, {"info", "sys", "serial_number", "", OD_UINT32, OD_ACCESS_CONST}},

{{0x2000, 0x01}, {"ctl", "sys", "reset_device", "", OD_EXEC, OD_ACCESS_WO}},
{{0x2000, 0x02}, {"ctl", "sys", "clear_errors", "", OD_EXEC, OD_ACCESS_WO}},

// {{0x2001, 0x01}, {"ctl", "drive", "startup", "", OD_EXEC, OD_ACCESS_WO}},
// {{0x2001, 0x02}, {"ctl", "drive", "shutdown", "", OD_EXEC, OD_ACCESS_WO}},
{{0x2001, 0x03}, {"ctl", "drive", "calibrate", "", OD_EXEC, OD_ACCESS_WO}},
{{0x2001, 0x04}, {"ctl", "drive", "invert_rotdir", "", OD_EXEC, OD_ACCESS_WO}},
// {{0x2001, 0x05}, {"ctl", "drive", "enable_torque_ref", "", OD_EXEC, OD_ACCESS_WO}},
// {{0x2001, 0x06}, {"ctl", "drive", "enable_speed_ref", "", OD_EXEC, OD_ACCESS_WO}},
// {{0x2001, 0x07}, {"ctl", "drive", "enable_manual_field", "", OD_EXEC, OD_ACCESS_WO}},
// {{0x2001, 0x08}, {"ctl", "drive", "disable_manual_field", "", OD_EXEC, OD_ACCESS_WO}},
// {{0x2001, 0x09}, {"ctl", "drive", "set_field_current", "", OD_FLOAT32, OD_ACCESS_WO}},
// {{0x2001, 0x0A}, {"ctl", "drive", "enable_open_loop", "", OD_EXEC, OD_ACCESS_WO}},
// {{0x2001, 0x0B}, {"ctl", "drive", "enable_closed_loop", "", OD_EXEC, OD_ACCESS_WO}},
// {{0x2001, 0x0C}, {"ctl", "drive", "set_current", "", OD_FLOAT32, OD_ACCESS_WO}},
{{0x2001, 0x0D}, {"ctl", "drive", "set_gamma_correction", "", OD_FLOAT32, OD_ACCESS_WO}},
{{0x2001, 0x0E}, {"ctl", "drive", "reset_driver_fault", "", OD_EXEC, OD_ACCESS_WO}},



{{0x5000, 0x01}, {"watch", "sys", "uptime", "s", OD_FLOAT32, OD_ACCESS_RO}},
{{0x5000, 0x02}, {"watch", "sys", "syslog_message", "", OD_UINT32, OD_ACCESS_RO}},
{{0x5000, 0x03}, {"watch", "drive", "voltage_dc", "", OD_FLOAT32, OD_ACCESS_RO}},
{{0x5000, 0x04}, {"watch", "drive", "current_pha", "", OD_FLOAT32, OD_ACCESS_RO}},
{{0x5000, 0x05}, {"watch", "drive", "current_phb", "", OD_FLOAT32, OD_ACCESS_RO}},
{{0x5000, 0x06}, {"watch", "drive", "current_phc", "", OD_FLOAT32, OD_ACCESS_RO}},
{{0x5000, 0x07}, {"watch", "drive", "current_f", "", OD_FLOAT32, OD_ACCESS_RO}},


{{0x3000, 0x01}, {"config", "ucanopen", "node_id", "", OD_UINT32, OD_ACCESS_RW}},
{{0x3000, 0x02}, {"config", "ucanopen", "heartbeat_period", "ms", OD_UINT32, OD_ACCESS_RW}},
{{0x3000, 0x03}, {"config", "ucanopen", "tpdo1_period", "ms", OD_UINT32, OD_ACCESS_RW}},
{{0x3000, 0x04}, {"config", "ucanopen", "tpdo2_period", "ms", OD_UINT32, OD_ACCESS_RW}},
{{0x3000, 0x05}, {"config", "ucanopen", "tpdo3_period", "ms", OD_UINT32, OD_ACCESS_RW}},
{{0x3000, 0x06}, {"config", "ucanopen", "tpdo4_period", "ms", OD_UINT32, OD_ACCESS_RW}},
{{0x3000, 0x07}, {"config", "ucanopen", "rpdo1_timeout", "ms", OD_UINT32, OD_ACCESS_RW}},
{{0x3000, 0x08}, {"config", "ucanopen", "rpdo2_timeout", "ms", OD_UINT32, OD_ACCESS_RW}},
{{0x3000, 0x09}, {"config", "ucanopen", "rpdo3_timeout", "ms", OD_UINT32, OD_ACCESS_RW}},
{{0x3000, 0x0A}, {"config", "ucanopen", "rpdo4_timeout", "ms", OD_UINT32, OD_ACCESS_RW}},
{{0x3000, 0x0B}, {"config", "ucanopen", "rpdo1_id", "", OD_UINT32, OD_ACCESS_RW}},
{{0x3000, 0x0C}, {"config", "ucanopen", "rpdo2_id", "", OD_UINT32, OD_ACCESS_RW}},
{{0x3000, 0x0D}, {"config", "ucanopen", "rpdo3_id", "", OD_UINT32, OD_ACCESS_RW}},
{{0x3000, 0x0E}, {"config", "ucanopen", "rpdo4_id", "", OD_UINT32, OD_ACCESS_RW}},

{{0x3001, 0x01}, {"config", "syslog", "enabled_errors", "", OD_UINT32, OD_ACCESS_RW}},
{{0x3001, 0x02}, {"config", "syslog", "fatal_errors", "", OD_UINT32, OD_ACCESS_RW}},

{{0x3002, 0x01}, {"config", "drive", "fwd_rotation_direction", "", OD_INT16, OD_ACCESS_RO}},
{{0x3002, 0x02}, {"config", "drive", "cal_stator_current", "A", OD_FLOAT32, OD_ACCESS_RW}},
{{0x3002, 0x03}, {"config", "drive", "cal_field_current", "A", OD_FLOAT32, OD_ACCESS_RW}},
{{0x3002, 0x04}, {"config", "drive", "cal_speed_rpm", "rpm", OD_FLOAT32, OD_ACCESS_RW}},
{{0x3002, 0x05}, {"config", "drive", "cal_di_dt", "", OD_FLOAT32, OD_ACCESS_RW}},
{{0x3002, 0x06}, {"config", "drive", "cal_accel", "", OD_FLOAT32, OD_ACCESS_RW}},

{{0x3003, 0x01}, {"config", "motor", "parameters_specified", "", OD_BOOL, OD_ACCESS_RO}},
{{0x3003, 0x02}, {"config", "motor", "pole_pairs", "", OD_INT16, OD_ACCESS_RW}},
{{0x3003, 0x03}, {"config", "motor", "R", "Ω", OD_FLOAT32, OD_ACCESS_RW}},
{{0x3003, 0x04}, {"config", "motor", "Ld", "H", OD_FLOAT32, OD_ACCESS_RW}},
{{0x3003, 0x05}, {"config", "motor", "kLd", "", OD_FLOAT32, OD_ACCESS_RW}},
{{0x3003, 0x06}, {"config", "motor", "Lq", "H", OD_FLOAT32, OD_ACCESS_RW}},
{{0x3003, 0x07}, {"config", "motor", "kLq", "", OD_FLOAT32, OD_ACCESS_RW}},
{{0x3003, 0x08}, {"config", "motor", "psi", "Wb", OD_FLOAT32, OD_ACCESS_RW}},
{{0x3003, 0x09}, {"config", "motor", "otp_aw", "°C", OD_FLOAT32, OD_ACCESS_RW}},
{{0x3003, 0x0A}, {"config", "motor", "otp_fw", "°C", OD_FLOAT32, OD_ACCESS_RW}},
{{0x3003, 0x0B}, {"config", "motor", "fan_temp_on", "°C", OD_FLOAT32, OD_ACCESS_RW}},
{{0x3003, 0x0C}, {"config", "motor", "fan_temp_off", "°C", OD_FLOAT32, OD_ACCESS_RW}},

{{0x3004, 0x01}, {"config", "converter", "uvp_dc", "V", OD_FLOAT32, OD_ACCESS_RW}},
{{0x3004, 0x02}, {"config", "converter", "ovp_dc", "V", OD_FLOAT32, OD_ACCESS_RW}},
{{0x3004, 0x03}, {"config", "converter", "ocp_pwrmodule", "A", OD_FLOAT32, OD_ACCESS_RW}},
{{0x3004, 0x04}, {"config", "converter", "ocp_excmodule", "A", OD_FLOAT32, OD_ACCESS_RW}},
{{0x3004, 0x05}, {"config", "converter", "otp_pwrmodule", "°C", OD_FLOAT32, OD_ACCESS_RW}},
{{0x3004, 0x06}, {"config", "converter", "otp_excmodule", "°C", OD_FLOAT32, OD_ACCESS_RW}},
{{0x3004, 0x07}, {"config", "converter", "otp_pcb", "°C", OD_FLOAT32, OD_ACCESS_RW}},
{{0x3004, 0x08}, {"config", "converter", "fan_temp_on", "°C", OD_FLOAT32, OD_ACCESS_RW}},
{{0x3004, 0x09}, {"config", "converter", "fan_temp_off", "°C", OD_FLOAT32, OD_ACCESS_RW}},
{{0x3004, 0x0A}, {"config", "converter", "kp_exc_current", "", OD_FLOAT32, OD_ACCESS_RW}},
{{0x3004, 0x0B}, {"config", "converter", "ki_exc_current", "", OD_FLOAT32, OD_ACCESS_RW}},

{{0x3005, 0x01}, {"config", "model_basic", "reference", "", OD_UINT32, OD_ACCESS_RO}},
{{0x3005, 0x02}, {"config", "model_basic", "is_motor_max", "A", OD_FLOAT32, OD_ACCESS_RW}},
{{0x3005, 0x03}, {"config", "model_basic", "is_gener_max", "A", OD_FLOAT32, OD_ACCESS_RW}},
{{0x3005, 0x04}, {"config", "model_basic", "if_max", "A", OD_FLOAT32, OD_ACCESS_RW}},
{{0x3005, 0x05}, {"config", "model_basic", "torque_pos_max", "Nm", OD_FLOAT32, OD_ACCESS_RW}},
{{0x3005, 0x06}, {"config", "model_basic", "torque_neg_max", "Nm", OD_FLOAT32, OD_ACCESS_RW}},
{{0x3005, 0x07}, {"config", "model_basic", "speed_rpm_max", "rpm", OD_FLOAT32, OD_ACCESS_RW}},
{{0x3005, 0x08}, {"config", "model_basic", "id_max_flux_weakening", "A", OD_FLOAT32, OD_ACCESS_RW}},
{{0x3005, 0x09}, {"config", "model_basic", "gamma_correction", "rad", OD_FLOAT32, OD_ACCESS_RW}},

{{0x3006, 0x01}, {"config", "model_controllers", "kp_speed", "", OD_FLOAT32, OD_ACCESS_RW}},
{{0x3006, 0x02}, {"config", "model_controllers", "ki_speed", "", OD_FLOAT32, OD_ACCESS_RW}},
{{0x3006, 0x03}, {"config", "model_controllers", "kp_id", "", OD_FLOAT32, OD_ACCESS_RW}},
{{0x3006, 0x04}, {"config", "model_controllers", "ki_id", "", OD_FLOAT32, OD_ACCESS_RW}},
{{0x3006, 0x05}, {"config", "model_controllers", "kp_iq", "", OD_FLOAT32, OD_ACCESS_RW}},
{{0x3006, 0x06}, {"config", "model_controllers", "ki_iq", "", OD_FLOAT32, OD_ACCESS_RW}},
{{0x3006, 0x07}, {"config", "model_controllers", "kp_flux_weakening", "", OD_FLOAT32, OD_ACCESS_RW}},
{{0x3006, 0x08}, {"config", "model_controllers", "ki_flux_weakening", "", OD_FLOAT32, OD_ACCESS_RW}},

{{0x3007, 0x01}, {"config", "resolver", "pole_pairs", "", OD_INT16, OD_ACCESS_RW}},
{{0x3007, 0x02}, {"config", "resolver", "install_direction", "", OD_INT16, OD_ACCESS_RO}},
{{0x3007, 0x03}, {"config", "resolver", "install_offset", "rad", OD_FLOAT32, OD_ACCESS_RO}},
{{0x3007, 0x04}, {"config", "resolver", "observer_natural_freq", "rad/s", OD_FLOAT32, OD_ACCESS_RW}},
{{0x3007, 0x05}, {"config", "resolver", "observer_damping_factor", "", OD_FLOAT32, OD_ACCESS_RW}},

{{0x3008, 0x01}, {"config", "precharge", "dclink_charge_threshold", "V", OD_FLOAT32, OD_ACCESS_RW}},
{{0x3008, 0x02}, {"config", "precharge", "dclink_charge_timeout_ms", "ms", OD_UINT32, OD_ACCESS_RW}},
{{0x3008, 0x03}, {"config", "precharge", "charging_contactor_holdup_ms", "ms", OD_UINT32, OD_ACCESS_RW}},
{{0x3008, 0x04}, {"config", "precharge", "dclink_discharge_threshold", "V", OD_FLOAT32, OD_ACCESS_RW}},
{{0x3008, 0x05}, {"config", "precharge", "dclink_discharge_timeout_ms", "ms", OD_UINT32, OD_ACCESS_RW}},

{{0x3009, 0x01}, {"config", "insulation", "min_batt_voltage", "V", OD_FLOAT32, OD_ACCESS_RW}},
{{0x3009, 0x02}, {"config", "insulation", "testing_period_ms", "ms", OD_UINT32, OD_ACCESS_RW}},
{{0x3009, 0x03}, {"config", "insulation", "delay_before_adc_ms", "ms", OD_UINT32, OD_ACCESS_RW}},
{{0x3009, 0x04}, {"config", "insulation", "insulation_low_limit", "Ω", OD_FLOAT32, OD_ACCESS_RW}},


}
};

} // namespace srmdrive

