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

#include "cshpp_def_sys.hpp"

// PDO layouts mirror src/common/contract/cshpp/pdo.hpp and the position bytes
// src/common/contract/position.hpp of the h2-hess firmware. Keep both in sync.

namespace cshpp {

// Байт положения клапана в TPDO.
struct ValveReport {
  bool open : 1;
  uint8_t _reserved_ : 7;
};

static_assert(sizeof(ValveReport) == 1);

inline constexpr bool is_valve_report(ValveReport r) {
  return r._reserved_ == 0;
}

// Байт ручного уровня клапана в RPDO.
struct ValveRequest {
  bool open : 1;
  uint8_t _reserved_ : 7;
};

static_assert(sizeof(ValveRequest) == 1);

// Байт ручного уровня контактора в RPDO.
struct ContactorRequest {
  bool close : 1;
  uint8_t _reserved_ : 7;
};

static_assert(sizeof(ContactorRequest) == 1);

struct CobTpdo1 {
  uint64_t flags : status::status_count;

  uint64_t _reserved1_ : 64 - status::status_count - 8 - 8;

  uint64_t level : 2;
  uint64_t _reserved2_ : 6;

  uint64_t counter : 2;
  uint64_t _reserved3_ : 6;
};

inline constexpr bool is_valid(CobTpdo1 const& t) {
  return t._reserved1_ == 0 && t._reserved2_ == 0 && t._reserved3_ == 0;
}

struct CobTpdo2 {
  int16_t hydrogen_pressure; // 0.1 atm/LSB, Р1-1

  int16_t hydrogen_flow;     // 0.1 l/min/LSB, ИР1-1

  uint8_t mode;              // Mode, сообщаемый установкой

  uint8_t state;             // Stage, только для диагностики

  uint8_t _reserved1_;

  uint8_t counter : 2;
  uint8_t _reserved2_ : 6;
};

struct CobTpdo3 {
  ValveReport hydrogen_supply; // К1-3

  ValveReport fuelcell_inlet;  // К1-4

  uint8_t _reserved1_;

  uint8_t _reserved2_;

  uint8_t _reserved3_;

  uint8_t _reserved4_;

  uint8_t _reserved5_;

  uint8_t counter : 2;
  uint8_t _reserved6_ : 6;
};

inline constexpr bool is_valid(CobTpdo3 const& t) {
  return is_valve_report(t.hydrogen_supply)
      && is_valve_report(t.fuelcell_inlet)
      && t._reserved1_ == 0
      && t._reserved2_ == 0
      && t._reserved3_ == 0
      && t._reserved4_ == 0
      && t._reserved5_ == 0
      && t._reserved6_ == 0;
}

static_assert(sizeof(CobTpdo1) == 8);
static_assert(status::status_count <= 48);
static_assert(sizeof(CobTpdo2) == 8);
static_assert(sizeof(CobTpdo3) == 8);

// Режим, который станция оператора просит у установки. Режимы СХКВ и PDU
// установка выбирает сама по своему этапу: узлом, которым она командует,
// больше никто не командует.
struct CobRpdo1 {
  uint8_t mode;        // Byte 0: режим (Mode)

  uint8_t _reserved1_; // Byte 1
  uint8_t _reserved2_; // Byte 2
  uint8_t _reserved3_; // Byte 3
  uint8_t _reserved4_; // Byte 4
  uint8_t _reserved5_; // Byte 5
  uint8_t _reserved6_; // Byte 6

  // Byte 7
  uint8_t counter : 2;
  uint8_t _reserved7_ : 6;
};

// Ручные уровни клапанов продувки. Установка исполняет их только в продувке;
// до клапанов СХКВ станция дотягивается через неё, но лишь до уровней.
// Незаполненный уровень читается как «закрыт».
struct CobRpdo2 {
  ValveRequest hydrogen_supply; // К1-3
  ValveRequest fuelcell_inlet;  // К1-4
  ValveRequest storage_inlet;   // К7
  ValveRequest storage_outlet;  // К9

  uint8_t _reserved1_;
  uint8_t _reserved2_;
  uint8_t _reserved3_;

  uint8_t counter : 2;
  uint8_t _reserved4_ : 6;
};

// Ручные уровни контакторов PDU в той же продувке, через установку так же.
// Незаполненный уровень читается как «разомкнут».
struct CobRpdo3 {
  ContactorRequest fuelcell_main;      // KM1
  ContactorRequest fuelcell_precharge; // KM2
  ContactorRequest inverter_main;      // KM3
  ContactorRequest inverter_precharge; // KM4

  uint8_t _reserved1_;
  uint8_t _reserved2_;
  uint8_t _reserved3_;

  uint8_t counter : 2;
  uint8_t _reserved4_ : 6;
};

// Линия самой установки, которую стенд передаёт вместо датчиков, объявленных
// неустановленными: установка берёт из кадра только их и слушает кадр, лишь
// пока такой датчик объявлен. Кодируются так же, как в телеметрии.
struct CobRpdo4 {
  int16_t hydrogen_pressure; // 0.1 atm/LSB, Р1-1

