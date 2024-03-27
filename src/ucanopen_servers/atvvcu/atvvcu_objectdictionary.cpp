#include "atvvcu_server.h"


namespace atvvcu {

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

{{0x1018, 0x04}, {"info", "sys", "serial_number", "", OD_ACCESS_CONST, OD_UINT32}},

{{0x2000, 0x01}, {"ctl", "sys", "reset_device", "", OD_ACCESS_WO, OD_EXEC}},
{{0x2000, 0x02}, {"ctl", "sys", "clear_errors", "", OD_ACCESS_WO, OD_EXEC}},

{{0x5000, 0x01}, {"watch", "sys", "uptime", "s", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x02}, {"watch", "sys", "syslog_message", "", OD_ACCESS_RO, OD_UINT32}},

{{0x5001, 0x01}, {"watch", "motordrive", "vdc_lf", "V", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5001, 0x02}, {"watch", "motordrive", "vdc_lb", "V", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5001, 0x03}, {"watch", "motordrive", "vdc_rf", "V", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5001, 0x04}, {"watch", "motordrive", "vdc_rb", "V", OD_ACCESS_RO, OD_FLOAT32}},

{{0x5002, 0x01}, {"watch", "motordrive", "iac_lf", "A", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5002, 0x02}, {"watch", "motordrive", "iac_lb", "A", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5002, 0x03}, {"watch", "motordrive", "iac_rf", "A", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5002, 0x04}, {"watch", "motordrive", "iac_rb", "A", OD_ACCESS_RO, OD_FLOAT32}},

{{0x5003, 0x01}, {"watch", "motordrive", "speed_lf", "rpm", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5003, 0x02}, {"watch", "motordrive", "speed_lb", "rpm", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5003, 0x03}, {"watch", "motordrive", "speed_rf", "rpm", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5003, 0x04}, {"watch", "motordrive", "speed_rb", "rpm", OD_ACCESS_RO, OD_FLOAT32}},

{{0x5004, 0x01}, {"watch", "motordrive", "torque_lf", "Nm", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5004, 0x02}, {"watch", "motordrive", "torque_lb", "Nm", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5004, 0x03}, {"watch", "motordrive", "torque_rf", "Nm", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5004, 0x04}, {"watch", "motordrive", "torque_rb", "Nm", OD_ACCESS_RO, OD_FLOAT32}},

{{0x5005, 0x01}, {"watch", "motordrive", "motortemp_lf", "o", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5005, 0x02}, {"watch", "motordrive", "motortemp_lb", "o", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5005, 0x03}, {"watch", "motordrive", "motortemp_rf", "o", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5005, 0x04}, {"watch", "motordrive", "motortemp_rb", "o", OD_ACCESS_RO, OD_FLOAT32}},

{{0x5006, 0x01}, {"watch", "motordrive", "moduletemp_lf", "o", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5006, 0x02}, {"watch", "motordrive", "moduletemp_lb", "o", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5006, 0x03}, {"watch", "motordrive", "moduletemp_rf", "o", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5006, 0x04}, {"watch", "motordrive", "moduletemp_rb", "o", OD_ACCESS_RO, OD_FLOAT32}},

{{0x5010, 0x01}, {"watch", "bms", "i_batt", "A", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5010, 0x02}, {"watch", "bms", "t_cellmin", "o", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5010, 0x03}, {"watch", "bms", "t_cellmax", "o", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5010, 0x04}, {"watch", "bms", "charge_pct", "%", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5010, 0x05}, {"watch", "bms", "v_batt", "V", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5010, 0x06}, {"watch", "bms", "internal_state", "", OD_ACCESS_RO, OD_UINT32}},
{{0x5010, 0x07}, {"watch", "bms", "error_register_1", "", OD_ACCESS_RO, OD_UINT32}},
{{0x5010, 0x08}, {"watch", "bms", "error_register_2", "", OD_ACCESS_RO, OD_UINT32}},
{{0x5010, 0x09}, {"watch", "bms", "discrete_inputs_1", "", OD_ACCESS_RO, OD_UINT32}},
{{0x5010, 0x0A}, {"watch", "bms", "discrete_inputs_2", "", OD_ACCESS_RO, OD_UINT32}},


{{0x3002, 0x01}, {"config", "drive", "torque_max", "", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3002, 0x02}, {"config", "drive", "slip_threshold", "", OD_ACCESS_RW, OD_FLOAT32}},
{{0x3002, 0x03}, {"config", "drive", "antislip_coef", "", OD_ACCESS_RW, OD_FLOAT32}},


}
};

} // namespace atvvcu

