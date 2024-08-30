#pragma once


#include <cstdint>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>


namespace loco {


struct CobTpdo1 {
    uint16_t drive_state : 8;

    uint16_t pwm_on : 1;
    uint16_t error : 1;
    uint16_t warning : 1;
    uint16_t opmode : 2;
    uint16_t ctlmode : 1;
    uint16_t ctlloop : 2;

    int16_t torque : 16;

    int16_t speed : 16;

    uint16_t manual_field: 1;
    uint16_t _reserved1 : 7;

    uint16_t counter : 2;
    uint16_t _reserved2 : 6;
};


struct CobTpdo2 {
    uint16_t dc_voltage : 16;

    uint16_t stator_current : 16;

    uint16_t field_current : 8;

    uint16_t _reserved1 : 8;

    uint16_t _reserved2 : 8;

    uint16_t counter : 2;
    uint16_t _reserved3 : 6;
};


struct CobTpdo3 {
    uint16_t pwrmodule_temp : 8;

    uint16_t excmodule_temp : 8;

    uint16_t pcb_temp : 8;

    uint16_t aw_temp : 8;

    uint16_t fw_temp : 8;

    uint16_t _reserved1 : 8;

    uint16_t _reserved2 : 8;

    uint16_t counter : 2;
    uint16_t _reserved3 : 6;
};


struct CobTpdo4 {
    uint32_t errors;
    uint16_t warnings;
    uint16_t _reserved1 : 8;
    uint16_t counter : 2;
    uint16_t _reserved2 : 6;
};


struct CobRpdo1 {
    uint16_t power : 1;
    uint16_t start : 1;
    uint16_t ctlmode : 1;
    uint16_t _reserved1 : 13;

    int16_t ref_torque;

    int16_t ref_speed;

    uint16_t _reserved2 : 8;

    uint16_t counter : 2;
    uint16_t _reserved3 : 6;
};


struct CobRpdo2 {
    uint16_t ctlloop : 2;
    uint16_t _reserved1 : 6;

    uint16_t _reserved2 : 8;

    int16_t ref_d_angle : 16;

    int16_t ref_d_current : 16;

    uint16_t _reserved3 : 8;

    uint16_t counter : 2;
    uint16_t _reserved4 : 6;
};


struct CobRpdo3 {
    uint16_t manual_field : 1;
    uint16_t _reserved1 : 15;

    uint16_t ref_f_current : 16;

    uint16_t _reserved2 : 16;

    uint16_t _reserved3 : 8;

    uint16_t counter : 2;
    uint16_t _reserved4 : 6;
};


enum class DriveState : uint8_t {
    init,
    standby,
    power_on,
    ready,
    start,
    working,
    stop,
    power_off,
    testing
};


inline const std::unordered_set<uint8_t> drive_state_values {
    std::to_underlying(DriveState::init),
    std::to_underlying(DriveState::standby),
    std::to_underlying(DriveState::power_on),
    std::to_underlying(DriveState::ready),
    std::to_underlying(DriveState::start),
    std::to_underlying(DriveState::working),
    std::to_underlying(DriveState::stop),
    std::to_underlying(DriveState::power_off),
    std::to_underlying(DriveState::testing),
};


inline const std::unordered_map<DriveState, std::string_view> drive_state_names = {
    {DriveState::init, "init"},
    {DriveState::standby, "standby"},
    {DriveState::power_on, "power_on"},
    {DriveState::ready, "ready"},
    {DriveState::start, "start"},
    {DriveState::working, "working"},
    {DriveState::stop, "stop"},
    {DriveState::power_off, "power_off"},
    {DriveState::testing, "testing"},
};


enum class OperatingMode : uint8_t {
    normal,
    testing,
};


inline const std::unordered_set<uint8_t> opmode_values = {
    std::to_underlying(OperatingMode::normal),
    std::to_underlying(OperatingMode::testing)
};

inline const std::unordered_map<OperatingMode, std::string_view> opmode_names = {
    {OperatingMode::normal, "normal"},
    {OperatingMode::testing, "testing"},
};


enum class ControlMode : uint8_t {
    torque,
    speed
};


inline const std::unordered_set<uint8_t> ctlmode_values = {
    std::to_underlying(ControlMode::torque),
    std::to_underlying(ControlMode::speed)
};


inline const std::unordered_map<ControlMode, std::string_view> ctlmode_names = {
    {ControlMode::torque, "torque"},
    {ControlMode::speed, "speed"}
};


enum class ControlLoop : uint8_t {
    closed,
    open,
    semiclosed
};


inline const std::unordered_set<uint8_t> ctlloop_values = {
    std::to_underlying(ControlLoop::closed),
    std::to_underlying(ControlLoop::open),
    std::to_underlying(ControlLoop::semiclosed)
};


inline const std::unordered_map<ControlLoop, std::string_view> ctlloop_names = {
    {ControlLoop::closed, "closed"},
    {ControlLoop::open, "open"},
    {ControlLoop::semiclosed, "semiclosed"}
};


} // namespace loco