  int16_t hydrogen_flow;     // 0.1 l/min/LSB, ИР1-1

  uint8_t _reserved1_;

  uint8_t _reserved2_;

  uint8_t _reserved3_;

  uint8_t counter : 2;
  uint8_t _reserved4_ : 6;
};

static_assert(sizeof(CobRpdo1) == 8);
static_assert(sizeof(CobRpdo2) == 8);
static_assert(sizeof(CobRpdo3) == 8);
static_assert(sizeof(CobRpdo4) == 8);

// Клапаны в порядке RPDO2.
enum class Valve : uint8_t {
  hydrogen_supply, // К1-3
  fuelcell_inlet,  // К1-4
  storage_inlet,   // К7
  storage_outlet,  // К9
};

inline constexpr size_t valve_count = 4;

// Контакторы PDU в порядке RPDO3.
enum class Contactor : uint8_t {
  fuelcell_main,      // KM1
  fuelcell_precharge, // KM2
  inverter_main,      // KM3
  inverter_precharge, // KM4
};

inline constexpr size_t contactor_count = 4;

// Подставляемые величины в порядке RPDO4.
enum class Substitute : uint8_t {
  hydrogen_pressure, // Р1-1, атм
  hydrogen_flow,     // ИР1-1, л/мин
};

inline constexpr size_t substitute_count = 2;

// Код с шагом 0.1, как у emb::scaled в прошивке: округление к ближайшему и
// насыщение на краях int16.
inline int16_t encode_deci(float v) {
  float const code =
      std::clamp(v * 10.f,
                 static_cast<float>(std::numeric_limits<int16_t>::min()),
                 static_cast<float>(std::numeric_limits<int16_t>::max()));
  return static_cast<int16_t>(std::lround(code));
}

// Mirrors contract::cshpp::mode (common/contract/cshpp/mode.hpp).
enum class Mode : uint8_t {
  idle,
  filling,
  generation,
  purge
};

inline std::unordered_set<int> const mode_values = {
    std::to_underlying(Mode::idle),
    std::to_underlying(Mode::filling),
    std::to_underlying(Mode::generation),
    std::to_underlying(Mode::purge)};

inline std::unordered_map<Mode, std::string_view> const mode_names = {
    {Mode::idle, "ожидание"},
    {Mode::filling, "заправка"},
    {Mode::generation, "генерация"},
    {Mode::purge, "продувка"}};

inline constexpr bool is_valid(CobTpdo2 const& t) {
  return (t.mode == std::to_underlying(Mode::idle)
          || t.mode == std::to_underlying(Mode::filling)
          || t.mode == std::to_underlying(Mode::generation)
          || t.mode == std::to_underlying(Mode::purge))
      && t._reserved1_ == 0
      && t._reserved2_ == 0;
}

// Mirrors control::stage (app/cshpp/control/fsm.hpp). Контракт не фиксирует
// эти значения: они принадлежат прошивке и меняются вместе с ней. Режимов
// меньше, чем этапов: генерация — это процедура, а останов переживает
// начавшую его команду.
enum class Stage : uint8_t {
  idle,
  purge,
  filling,           // СХКВ заправляется от электролизёра, установка ждёт
  gen_await_storage, // СХКВ переведена в выдачу: ждём К9 и Р2, Р3
  gen_leak_test,     // К1-3 открыт, К1-4 закрыт: проверка ИР1-1 и Р1-1/Р3
  gen_run,           // К1-4 открыт, ЭХГ запущен
  stopping,          // сигнал останова ЭХГ, затем закрытие линии
};

inline std::unordered_map<Stage, std::string_view> const stage_names = {
    {Stage::idle, "ожидание"},
    {Stage::purge, "продувка"},
    {Stage::filling, "заправка"},
    {Stage::gen_await_storage, "ожидание выдачи СХКВ"},
    {Stage::gen_leak_test, "проверка утечки"},
    {Stage::gen_run, "генерация"},
    {Stage::stopping, "останов"},
};

// Mirrors contract::valve_position (common/contract/position.hpp).
enum class ValvePosition : uint8_t {
  closed,
  open
};

inline std::unordered_map<ValvePosition, std::string_view> const
    valve_position_names = {
        {ValvePosition::closed, "закрыт"},
        {ValvePosition::open, "открыт"},
};

// Mirrors contract::contactor_position (common/contract/position.hpp).
enum class ContactorPosition : uint8_t {
  open,
  closed
};

inline constexpr ValvePosition unpack_valve_bit(bool b) {
  return b ? ValvePosition::open : ValvePosition::closed;
}

inline constexpr ValveRequest pack_valve_request(ValvePosition p) {
  return {.open = p == ValvePosition::open, ._reserved_ = 0};
}

inline constexpr ContactorRequest pack_contactor_request(ContactorPosition p) {
  return {.close = p == ContactorPosition::closed, ._reserved_ = 0};
}

} // namespace cshpp
