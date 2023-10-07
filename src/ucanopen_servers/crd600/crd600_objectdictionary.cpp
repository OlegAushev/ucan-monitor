#include "crd600_server.h"


namespace crd600 {

using namespace ucanopen;


extern const ucanopen::ObjectDictionary object_dictionary = {
.config = {
    .watch_category = "watch",
    .config_category = "config"
},
.entries = {
{{0x1008, 0x00}, {"sys", "info", "device_name", "", OD_STRING, OD_ACCESS_CONST}},
{{0x1009, 0x00}, {"sys", "info", "hardware_version", "", OD_STRING, OD_ACCESS_CONST}},
{{0x100A, 0x00}, {"sys", "info", "firmware_version", "", OD_STRING, OD_ACCESS_CONST}},

{{0x1010, 0x01}, {"sys", "ctl", "save_all_parameters", "", OD_EXEC, OD_ACCESS_WO}},
{{0x1011, 0x01}, {"sys", "ctl", "restore_all_default_parameters", "", OD_EXEC, OD_ACCESS_WO}},

{{0x1018, 0x04}, {"sys", "info", "serial_number", "", OD_UINT32, OD_ACCESS_CONST}},

{{0x2000, 0x01}, {"sys", "ctl", "reset_device", "", OD_EXEC, OD_ACCESS_WO}},
{{0x2000, 0x02}, {"sys", "ctl", "clear_errors", "", OD_EXEC, OD_ACCESS_WO}},

{{0x2001, 0x01}, {"drive", "ctl", "powerup", "", OD_EXEC, OD_ACCESS_WO}},
{{0x2001, 0x02}, {"drive", "ctl", "powerdown", "", OD_EXEC, OD_ACCESS_WO}},


{{0x5000, 0x01}, {"watch", "watch", "uptime", "s", OD_FLOAT32, OD_ACCESS_RO}},
{{0x5000, 0x02}, {"watch", "watch", "syslog_message", "", OD_UINT32, OD_ACCESS_RO}},


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

}
};

} // namespace crd600

