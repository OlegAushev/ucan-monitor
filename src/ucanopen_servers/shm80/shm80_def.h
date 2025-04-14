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

#include "shm80_def_syslog.h"

namespace shm80 {

struct CobTpdo1 {
    uint16_t drive_state : 8;

    uint16_t pwm_on : 1;
    uint16_t error : 1;
    uint16_t warning : 1;
    uint16_t opmode : 2;
    uint16_t ctlmode : 1;
    uint16_t ctlloop : 2;

    uint16_t pdu_precharge_state : 3;
    uint16_t instester_state : 3;
    uint16_t manual_field : 1;
    uint16_t _reserved1_ : 1;

    uint16_t pdu_main_contactor : 1;
    uint16_t pdu_charging_contactor : 1;
    uint16_t pdu_bypassing_contactor : 1;
    uint16_t _reserved2_ : 5;

    uint16_t _reserved3_;

    uint16_t din_ship_failure_warning : 1;
    uint16_t din_ship_failure : 1;
    uint16_t din_start : 1;
    uint16_t dout_power_request : 1;
    uint16_t dout_drive_ready : 1;
    uint16_t dout_drive_started : 1;
    uint16_t dout_drive_failure : 1;
    uint16_t _reserved4_ : 1;

    uint16_t counter : 2;
    uint16_t _reserved5_ : 6;
};

struct CobTpdo2 {
    int16_t dc_voltage;

    int16_t stator_current;

    uint16_t _reserved1_;

    uint16_t field_current : 8;

    uint16_t counter : 2;
    uint16_t _reserved2_ : 6;
};

struct CobTpdo3 {
    int16_t speed;

    int16_t angle;

    uint16_t _reserved1_;

    uint16_t _reserved2_ : 8;

    uint16_t counter : 2;
    uint16_t _reserved3_ : 6;
};

struct CobTpdo4 {
    uint32_t errors;
    uint16_t warnings;
    uint16_t domain : 8;
    uint16_t counter : 2;
    uint16_t _reserved_ : 6;
};

struct CobRpdo1 {
    uint16_t emergency_stop : 1;
    uint16_t power : 1;
    uint16_t start : 1;
    uint16_t ctlmode : 1;
    uint16_t isotest_dis : 1;
    uint16_t _reserved1_ : 3;

    uint16_t _reserved2_ : 8;

    int16_t ref_torque;

    int16_t ref_speed;

    uint16_t _reserved3_ : 8;

    uint16_t counter : 2;
    uint16_t _reserved4_ : 6;
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
    uint16_t _reserved_ : 1;
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
            {DriveState::init, "инициализация"},
            {DriveState::standby, "ожидание"},
            {DriveState::powering_up, "включение"},
            {DriveState::powering_down, "выключение"},
            {DriveState::ready, "готов"},
            {DriveState::running, "работа"},
            {DriveState::stopping, "останов"},
            {DriveState::calibrating, "калибровка"},
            {DriveState::testing, "тестирование"},
};

enum class OperatingMode { normal, testing };

inline const std::unordered_set<int> opmode_values = {
    std::to_underlying(OperatingMode::normal),
    std::to_underlying(OperatingMode::testing),
};

inline const std::map<OperatingMode, std::string_view> opmode_names = {
    {OperatingMode::normal, "нормальный"},
    {OperatingMode::testing, "тестирование"},
};

enum class ControlMode { torque, speed };

inline const std::unordered_set<int> ctlmode_values = {
    std::to_underlying(ControlMode::torque),
    std::to_underlying(ControlMode::speed)};

inline const std::unordered_map<ControlMode, std::string_view> ctlmode_names = {
    {ControlMode::torque, "момент"},
    {ControlMode::speed, "скорость"}};

enum class ControlLoop { closed, open, semiclosed, openvolt };

inline const std::unordered_set<int> ctlloop_values = {
    std::to_underlying(ControlLoop::closed),
    std::to_underlying(ControlLoop::open),
    std::to_underlying(ControlLoop::semiclosed),
    std::to_underlying(ControlLoop::openvolt)};

inline const std::unordered_map<ControlLoop, std::string_view> ctlloop_names = {
    {ControlLoop::closed, "замкнутый"},
    {ControlLoop::open, "разомкнутый"},
    {ControlLoop::semiclosed, "замкнутый 2"},
    {ControlLoop::openvolt, "разомкнутый 2"}};

inline const std::vector<std::string_view> precharge_states =
        {"...", "откл", "заряд", "пауза", "готов", "разряд", "ошибка"};

inline const std::vector<std::string_view> insulation_tester_states =
        {"ини-ция", "ожидание", "изм+", "изм-", "обработка"};

} // namespace shm80
