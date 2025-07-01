#pragma once

#include <array>
#include <cstdint>
#include <cstring>
#include <map>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "sevpress_def_syslog.hpp"

namespace sevpress {

struct CobTpdo1 {
    uint8_t drive_state : 8;

    uint8_t pwm_on : 1;
    uint8_t error : 1;
    uint8_t warning : 1;
    uint8_t opmode : 2;
    uint8_t ctlmode : 1;
    uint8_t ctlloop : 2;

    uint8_t _reserved1_;

    uint8_t _reserved2_;

    uint8_t _reserved3_;

    uint8_t _reserved4_;

    uint8_t _reserved5_;

    uint8_t counter : 2;
    uint8_t _reserved6_ : 6;
};
static_assert(sizeof(CobTpdo1) == 8);

struct CobTpdo2 {
    int16_t dc_voltage;

    int16_t phase_current;

    uint8_t _reserved1_;

    uint8_t _reserved2_;

    uint8_t _reserved3_;

    uint8_t counter : 2;
    uint8_t _reserved4_ : 6;
};
static_assert(sizeof(CobTpdo2) == 8);

struct CobTpdo3 {
    int16_t speed;

    int16_t angle;

    uint8_t _reserved1_;

    uint8_t _reserved2_;

    uint8_t _reserved3_;

    uint8_t counter : 2;
    uint8_t _reserved4_ : 6;
};
static_assert(sizeof(CobTpdo3) == 8);

struct CobTpdo4 {
    uint32_t errors;
    uint16_t warnings;
    uint8_t domain : 8;
    uint8_t counter : 2;
    uint8_t _reserved_ : 6;
};
static_assert(sizeof(CobTpdo4) == 8);

struct CobRpdo1 {
    uint8_t emergency : 1;
    uint8_t power : 1;
    uint8_t start : 1;
    uint8_t ctlmode : 1;
    uint8_t _reserved1_ : 4;

    uint8_t _reserved2_;

    int16_t torque_ref;

    int16_t speed_ref;

    uint8_t _reserved3_;

    uint8_t counter : 2;
    uint8_t _reserved4_ : 6;
};
static_assert(sizeof(CobRpdo1) == 8);

struct CobRpdo2 {
    int16_t angle_ref;

    int16_t current_ref;

    uint16_t voltage_ref;

    uint8_t _reserved1_;

    uint8_t counter : 2;
    uint8_t opmode : 2;
    uint8_t ctlloop : 2;
    uint8_t _reserved2_ : 2;
};
static_assert(sizeof(CobRpdo2) == 8);

enum class DriveState {
  init,
  standby,
  powering_up,
  powering_down,
  ready,
  running,
  stopping,
  testing,
};

inline std::unordered_set<int> const drive_state_values = {
    std::to_underlying(DriveState::init),
    std::to_underlying(DriveState::standby),
    std::to_underlying(DriveState::powering_up),
    std::to_underlying(DriveState::powering_down),
    std::to_underlying(DriveState::ready),
    std::to_underlying(DriveState::running),
    std::to_underlying(DriveState::stopping),
    std::to_underlying(DriveState::testing),
    // std::to_underlying(DriveState::calibrating),
};

inline std::unordered_map<DriveState, std::string_view> const
    drive_state_names = {
        {DriveState::init, "инициализация"},
        {DriveState::standby, "ожидание"},
        {DriveState::powering_up, "включение"},
        {DriveState::powering_down, "выключение"},
        {DriveState::ready, "готов"},
        {DriveState::running, "работа"},
        {DriveState::stopping, "останов"},
        {DriveState::testing, "тестирование"},
        // {DriveState::calibrating, "калибровка"},
};

inline std::unordered_map<DriveState, std::string_view> const
    drive_state_names_upper = {
        {DriveState::init, "ИНИЦИАЛИЗАЦИЯ"},
        {DriveState::standby, "ОЖИДАНИЕ"},
        {DriveState::powering_up, "ВКЛЮЧЕНИЕ"},
        {DriveState::powering_down, "ВЫКЛЮЧЕНИЕ"},
        {DriveState::ready, "ГОТОВ"},
        {DriveState::running, "РАБОТА"},
        {DriveState::stopping, "ОСТАНОВ"},
        {DriveState::testing, "ТЕСТИРОВАНИЕ"},
        // {DriveState::calibrating, "КАЛИБРОВКА"},
};

enum class OperatingMode {
  normal,
  testing
};

inline std::unordered_set<int> const opmode_values = {
    std::to_underlying(OperatingMode::normal),
    std::to_underlying(OperatingMode::testing),
};

inline std::map<OperatingMode, std::string_view> const opmode_names = {
    {OperatingMode::normal, "нормальный"},
    {OperatingMode::testing, "тестирование"},
};

enum class ControlMode {
  torque,
  speed
};

inline std::unordered_set<int> const ctlmode_values = {
    std::to_underlying(ControlMode::torque),
    std::to_underlying(ControlMode::speed)};

inline std::unordered_map<ControlMode, std::string_view> const ctlmode_names = {
    {ControlMode::torque, "момент"}, {ControlMode::speed, "скорость"}};

enum class ControlLoop {
  closed,
  open,
  semiclosed,
  openvolt
};

inline std::unordered_set<int> const ctlloop_values = {
    std::to_underlying(ControlLoop::closed),
    std::to_underlying(ControlLoop::open),
    std::to_underlying(ControlLoop::semiclosed),
    std::to_underlying(ControlLoop::openvolt)};

inline std::unordered_map<ControlLoop, std::string_view> const ctlloop_names = {
    {ControlLoop::closed, "замкнутый"},
    {ControlLoop::open, "разомкнутый"},
    {ControlLoop::semiclosed, "замкнутый 2"},
    {ControlLoop::openvolt, "разомкнутый 2"}};

} // namespace sevpress
