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

{{0x2001, 0x01}, {"ctl", "pdm", "enable_contactor", "", OD_ACCESS_WO, OD_UINT32}},
{{0x2001, 0x02}, {"ctl", "pdm", "disable_contactor", "", OD_ACCESS_WO, OD_UINT32}},

{{0x2002, 0x01}, {"ctl", "motordrive", "control_mode", "", OD_ACCESS_WO, OD_UINT32}},
{{0x2002, 0x02}, {"ctl", "motordrive", "enable_status", "", OD_ACCESS_WO, OD_UINT32}},
{{0x2002, 0x03}, {"ctl", "motordrive", "active_discharge_status", "", OD_ACCESS_WO, OD_UINT32}},
{{0x2002, 0x04}, {"ctl", "motordrive", "mainrelay_status", "", OD_ACCESS_WO, OD_UINT32}},
{{0x2002, 0x05}, {"ctl", "motordrive", "gear", "", OD_ACCESS_WO, OD_UINT32}},
{{0x2002, 0x06}, {"ctl", "motordrive", "footbrake_status", "", OD_ACCESS_WO, OD_UINT32}},
{{0x2002, 0x07}, {"ctl", "motordrive", "handbrake_status", "", OD_ACCESS_WO, OD_UINT32}},
{{0x2002, 0x08}, {"ctl", "motordrive", "faultreset_status", "", OD_ACCESS_WO, OD_UINT32}},
{{0x2002, 0x09}, {"ctl", "motordrive", "speed_ref", "", OD_ACCESS_WO, OD_UINT32}},
{{0x2002, 0x0A}, {"ctl", "motordrive", "torque_ref", "", OD_ACCESS_WO, OD_UINT32}},
{{0x2002, 0x0B}, {"ctl", "motordrive", "eltorque_max", "", OD_ACCESS_WO, OD_UINT32}},
{{0x2002, 0x0C}, {"ctl", "motordrive", "braketorque_max", "", OD_ACCESS_WO, OD_UINT32}},
{{0x2002, 0x0D}, {"ctl", "motordrive", "enable_node", "", OD_ACCESS_WO, OD_UINT32}},
{{0x2002, 0x0E}, {"ctl", "motordrive", "disable_node", "", OD_ACCESS_WO, OD_UINT32}},


{{0x5000, 0x01}, {"watch", "sys", "uptime", "s", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5000, 0x02}, {"watch", "sys", "syslog_message", "", OD_ACCESS_RO, OD_UINT32}},

{{0x5000, 0x03}, {"watch", "pdm", "contactor_state", "", OD_ACCESS_RO, OD_UINT32}},

{{0x5001, 0x01}, {"watch", "motordrive", "status_lf", "", OD_ACCESS_RO, OD_UINT32}},
{{0x5001, 0x02}, {"watch", "motordrive", "speed_lf", "", OD_ACCESS_RO, OD_INT32}},
{{0x5001, 0x03}, {"watch", "motordrive", "torque_lf", "", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5001, 0x04}, {"watch", "motordrive", "vdc_lf", "", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5001, 0x05}, {"watch", "motordrive", "ac_lf", "", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5001, 0x06}, {"watch", "motordrive", "motortemp_lf", "", OD_ACCESS_RO, OD_INT32}},
{{0x5001, 0x07}, {"watch", "motordrive", "conttemp_lf", "", OD_ACCESS_RO, OD_INT32}},

{{0x5002, 0x01}, {"watch", "motordrive", "status_lb", "", OD_ACCESS_RO, OD_UINT32}},
{{0x5002, 0x02}, {"watch", "motordrive", "speed_lb", "", OD_ACCESS_RO, OD_INT32}},
{{0x5002, 0x03}, {"watch", "motordrive", "torque_lb", "", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5002, 0x04}, {"watch", "motordrive", "vdc_lb", "", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5002, 0x05}, {"watch", "motordrive", "ac_lb", "", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5002, 0x06}, {"watch", "motordrive", "motortemp_lb", "", OD_ACCESS_RO, OD_INT32}},
{{0x5002, 0x07}, {"watch", "motordrive", "conttemp_lb", "", OD_ACCESS_RO, OD_INT32}},

{{0x5003, 0x01}, {"watch", "motordrive", "status_rf", "", OD_ACCESS_RO, OD_UINT32}},
{{0x5003, 0x02}, {"watch", "motordrive", "speed_rf", "", OD_ACCESS_RO, OD_INT32}},
{{0x5003, 0x03}, {"watch", "motordrive", "torque_rf", "", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5003, 0x04}, {"watch", "motordrive", "vdc_rf", "", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5003, 0x05}, {"watch", "motordrive", "ac_rf", "", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5003, 0x06}, {"watch", "motordrive", "motortemp_rf", "", OD_ACCESS_RO, OD_INT32}},
{{0x5003, 0x07}, {"watch", "motordrive", "conttemp_rf", "", OD_ACCESS_RO, OD_INT32}},

{{0x5004, 0x01}, {"watch", "motordrive", "status_rb", "", OD_ACCESS_RO, OD_UINT32}},
{{0x5004, 0x02}, {"watch", "motordrive", "speed_rb", "", OD_ACCESS_RO, OD_INT32}},
{{0x5004, 0x03}, {"watch", "motordrive", "torque_rb", "", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5004, 0x04}, {"watch", "motordrive", "vdc_rb", "", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5004, 0x05}, {"watch", "motordrive", "ac_rb", "", OD_ACCESS_RO, OD_FLOAT32}},
{{0x5004, 0x06}, {"watch", "motordrive", "motortemp_rb", "", OD_ACCESS_RO, OD_INT32}},
{{0x5004, 0x07}, {"watch", "motordrive", "conttemp_rb", "", OD_ACCESS_RO, OD_INT32}},



}
};

} // namespace atvvcu

