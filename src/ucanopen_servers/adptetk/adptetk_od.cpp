#include "adptetk_server.hpp"

namespace adptetk {

using namespace ucanopen;

// Mirrors object_dictionary_storage in
// src/app/inverter/comm/can/canopen/od.cpp of the adpt-etk-inverter firmware.
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
{{0x2001, 0x01}, {"ctl", "drive", "calibrate_angle_sensor",   "",   OD_ACCESS_WO, OD_EXEC}},
{{0x2001, 0x02}, {"ctl", "drive", "save_angle_sensor_config", "",   OD_ACCESS_WO, OD_EXEC}},
{{0x2001, 0x03}, {"ctl", "drive", "set_angle_correction",     "°",  OD_ACCESS_WO, OD_FLOAT32}},
{{0x2001, 0x04}, {"ctl", "drive", "Vd_limit_factor",          "",   OD_ACCESS_WO, OD_FLOAT32}},
{{0x2001, 0x05}, {"ctl", "drive", "pwm_freq",                 "Hz", OD_ACCESS_WO, OD_FLOAT32}},
{{0x2001, 0x06}, {"ctl", "drive", "dc_a",                     "",   OD_ACCESS_WO, OD_FLOAT32}},
{{0x2001, 0x07}, {"ctl", "drive", "dc_b",                     "",   OD_ACCESS_WO, OD_FLOAT32}},
{{0x2001, 0x08}, {"ctl", "drive", "dc_c",                     "",   OD_ACCESS_WO, OD_FLOAT32}},
//##########################################################################################################################################################################################################################################################################################################
{{0x5000, 0x01}, {"watch", "sys", "uptime",     "s",  OD_ACCESS_RO, OD_FLOAT32}},
//##########################################################################################################################################################################################################################################################################################################
{{0x5000, 0x11}, {"watch", "elec", "Vdc",    "V", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x12}, {"watch", "elec", "Idc",    "A", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x13}, {"watch", "elec", "Ia",     "A", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x14}, {"watch", "elec", "Ib",     "A", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x15}, {"watch", "elec", "Ic",     "A", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x16}, {"watch", "elec", "Is",     "A", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x17}, {"watch", "elec", "Id",     "A", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x18}, {"watch", "elec", "Idref",  "A", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x19}, {"watch", "elec", "Iq",     "A", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x1A}, {"watch", "elec", "Iqref",  "A", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x1B}, {"watch", "elec", "Vd",     "V", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x1C}, {"watch", "elec", "Vq",     "V", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x1D}, {"watch", "elec", "Vdcomp", "V", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x1E}, {"watch", "elec", "Vqcomp", "V", OD_ACCESS_RO, OD_FLOAT32}},
//##########################################################################################################################################################################################################################################################################################################
{{0x5000, 0x21}, {"watch", "temp", "Tmcu",   "°C", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x22}, {"watch", "temp", "Tpwr",   "°C", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x23}, {"watch", "temp", "Tmot",   "°C", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x24}, {"watch", "temp", "Tpwr_a", "°C", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x25}, {"watch", "temp", "Tpwr_b", "°C", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x26}, {"watch", "temp", "Tpwr_c", "°C", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x27}, {"watch", "temp", "Tmot1",  "°C", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x28}, {"watch", "temp", "Tmot2",  "°C", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x29}, {"watch", "temp", "Tmot3",  "°C", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x2A}, {"watch", "temp", "Tmot4",  "°C", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x2B}, {"watch", "temp", "Tmot5",  "°C", OD_ACCESS_RO, OD_FLOAT32}},
//##########################################################################################################################################################################################################################################################################################################
{{0x5000, 0x31}, {"watch", "torque", "ref",     "%", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x32}, {"watch", "torque", "ref_mod", "%", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x33}, {"watch", "torque", "actual",  "%", OD_ACCESS_RO, OD_FLOAT32}},
//##########################################################################################################################################################################################################################################################################################################
{{0x5000, 0x41}, {"watch", "speed", "ref",     "rpm", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x42}, {"watch", "speed", "ref_mod", "rpm", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x43}, {"watch", "speed", "actual",  "rpm", OD_ACCESS_RO, OD_FLOAT32}},
//##########################################################################################################################################################################################################################################################################################################
{{0x5000, 0x51}, {"watch", "angle", "ref",    "°", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x52}, {"watch", "angle", "actual", "°", OD_ACCESS_RO, OD_FLOAT32}},
//##########################################################################################################################################################################################################################################################################################################
{{0x5000, 0x61}, {"watch", "model", "angCorr",  "°",  OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x62}, {"watch", "model", "Vdlim",    "",   OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x63}, {"watch", "model", "pwm_freq", "Hz", OD_ACCESS_RO, OD_FLOAT32}},
//##########################################################################################################################################################################################################################################################################################################
{{0x5000, 0x71}, {"watch", "hall", "speed",      "rpm", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x72}, {"watch", "hall", "angle",      "°",   OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x73}, {"watch", "hall", "sector",     "",    OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x74}, {"watch", "hall", "inA",        "",    OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x75}, {"watch", "hall", "inB",        "",    OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x76}, {"watch", "hall", "inC",        "",    OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x77}, {"watch", "hall", "emu_angle",  "°",   OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x78}, {"watch", "hall", "emu_dspeed", "rpm", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x79}, {"watch", "hall", "emu_dangle", "°",   OD_ACCESS_RO, OD_FLOAT32}},
//##########################################################################################################################################################################################################################################################################################################
{{0x5000, 0x81}, {"watch", "ext", "V1", "V", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x82}, {"watch", "ext", "V2", "V", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x83}, {"watch", "ext", "V3", "V", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x84}, {"watch", "ext", "V4", "V", OD_ACCESS_RO, OD_FLOAT32}},
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
{{0x3002, 0x01}, {"config", "drive", "phase_swap",       "",      OD_ACCESS_RW, OD_BOOL}},
{{0x3002, 0x02}, {"config", "drive", "torque_slope",     "pu/s",  OD_ACCESS_RW, OD_FLOAT32}},
{{0x3002, 0x03}, {"config", "drive", "speed_slope",      "rpm/s", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3002, 0x04}, {"config", "drive", "runout_speed",     "rpm",   OD_ACCESS_RW, OD_FLOAT32}},
{{0x3002, 0x05}, {"config", "drive", "stopping_torque",  "pu",    OD_ACCESS_RW, OD_FLOAT32}},
{{0x3002, 0x06}, {"config", "drive", "stopping_timeout", "ms",    OD_ACCESS_RW, OD_UINT32}},
{{0x3002, 0x07}, {"config", "drive", "active_discharge", "",      OD_ACCESS_RW, OD_BOOL}},
//##########################################################################################################################################################################################################################################################################################################
{{0x3003, 0x01}, {"config", "protection", "uvp_dc",           "V",  OD_ACCESS_RW, OD_FLOAT32}},
{{0x3003, 0x02}, {"config", "protection", "ovp_dc",           "V",  OD_ACCESS_RW, OD_FLOAT32}},
{{0x3003, 0x03}, {"config", "protection", "ocp_phase",        "A",  OD_ACCESS_RW, OD_FLOAT32}},
{{0x3003, 0x04}, {"config", "protection", "otp_motor",        "°C", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3003, 0x05}, {"config", "protection", "otp_power",        "°C", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3003, 0x06}, {"config", "protection", "otp_mcu",          "°C", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3003, 0x07}, {"config", "protection", "ot_warn_margin",   "°C", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3003, 0x08}, {"config", "protection", "ot_reset_margin",  "°C", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3003, 0x09}, {"config", "protection", "stall_current_th", "pu", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3003, 0x0A}, {"config", "protection", "stall_timeout",    "ms", OD_ACCESS_RW, OD_UINT32}},
{{0x3003, 0x0B}, {"config", "protection", "watchdog_timeout", "ms", OD_ACCESS_RW, OD_UINT32}},
//##########################################################################################################################################################################################################################################################################################################
{{0x3004, 0x01}, {"config", "motor", "pole_pairs", "",    OD_ACCESS_RW, OD_INT32}},
{{0x3004, 0x02}, {"config", "motor", "R",          "Ohm", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3004, 0x03}, {"config", "motor", "Ld",         "H",   OD_ACCESS_RW, OD_FLOAT32}},
{{0x3004, 0x04}, {"config", "motor", "Lq",         "H",   OD_ACCESS_RW, OD_FLOAT32}},
{{0x3004, 0x05}, {"config", "motor", "psi",        "Wb",  OD_ACCESS_RW, OD_FLOAT32}},
//##########################################################################################################################################################################################################################################################################################################
{{0x3005, 0x01}, {"config", "model", "iSMot_limit",      "A",   OD_ACCESS_RW, OD_FLOAT32}},
{{0x3005, 0x02}, {"config", "model", "iSGen_limit",      "A",   OD_ACCESS_RW, OD_FLOAT32}},
{{0x3005, 0x03}, {"config", "model", "speed_limit",      "rpm", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3005, 0x04}, {"config", "model", "angle_correction", "°",   OD_ACCESS_RW, OD_FLOAT32}},
{{0x3005, 0x05}, {"config", "model", "vD_limit_factor",  "",    OD_ACCESS_RW, OD_FLOAT32}},
//##########################################################################################################################################################################################################################################################################################################
{{0x3006, 0x01}, {"config", "model_controllers", "speed_kP", "", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3006, 0x02}, {"config", "model_controllers", "speed_kI", "", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3006, 0x03}, {"config", "model_controllers", "iD_kP",    "", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3006, 0x04}, {"config", "model_controllers", "iD_kI",    "", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3006, 0x05}, {"config", "model_controllers", "iQ_kP",    "", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3006, 0x06}, {"config", "model_controllers", "iQ_kI",    "", OD_ACCESS_RW, OD_FLOAT32}},
//##########################################################################################################################################################################################################################################################################################################
{{0x3007, 0x01}, {"config", "mras", "kP",          "",    OD_ACCESS_RW, OD_FLOAT32}},
{{0x3007, 0x02}, {"config", "mras", "kI",          "",    OD_ACCESS_RW, OD_FLOAT32}},
{{0x3007, 0x03}, {"config", "mras", "speed_limit", "rpm", OD_ACCESS_RW, OD_FLOAT32}},
//##########################################################################################################################################################################################################################################################################################################
{{0x3008, 0x01}, {"config", "phase_current_sensor", "gain_a",               "A/V", OD_ACCESS_CONST, OD_FLOAT32}},
{{0x3008, 0x02}, {"config", "phase_current_sensor", "gain_b",               "A/V", OD_ACCESS_CONST, OD_FLOAT32}},
{{0x3008, 0x03}, {"config", "phase_current_sensor", "gain_c",               "A/V", OD_ACCESS_CONST, OD_FLOAT32}},
{{0x3008, 0x04}, {"config", "phase_current_sensor", "zero_drift_a",         "A",   OD_ACCESS_RO,    OD_FLOAT32}},
{{0x3008, 0x05}, {"config", "phase_current_sensor", "zero_drift_b",         "A",   OD_ACCESS_RO,    OD_FLOAT32}},
{{0x3008, 0x06}, {"config", "phase_current_sensor", "zero_drift_c",         "A",   OD_ACCESS_RO,    OD_FLOAT32}},
{{0x3008, 0x07}, {"config", "phase_current_sensor", "zero_drift_threshold", "A",   OD_ACCESS_RW,    OD_FLOAT32}},
//##########################################################################################################################################################################################################################################################################################################
{{0x3009, 0x01}, {"config", "dc_voltage_sensor", "gain",   "V/V", OD_ACCESS_CONST, OD_FLOAT32}},
{{0x3009, 0x02}, {"config", "dc_voltage_sensor", "offset", "V",   OD_ACCESS_CONST, OD_FLOAT32}},
//##########################################################################################################################################################################################################################################################################################################
{{0x300A, 0x01}, {"config", "hall_anglesensor", "enabled",            "",  OD_ACCESS_RW, OD_BOOL}},
{{0x300A, 0x02}, {"config", "hall_anglesensor", "poll_num",           "",  OD_ACCESS_RW, OD_INT32}},
{{0x300A, 0x03}, {"config", "hall_anglesensor", "speed_timeconstant", "s", OD_ACCESS_RW, OD_FLOAT32}},
{{0x300A, 0x04}, {"config", "hall_anglesensor", "fwd_a",  "°", OD_ACCESS_RW, OD_FLOAT32}},
{{0x300A, 0x05}, {"config", "hall_anglesensor", "fwd_ab", "°", OD_ACCESS_RW, OD_FLOAT32}},
{{0x300A, 0x06}, {"config", "hall_anglesensor", "fwd_b",  "°", OD_ACCESS_RW, OD_FLOAT32}},
{{0x300A, 0x07}, {"config", "hall_anglesensor", "fwd_bc", "°", OD_ACCESS_RW, OD_FLOAT32}},
{{0x300A, 0x08}, {"config", "hall_anglesensor", "fwd_c",  "°", OD_ACCESS_RW, OD_FLOAT32}},
{{0x300A, 0x09}, {"config", "hall_anglesensor", "fwd_ca", "°", OD_ACCESS_RW, OD_FLOAT32}},
{{0x300A, 0x0A}, {"config", "hall_anglesensor", "rev_a",  "°", OD_ACCESS_RW, OD_FLOAT32}},
{{0x300A, 0x0B}, {"config", "hall_anglesensor", "rev_ab", "°", OD_ACCESS_RW, OD_FLOAT32}},
{{0x300A, 0x0C}, {"config", "hall_anglesensor", "rev_b",  "°", OD_ACCESS_RW, OD_FLOAT32}},
{{0x300A, 0x0D}, {"config", "hall_anglesensor", "rev_bc", "°", OD_ACCESS_RW, OD_FLOAT32}},
{{0x300A, 0x0E}, {"config", "hall_anglesensor", "rev_c",  "°", OD_ACCESS_RW, OD_FLOAT32}},
{{0x300A, 0x0F}, {"config", "hall_anglesensor", "rev_ca", "°", OD_ACCESS_RW, OD_FLOAT32}},
//##########################################################################################################################################################################################################################################################################################################
{{0x300B, 0x01}, {"config", "hall_calibrator", "phase_current",       "pu",  OD_ACCESS_RW, OD_FLOAT32}},
{{0x300B, 0x02}, {"config", "hall_calibrator", "phase_current_slope", "pu",  OD_ACCESS_RW, OD_FLOAT32}},
{{0x300B, 0x03}, {"config", "hall_calibrator", "speed",               "rpm", OD_ACCESS_RW, OD_FLOAT32}},
{{0x300B, 0x04}, {"config", "hall_calibrator", "elrevolution_num",    "",    OD_ACCESS_RW, OD_INT32}},
//##########################################################################################################################################################################################################################################################################################################
}
};
// clang-format on

} // namespace adptetk
