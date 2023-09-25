#pragma once


#include <cstdint>
#include <string>
#include <vector>


namespace launchpad {

struct CobTpdo4 {
    uint64_t counter : 2;
    uint64_t errors : 31;
    uint64_t warnings : 31;
};


struct CobRpdo1 {
    uint32_t counter : 2;
    uint32_t _reserved : 30;
    float value;
};


struct CobRpdo2 {
    uint32_t counter : 2;
    uint32_t _reserved : 30;
    float value;
};


struct CobRpdo3 {
    uint32_t counter : 2;
    uint32_t _reserved : 30;
    float value;
};


struct CobRpdo4 {
    uint32_t counter : 2;
    uint32_t _reserved : 30;
    float value;
};


struct CobClientTpdo1 {
    uint32_t counter : 2;
    uint32_t _reserved : 30;
    float value;
};


struct CobClientTpdo2 {
    uint32_t counter : 2;
    uint32_t _reserved : 30;
    float value;
};


struct CobClientTpdo3 {
    uint32_t counter : 2;
    uint32_t _reserved : 30;
    float value;
};


struct CobClientTpdo4 {
    uint32_t counter : 2;
    uint32_t _reserved : 30;
    float value;
};


inline const std::vector<std::string> syslog_messages = {
    "",
    "         @syslog: Boot CPU1...",
    "[  OK  ] @syslog: CPU1 ready.",
    "         @syslog: Boot CPU2...",
    "[  OK  ] @syslog: CPU2 ready.",
    "[  OK  ] @syslog: Configured CPU1 periphery.",
    "[  OK  ] @syslog: Configured CPU2 periphery.",
    "[  OK  ] @syslog: Device ready.",
    "[FAILED] @syslog: Device busy.",
    "[FAILED] @syslog: Error occurred.",
    "         @syslog: Resetting device...",
    "[ WARN ] @syslog: Lost SDO request.",
    "[ WARN ] @syslog: Cannot communicate with EEPROM: disabled.",
    "[  OK  ] @syslog: Read settings.",
    "[FAILED] @syslog: Failed to read settings.",
    "[  OK  ] @syslog: Applied settings.",
    "[FAILED] @syslog: Failed to apply settings.",
    "[  OK  ] @syslog: Restored settings.",
    "[FAILED] @syslog: Failed to restore settings.",
    "[  OK  ] @syslog: Loaded default settings.",
};


inline const std::vector<std::string_view> error_list = {
    "can_bus_connection_lost",
};


inline const std::vector<std::string_view> warning_list = {
    "can_bus_error",
    "can_bus_overrun",
    "can_bus_connection_lost",
};

} // namespace launchpad

