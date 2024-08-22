#include "moyka_server.h"


namespace moyka {

using namespace ucanopen;


extern const ucanopen::ObjectDictionary object_dictionary = {
.config = {
    .watch_category = "WATCH",
    .config_category = "CONFIG"
},
.entries = {
{{0x2000, 0x00}, {"WATCH",	"WATCH", "UPTIME",		"s", OD_ACCESS_RO,	OD_FLOAT32}},
{{0x2000, 0x01}, {"WATCH",	"WATCH", "DRIVE_STATE",		"", OD_ACCESS_RO,	OD_UINT32}},
{{0x2000, 0x02}, {"WATCH",	"WATCH", "FAULTS",		"", OD_ACCESS_RO,	OD_UINT32}},
{{0x2000, 0x03}, {"WATCH",	"WATCH", "DC_VOLTAGE",		"V", OD_ACCESS_RO,	OD_FLOAT32}},
{{0x2000, 0x04}, {"WATCH",	"WATCH", "DC_CURRENT",		"A", OD_ACCESS_RO,	OD_FLOAT32}},
{{0x2000, 0x05}, {"WATCH",	"WATCH", "FIELD_CURRENT",	"A", OD_ACCESS_RO,	OD_FLOAT32}},
{{0x2000, 0x06}, {"WATCH",	"WATCH", "STATOR_CURRENT",	"A", OD_ACCESS_RO,	OD_FLOAT32}},
{{0x2000, 0x07}, {"WATCH",	"WATCH", "PHA_CURRENT",		"A", OD_ACCESS_RO,	OD_FLOAT32}},
{{0x2000, 0x08}, {"WATCH",	"WATCH", "PHB_CURRENT",		"A", OD_ACCESS_RO,	OD_FLOAT32}},
{{0x2000, 0x09}, {"WATCH",	"WATCH", "PHC_CURRENT",		"A", OD_ACCESS_RO,	OD_FLOAT32}},
{{0x2000, 0x0A}, {"WATCH",	"WATCH", "D_CURRENT",		"A", OD_ACCESS_RO,	OD_FLOAT32}},
{{0x2000, 0x0B}, {"WATCH",	"WATCH", "Q_CURRENT",		"A", OD_ACCESS_RO,	OD_FLOAT32}},
{{0x2000, 0x0C}, {"WATCH",	"WATCH", "PHA_TEMP",		"°C", OD_ACCESS_RO,	OD_FLOAT32}},
{{0x2000, 0x0D}, {"WATCH",	"WATCH", "PHB_TEMP",		"°C", OD_ACCESS_RO,	OD_FLOAT32}},
{{0x2000, 0x0E}, {"WATCH",	"WATCH", "PHC_TEMP",		"°C", OD_ACCESS_RO,	OD_FLOAT32}},
{{0x2000, 0x0F}, {"WATCH",	"WATCH", "CASE_TEMP",		"°C", OD_ACCESS_RO,	OD_FLOAT32}},
{{0x2000, 0x10}, {"WATCH",	"WATCH", "MOTOR_S_TEMP",	"°C", OD_ACCESS_RO,	OD_FLOAT32}},
{{0x2000, 0x11}, {"WATCH",	"WATCH", "MOTOR_FW_TEMP",	"°C", OD_ACCESS_RO,	OD_FLOAT32}},
{{0x2000, 0x12}, {"WATCH",	"WATCH", "GAMMA_ANGLE_DEG",	"°", OD_ACCESS_RO,	OD_FLOAT32}},
{{0x2000, 0x13}, {"WATCH",	"WATCH", "SPEED_RPM",		"rpm", OD_ACCESS_RO,	OD_FLOAT32}},
{{0x2000, 0x14}, {"WATCH",	"WATCH", "TORQUE",		"Nm", OD_ACCESS_RO,	OD_FLOAT32}},
{{0x2000, 0x15}, {"WATCH",	"WATCH", "MECH_POWER",		"W", OD_ACCESS_RO,	OD_FLOAT32}},
{{0x2000, 0x16}, {"WATCH",	"WATCH", "OUT_ELEC_POWER",	"W", OD_ACCESS_RO,	OD_FLOAT32}},

{{0x2001, 0x00}, {"DRIVE CONTROL", 	"DRIVE CONTROL", "POWER UP DRIVE", 			"", OD_ACCESS_WO,	OD_EXEC}},
{{0x2001, 0x01}, {"DRIVE CONTROL",	"DRIVE CONTROL", "POWER DOWN DRIVE", 			"", OD_ACCESS_WO,	OD_EXEC}},

{{0x2002, 0x00}, {"SYSTEM CONTROL", 	"SYSTEM CONTROL", "RESET DEVICE", 			"", OD_ACCESS_WO,	OD_EXEC}},
{{0x2002, 0x01}, {"SYSTEM CONTROL", 	"SYSTEM CONTROL", "RESET PARAMETERS", 			"", OD_ACCESS_WO,	OD_EXEC}},
{{0x2002, 0x02}, {"SYSTEM CONTROL", 	"SYSTEM CONTROL", "APPLY PARAMETERS", 			"", OD_ACCESS_WO,	OD_EXEC}},
{{0x2002, 0x03}, {"SYSTEM CONTROL", 	"SYSTEM CONTROL", "BEGIN POSITION SENSOR CALIBRATION", 	"", OD_ACCESS_WO,	OD_EXEC}},
{{0x2002, 0x04}, {"SYSTEM CONTROL", 	"SYSTEM CONTROL", "INVERT ROTATION", 			"", OD_ACCESS_WO,	OD_EXEC}},
{{0x2002, 0x05}, {"SYSTEM CONTROL", 	"SYSTEM CONTROL", "RESET FAULTS", 			"", OD_ACCESS_WO,	OD_EXEC}},

{{0x2100, 0x00}, {"CONFIG",	"MOTOR",	"R", 			"Ω",   OD_ACCESS_RW,	OD_FLOAT32}},
{{0x2100, 0x01}, {"CONFIG",	"MOTOR",	"LD", 			"H",   OD_ACCESS_RW,	OD_FLOAT32}},
{{0x2100, 0x02}, {"CONFIG",	"MOTOR",	"KLD", 			"",   OD_ACCESS_RW,	OD_FLOAT32}},
{{0x2100, 0x03}, {"CONFIG",	"MOTOR",	"LQ", 			"H",   OD_ACCESS_RW,	OD_FLOAT32}},
{{0x2100, 0x04}, {"CONFIG",	"MOTOR",	"KLQ", 			"",   OD_ACCESS_RW,	OD_FLOAT32}},
{{0x2100, 0x05}, {"CONFIG",	"MOTOR",	"OTP_STATOR", 		"°C",   OD_ACCESS_RW,	OD_FLOAT32}},
{{0x2100, 0x06}, {"CONFIG",	"MOTOR",	"OTP_FW", 		"°C",   OD_ACCESS_RW,	OD_FLOAT32}},
{{0x2100, 0x07}, {"CONFIG",	"MOTOR",	"FAN_TEMP_TH_ON", 	"°C",   OD_ACCESS_RW,	OD_FLOAT32}},
{{0x2100, 0x08}, {"CONFIG",	"MOTOR",	"FAN_TEMP_TH_OFF", 	"°C",   OD_ACCESS_RW,	OD_FLOAT32}},
{{0x2100, 0x09}, {"CONFIG",	"MOTOR",	"MOTOR_ID",	 	"",   OD_ACCESS_RW,	OD_UINT32}},

{{0x2101, 0x00}, {"CONFIG",	"MODEL",	"REFERENCE", 		"n-M",   OD_ACCESS_RW,	OD_INT16}},
{{0x2101, 0x01}, {"CONFIG",	"MODEL",	"KP_SPEED", 		"",   OD_ACCESS_RW,	OD_FLOAT32}},
{{0x2101, 0x02}, {"CONFIG",	"MODEL",	"KI_SPEED", 		"",   OD_ACCESS_RW,	OD_FLOAT32}},
{{0x2101, 0x03}, {"CONFIG",	"MODEL",	"KP_ID", 		"",   OD_ACCESS_RW,	OD_FLOAT32}},
{{0x2101, 0x04}, {"CONFIG",	"MODEL",	"KI_ID", 		"",   OD_ACCESS_RW,	OD_FLOAT32}},
{{0x2101, 0x05}, {"CONFIG",	"MODEL",	"KP_IQ", 		"",   OD_ACCESS_RW,	OD_FLOAT32}},
{{0x2101, 0x06}, {"CONFIG",	"MODEL",	"KI_IQ", 		"",   OD_ACCESS_RW,	OD_FLOAT32}},
{{0x2101, 0x07}, {"CONFIG",	"MODEL",	"KP_IF", 		"",   OD_ACCESS_RW,	OD_FLOAT32}},
{{0x2101, 0x08}, {"CONFIG",	"MODEL",	"KI_IF", 		"",   OD_ACCESS_RW,	OD_FLOAT32}},
{{0x2101, 0x09}, {"CONFIG",	"MODEL",	"IS_MOTOR_MAX", 	"A",   OD_ACCESS_RW,	OD_FLOAT32}},
{{0x2101, 0x0A}, {"CONFIG",	"MODEL",	"IS_GENER_MAX", 	"A",   OD_ACCESS_RW,	OD_FLOAT32}},
{{0x2101, 0x0B}, {"CONFIG",	"MODEL",	"IF_MAX", 		"A",   OD_ACCESS_RW,	OD_FLOAT32}},
{{0x2101, 0x0C}, {"CONFIG",	"MODEL",	"TORQUE_POS_MAX", 	"Nm",   OD_ACCESS_RW,	OD_FLOAT32}},
{{0x2101, 0x0D}, {"CONFIG",	"MODEL",	"TORQUE_NEG_MAX",	"Nm",   OD_ACCESS_RW,	OD_FLOAT32}},
{{0x2101, 0x0E}, {"CONFIG",	"MODEL",	"SPEED_MAX",		"rpm",   OD_ACCESS_RW,	OD_FLOAT32}},

{{0x2101, 0x0F}, {"CONFIG",	"MODEL",	"KP_FLUXWEAK",		"",   OD_ACCESS_RW,	OD_FLOAT32}},
{{0x2101, 0x10}, {"CONFIG",	"MODEL",	"KI_FLUXWEAK",		"",   OD_ACCESS_RW,	OD_FLOAT32}},
{{0x2101, 0x11}, {"CONFIG",	"MODEL",	"ID_MAX_FLUXWEAK",	"A",   OD_ACCESS_RW,	OD_FLOAT32}},

{{0x2101, 0x12}, {"CONFIG", "MODEL",    "KP_MRAS",      "",   OD_ACCESS_RW, OD_FLOAT32}},
{{0x2101, 0x13}, {"CONFIG", "MODEL",    "KI_MRAS",      "",   OD_ACCESS_RW, OD_FLOAT32}},

{{0x2102, 0x00}, {"CONFIG",	"CONVERTER",	"UVP_DC", "V",   OD_ACCESS_RW,	OD_FLOAT32}},
{{0x2102, 0x01}, {"CONFIG",	"CONVERTER",	"OVP_DC", "V",   OD_ACCESS_RW,	OD_FLOAT32}},
{{0x2102, 0x02}, {"CONFIG",	"CONVERTER",	"OCP_PHASE", "A",   OD_ACCESS_RW,	OD_FLOAT32}},
{{0x2102, 0x03}, {"CONFIG",	"CONVERTER",	"OCP_FIELD", "A",   OD_ACCESS_RW,	OD_FLOAT32}},
{{0x2102, 0x04}, {"CONFIG",	"CONVERTER",	"OCP_DC", "A",   OD_ACCESS_RW,	OD_FLOAT32}},
{{0x2102, 0x05}, {"CONFIG",	"CONVERTER",	"OTP_JUNCTION", "°C",   OD_ACCESS_RW,	OD_FLOAT32}},
{{0x2102, 0x06}, {"CONFIG",	"CONVERTER",	"OTP_CASE", "°C",   OD_ACCESS_RW,	OD_FLOAT32}},
{{0x2102, 0x07}, {"CONFIG",	"CONVERTER",	"FAN_TEMP_TH_ON", "°C",   OD_ACCESS_RW,	OD_FLOAT32}},
{{0x2102, 0x08}, {"CONFIG",	"CONVERTER",	"FAN_TEMP_TH_OFF", "°C",   OD_ACCESS_RW,	OD_FLOAT32}},
{{0x2102, 0x09}, {"CONFIG",	"CONVERTER",	"CONVERTER_ID", "",   OD_ACCESS_RW,	OD_UINT32}},

{{0x2103, 0x00}, {"CONFIG",	"CONTACTOR",	"DCLINK_CHARGE_THRESHOLD", "V",   OD_ACCESS_RW,	OD_FLOAT32}},
{{0x2103, 0x01}, {"CONFIG",	"CONTACTOR",	"DCLINK_CHARGE_TIMEOUT", "ms",   OD_ACCESS_RW,	OD_UINT32}},
{{0x2103, 0x02}, {"CONFIG",	"CONTACTOR",	"DCLINK_CONTACTOR_HOLDUP", "ms",   OD_ACCESS_RW,	OD_UINT32}},
{{0x2103, 0x03}, {"CONFIG",	"CONTACTOR",	"DCLINK_DISCHARGE_THRESHOLD", "V",   OD_ACCESS_RW,	OD_FLOAT32}},
{{0x2103, 0x04}, {"CONFIG",	"CONTACTOR",	"DCLINK_DISCHARGE_TIMEOUT",	"ms",   OD_ACCESS_RW,	OD_UINT32}},

{{0x2104, 0x00}, {"CONFIG",	"MCOSERVER",	"PERIOD_HB",		"ms",   OD_ACCESS_RW,	OD_UINT32}},
{{0x2104, 0x01}, {"CONFIG",	"MCOSERVER",	"PERIOD_TPDO1",		"ms",   OD_ACCESS_RW,	OD_UINT32}},
{{0x2104, 0x02}, {"CONFIG",	"MCOSERVER",	"PERIOD_TPDO2",		"ms",   OD_ACCESS_RW,	OD_UINT32}},
{{0x2104, 0x03}, {"CONFIG",	"MCOSERVER",	"PERIOD_TPDO3",		"ms",   OD_ACCESS_RW,	OD_UINT32}},
{{0x2104, 0x04}, {"CONFIG",	"MCOSERVER",	"PERIOD_TPDO4",		"ms",   OD_ACCESS_RW,	OD_UINT32}},

{{0x2105, 0x00}, {"CONFIG",	"POSSENS",	"SECTORS",		"",   OD_ACCESS_RW,	OD_UINT32}},
{{0x2105, 0x01}, {"CONFIG",	"POSSENS",	"CAL_S_CURRENT",	"A",   OD_ACCESS_RW,	OD_FLOAT32}},
{{0x2105, 0x02}, {"CONFIG",	"POSSENS",	"CAL_F_CURRENT",	"A",   OD_ACCESS_RW,	OD_FLOAT32}},
{{0x2105, 0x03}, {"CONFIG",	"POSSENS",	"CAL_SPEED_RPM",	"rpm",   OD_ACCESS_RW,	OD_FLOAT32}},
}
};


} // namespace brkdrive
