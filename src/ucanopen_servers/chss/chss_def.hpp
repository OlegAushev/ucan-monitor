#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include "chss_def_sys.hpp"

// PDO layouts mirror src/common/contract/chss/pdo.hpp of the h2-hess firmware.
// Keep both in sync.

namespace chss {

struct CobTpdo1 {
  uint64_t flags : status::status_count;

  uint64_t _reserved1_ : 64 - status::status_count - 8 - 8;

  uint64_t level : 2;
  uint64_t _reserved2_ : 6;

  uint64_t counter : 2;
  uint64_t _reserved3_ : 6;
};

struct CobTpdo2 {
  int16_t receiver_pressure;  // 0.1 atm/LSB, Р1-1

  int16_t inflow_rate;        // 0.1 l/min/LSB, ИР1

  int16_t fill_line_pressure; // 0.1 atm/LSB, Р1

  uint8_t inlet_valve;        // ValvePosition, К7

  uint8_t counter : 2;
  uint8_t _reserved1_ : 6;
};

struct CobTpdo3 {
  int16_t pressure_before_reducer; // 0.1 atm/LSB, Р2

  int16_t pressure_after_reducer;  // 0.1 atm/LSB, Р3

  uint8_t _reserved1_;

  uint8_t _reserved2_;

  uint8_t outlet_valve;            // ValvePosition, К9

  uint8_t counter : 2;
  uint8_t _reserved3_ : 6;
};

struct CobTpdo4 {
  int16_t mcu_temperature; // 0.1 degC/LSB

  uint8_t mode;            // Mode, сообщаемый узлом

  uint8_t flags;           // supply_ready_mask | receiver_full_mask

  uint8_t state;           // State, только для диагностики

  uint8_t _reserved1_;

  uint8_t _reserved2_;

  uint8_t counter : 2;
  uint8_t _reserved3_ : 6;
};

static_assert(sizeof(CobTpdo1) == 8);
static_assert(sizeof(CobTpdo2) == 8);
static_assert(sizeof(CobTpdo3) == 8);
static_assert(sizeof(CobTpdo4) == 8);

inline constexpr uint8_t supply_ready_mask = 0x1;
inline constexpr uint8_t receiver_full_mask = 0x2;

// Режим, запрошенный мастером, и ручные уровни клапанов. Уровни действуют,
// только пока СХКВ в продувке. Прошивка отвергает кадр с ненулевыми резервными
// полями или со счётчиком не по порядку.
struct CobRpdo1 {
  uint8_t mode;        // Byte 0: режим (Mode)

  uint8_t inlet_cmd;   // Byte 1: ValvePosition К7; только продувка
  uint8_t outlet_cmd;  // Byte 2: ValvePosition К9; только продувка

  uint8_t _reserved1_; // Byte 3
  uint8_t _reserved2_; // Byte 4
  uint8_t _reserved3_; // Byte 5
  uint8_t _reserved4_; // Byte 6

  // Byte 7
  uint8_t counter : 2;
  uint8_t _reserved5_ : 6;
};

static_assert(sizeof(CobRpdo1) == 8);

// Линия подачи, которую стенд передаёт вместо датчиков, объявленных
// неустановленными: СХКВ берёт из кадра только их и слушает кадр, лишь пока
// такой датчик объявлен. Р1-1, Р2 и Р3 идут одним кадром, потому что СХКВ
// сравнивает их (перепад на фильтре Ф2 — Р1-1 − Р2). Кодируются так же, как в
// телеметрии.
struct CobRpdo2 {
  int16_t receiver_pressure;       // 0.1 atm/LSB, Р1-1

  int16_t pressure_before_reducer; // 0.1 atm/LSB, Р2

  int16_t pressure_after_reducer;  // 0.1 atm/LSB, Р3

  uint8_t _reserved1_;

  uint8_t counter : 2;
  uint8_t _reserved2_ : 6;
};

// Линия заправки, передаётся так же; ИР1 — в л/мин, как в телеметрии.
struct CobRpdo3 {
  int16_t fill_line_pressure; // 0.1 atm/LSB, Р1

  int16_t inflow_rate;        // 0.1 l/min/LSB, ИР1

  uint8_t _reserved1_;

  uint8_t _reserved2_;

  uint8_t _reserved3_;

  uint8_t counter : 2;
  uint8_t _reserved4_ : 6;
};

static_assert(sizeof(CobRpdo2) == 8);
static_assert(sizeof(CobRpdo3) == 8);

// Подставляемые величины в порядке кадров: RPDO2, затем RPDO3.
enum class Substitute : uint8_t {
  receiver_pressure,       // Р1-1, атм
  pressure_before_reducer, // Р2, атм
  pressure_after_reducer,  // Р3, атм
  fill_line_pressure,      // Р1, атм
  inflow_rate,             // ИР1, л/мин
};

inline constexpr size_t substitute_count = 5;

// Код с шагом 0.1, как у emb::scaled в прошивке: округление к ближайшему и
// насыщение на краях int16.
inline int16_t encode_deci(float v) {
  float const code =
      std::clamp(v * 10.f,
                 static_cast<float>(std::numeric_limits<int16_t>::min()),
                 static_cast<float>(std::numeric_limits<int16_t>::max()));
  return static_cast<int16_t>(std::lround(code));
}

// Mirrors contract::chss::mode (common/contract/chss/mode.hpp).
enum class Mode : uint8_t {
  storage,
  filling,
  supply,
  purge
};

inline std::unordered_set<int> const mode_values = {
    std::to_underlying(Mode::storage),
    std::to_underlying(Mode::filling),
    std::to_underlying(Mode::supply),
    std::to_underlying(Mode::purge)};

inline std::unordered_map<Mode, std::string_view> const mode_names = {
    {Mode::storage, "хранение"},
    {Mode::filling, "заправка"},
    {Mode::supply, "подача"},
    {Mode::purge, "продувка"}};

inline std::unordered_map<Mode, std::string_view> const mode_names_upper = {
    {Mode::storage, "ХРАНЕНИЕ"},
    {Mode::filling, "ЗАПРАВКА"},
    {Mode::supply, "ПОДАЧА"},
    {Mode::purge, "ПРОДУВКА"}};

// Mirrors control::state::id (app/chss/control/fsm.hpp). Контракт не
// фиксирует эти значения: они принадлежат прошивке и меняются вместе с ней.
enum class State : uint8_t {
  storage,  // клапаны закрыты, ресивер под контролем утечки
  filling,  // К7 открыт до заполнения ресивера
  supply,   // К9 открыт, подача на редуктор
  purge,    // клапаны отданы уровням из кадра
  lockout,  // критическая неисправность: клапаны закрыты
};

inline std::unordered_set<int> const state_values = {
    std::to_underlying(State::storage),
    std::to_underlying(State::filling),
    std::to_underlying(State::supply),
    std::to_underlying(State::purge),
    std::to_underlying(State::lockout)};

inline std::unordered_map<State, std::string_view> const state_names = {
    {State::storage, "хранение"},
    {State::filling, "заправка"},
    {State::supply, "подача"},
    {State::purge, "продувка"},
    {State::lockout, "блокировка"},
};

// Mirrors contract::valve_position (common/contract/position.hpp).
enum class ValvePosition : uint8_t {
  closed,
  open
};

inline std::unordered_set<int> const valve_position_values = {
    std::to_underlying(ValvePosition::closed),
    std::to_underlying(ValvePosition::open)};

inline std::unordered_map<ValvePosition, std::string_view> const
    valve_position_names = {
        {ValvePosition::closed, "закрыт"},
        {ValvePosition::open, "открыт"},
};

} // namespace chss
