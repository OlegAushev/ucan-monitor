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

#include "psfb_def_sys.hpp"

// PDO layouts mirror src/app/converter/comm/can/canopen/pdo_layouts.hpp of the
// adpt-etk-psfb-converter firmware. Keep both in sync.

namespace psfb {

struct CobTpdo1 {
  uint8_t converter_state : 8;

  uint8_t pwm1_on : 1;
  uint8_t pwm2_on : 1;
  uint8_t warning : 1;
  uint8_t error : 1;
  uint8_t critical : 1;
  uint8_t emergency : 1;
  uint8_t _reserved1_ : 2;

  uint16_t overlap1;
  uint16_t overlap2;

  uint8_t _reserved2_;

  uint8_t counter : 2;
  uint8_t _reserved3_ : 6;
};

struct CobTpdo2 {
  int16_t hv_voltage; // V * 10
  int16_t lv_voltage; // V * 10

  uint8_t _reserved1_;
  uint8_t _reserved2_;
  uint8_t _reserved3_;

  uint8_t counter : 2;
  uint8_t _reserved4_ : 6;
};

struct CobTpdo3 {
  int16_t current1; // A * 10
  int16_t current2; // A * 10

  uint8_t _reserved1_;
  uint8_t _reserved2_;
  uint8_t _reserved3_;

  uint8_t counter : 2;
  uint8_t _reserved4_ : 6;
};

struct CobTpdo4 {
  uint64_t flags : sys::status::status_count;

  uint64_t _reserved1_ : 64 - sys::status::status_count - 8 - 8;

  uint64_t level : 2;
  uint64_t _reserved2_ : 6;

  uint64_t counter : 2;
  uint64_t _reserved3_ : 6;
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
  uint8_t _reserved1_ : 5;

  uint8_t _reserved2_;         // Byte 1

  uint16_t overlap1;           // Bytes 2-3: unsigned_pu * 10000
  uint16_t overlap2;           // Bytes 4-5: unsigned_pu * 10000

  uint8_t _reserved3_;         // Byte 6

  // Byte 7
  uint8_t counter : 2;
  uint8_t _reserved4_ : 6;
};

static_assert(sizeof(CobRpdo1) == 8);

// The scale of the per-unit setpoints carried by the PDOs above.
inline constexpr float overlap_scale = 10000.f;

// Mirrors dcdc::state::id (dcdc/fsm/fsm.hpp).
enum class ConverterState {
  init,
  standby,
  ready,
  running,
  fault,
};

inline std::unordered_set<int> const converter_state_values = {
    std::to_underlying(ConverterState::init),
    std::to_underlying(ConverterState::standby),
    std::to_underlying(ConverterState::ready),
    std::to_underlying(ConverterState::running),
    std::to_underlying(ConverterState::fault),
};

inline std::unordered_map<ConverterState, std::string_view> const
    converter_state_names = {
        {ConverterState::init, "инициализация"},
        {ConverterState::standby, "ожидание"},
        {ConverterState::ready, "готов"},
        {ConverterState::running, "работа"},
        {ConverterState::fault, "неисправность"},
};

inline std::unordered_map<ConverterState, std::string_view> const
    converter_state_names_upper = {
        {ConverterState::init, "ИНИЦИАЛИЗАЦИЯ"},
        {ConverterState::standby, "ОЖИДАНИЕ"},
        {ConverterState::ready, "ГОТОВ"},
        {ConverterState::running, "РАБОТА"},
        {ConverterState::fault, "НЕИСПРАВНОСТЬ"},
};

} // namespace psfb
