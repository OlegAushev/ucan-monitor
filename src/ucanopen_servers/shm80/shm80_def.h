#pragma once

#include <cstdint>
#include <cstring>
#include <map>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace shm80 {

struct CobTpdo1 {
    uint16_t drive_state : 8;

    uint16_t pwm_on : 1;
    uint16_t error : 1;
    uint16_t warning : 1;
    uint16_t opmode : 2;
    uint16_t ctlmode : 1;
    uint16_t ctlloop : 2;

    uint16_t precharge_state : 3;
    uint16_t calibration_state : 3;
    uint16_t manual_field : 1;
    uint16_t _reserved1 : 1;

    uint16_t _reserved2 : 8;

    uint16_t _reserved3;

    uint16_t _reserved4 : 8;


    uint16_t counter : 2;
    uint16_t _reserved5 : 6;
};

struct CobTpdo2 {
    int16_t dc_voltage;

    int16_t stator_current;

    uint16_t _reserved1;

    uint16_t field_current : 8;

    uint16_t counter : 2;
    uint16_t _reserved2 : 6;
};

struct CobTpdo3 {
    int16_t speed;

    int16_t angle;

    uint16_t _reserved1;

    uint16_t _reserved2 : 8;

    uint16_t counter : 2;
    uint16_t _reserved3 : 6;
};

struct CobTpdo4 {
    uint32_t errors;
    uint16_t warnings;
    uint16_t domain : 8;
    uint16_t counter : 2;
    uint16_t _reserved : 6;
};

struct CobRpdo1 {
    uint16_t emergency_stop : 1;
    uint16_t power : 1;
    uint16_t start : 1;
    uint16_t ctlmode : 1;
    uint16_t isotest_dis : 1;
    uint16_t _reserved1 : 3;

    uint16_t _reserved2 : 8;

    int16_t ref_torque;

    int16_t ref_speed;

    uint16_t _reserved3 : 8;

    uint16_t counter : 2;
    uint16_t _reserved4 : 6;
};

struct CobRpdo2 {
    int16_t ref_angle;

    int16_t ref_current;

    uint16_t ref_voltage;

    uint16_t ref_field : 8;

    uint16_t counter : 2;
    uint16_t opmode : 2;
    uint16_t ctlloop : 2;
    uint16_t manual_field : 1;
    uint16_t _reserved : 1;
};

enum class DriveState {
    init,
    standby,
    powering_up,
    powering_down,
    ready,
    running,
    stopping,
    calibrating,
    testing,
};

inline const std::unordered_set<int> drive_state_values = {
    std::to_underlying(DriveState::init),
    std::to_underlying(DriveState::standby),
    std::to_underlying(DriveState::powering_up),
    std::to_underlying(DriveState::powering_down),
    std::to_underlying(DriveState::ready),
    std::to_underlying(DriveState::running),
    std::to_underlying(DriveState::stopping),
    std::to_underlying(DriveState::calibrating),
    std::to_underlying(DriveState::testing),
};

inline const std::unordered_map<DriveState, std::string_view>
drive_state_names = {
    {DriveState::init, "init"},
    {DriveState::standby, "standby"},
    {DriveState::powering_up, "powering_up"},
    {DriveState::powering_down, "powering_down"},
    {DriveState::ready, "ready"},
    {DriveState::running, "running"},
    {DriveState::stopping, "stopping"},
    {DriveState::calibrating, "calibrating"},
    {DriveState::testing, "testing"},
};

enum class OperatingMode {
    running,
    testing
};


inline const std::unordered_set<int> opmode_values = {
    std::to_underlying(OperatingMode::running),
    std::to_underlying(OperatingMode::testing),
};

inline const std::map<OperatingMode, std::string_view> opmode_names = {
    {OperatingMode::running, "running"},
    {OperatingMode::testing, "testing"},
};


enum class ControlMode {
    torque,
    speed
};


inline const std::unordered_set<int> ctlmode_values = {
    std::to_underlying(ControlMode::torque),
    std::to_underlying(ControlMode::speed)
};


inline const std::unordered_map<ControlMode, std::string_view> ctlmode_names = {
    {ControlMode::torque, "torque"},
    {ControlMode::speed, "speed"}
};


enum class ControlLoop {
    closed,
    open,
    semiclosed,
    openvolt
};


inline const std::unordered_set<int> ctlloop_values = {
    std::to_underlying(ControlLoop::closed),
    std::to_underlying(ControlLoop::open),
    std::to_underlying(ControlLoop::semiclosed),
    std::to_underlying(ControlLoop::openvolt)
};


inline const std::unordered_map<ControlLoop, std::string_view> ctlloop_names = {
    {ControlLoop::closed, "closed"},
    {ControlLoop::open, "open"},
    {ControlLoop::semiclosed, "semiclosed"},
    {ControlLoop::openvolt, "openvolt"}
};

inline constexpr size_t syslog_domain_count = 9;

} // namespace shm80
