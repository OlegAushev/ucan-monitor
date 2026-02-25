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
  uint8_t control_mode : 2;
  uint8_t model_mode : 3;

  uint8_t _reserved1_;

  uint8_t _reserved2_;

  uint8_t _reserved3_;

  uint8_t _reserved4_;

  uint8_t _reserved5_;

  uint8_t counter : 2;
  uint8_t _reserved6_ : 6;
};

struct CobTpdo2 {
  int16_t dc_voltage;

  int16_t phase_current;

  uint8_t _reserved1_;

  uint8_t _reserved2_;

  uint8_t _reserved3_;

  uint8_t counter : 2;
  uint8_t _reserved4_ : 6;
};

struct CobTpdo3 {
  int16_t speed;

  int16_t angle;

  uint8_t _reserved1_;

  uint8_t _reserved2_;

  uint8_t _reserved3_;

  uint8_t counter : 2;
  uint8_t _reserved4_ : 6;
};

struct CobTpdo4 {
  uint32_t errors;
  uint16_t warnings;
  uint8_t domain : 8;
  uint8_t counter : 2;
  uint8_t _reserved_ : 6;
};

static_assert(sizeof(CobTpdo1) == 8);
static_assert(sizeof(CobTpdo2) == 8);
static_assert(sizeof(CobTpdo3) == 8);
static_assert(sizeof(CobTpdo4) == 8);

struct CobRpdo1 {
  // Byte 0: control bits
  uint8_t emergency : 1;
  uint8_t power : 1;
  uint8_t start : 1;
  uint8_t control_mode : 2;    // torque/speed/angle
  uint8_t model_mode : 3;      // idle/closed/semi/open_I/open_V

  uint8_t _reserved1_;         // Byte 1

  // Bytes 2-3: primary reference (meaning depends on control_mode)
  //   torque → signed_pu * 10000
  //   speed  → RPM int16
  //   angle  → edeg int16
  int16_t ref;

  // Bytes 4-5: auxiliary d-axis reference (meaning depends on model_mode)
  //   semiclosed/open_current → Idref * 10000
  //   open_voltage            → Vdref * 10000
  //   closed/idle             → ignored
  int16_t aux_ref;

  uint8_t _reserved2_;         // Byte 6

  // Byte 7
  uint8_t counter : 2;
  uint8_t _reserved3_ : 6;
};

static_assert(sizeof(CobRpdo1) == 8);

enum class DriveState {
  init,
  standby,
  powering_up,
  powering_down,
  ready,
  running,
  stopping,
  testing,
  calibrating,
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
    std::to_underlying(DriveState::calibrating),
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
        {DriveState::calibrating, "калибровка"},
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
        {DriveState::calibrating, "КАЛИБРОВКА"},
};

enum class ControlMode {
  torque,
  speed,
  angle
};

inline std::unordered_set<int> const ctlmode_values = {
    std::to_underlying(ControlMode::torque),
    std::to_underlying(ControlMode::speed),
    std::to_underlying(ControlMode::angle)};

inline std::unordered_map<ControlMode, std::string_view> const ctlmode_names = {
    {ControlMode::torque, "момент"},
    {ControlMode::speed, "скорость"},
    {ControlMode::angle, "угол"}};

enum class ModelMode {
  idle,
  closed,
  semiclosed,
  open_current,
  open_voltage
};

inline std::unordered_set<int> const model_mode_values = {
    std::to_underlying(ModelMode::idle),
    std::to_underlying(ModelMode::closed),
    std::to_underlying(ModelMode::semiclosed),
    std::to_underlying(ModelMode::open_current),
    std::to_underlying(ModelMode::open_voltage)};

inline std::unordered_map<ModelMode, std::string_view> const model_mode_names = {
    {ModelMode::idle, "ожидание"},
    {ModelMode::closed, "замкнутый"},
    {ModelMode::semiclosed, "полузамкнутый"},
    {ModelMode::open_current, "разомкнутый I"},
    {ModelMode::open_voltage, "разомкнутый V"}};

} // namespace sevpress
