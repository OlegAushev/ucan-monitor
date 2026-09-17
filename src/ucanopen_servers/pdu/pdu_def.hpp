#pragma once

#include <cstdint>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include "pdu_def_sys.hpp"

// PDO layouts mirror src/common/contract/pdu/pdo.hpp of the h2-hess firmware.
// Keep both in sync.

namespace pdu {

// Байт положения контактора в TPDO: заданное и фактическое положения.
struct ContactorReport {
  bool demanded : 1;
  bool actual : 1;
  uint8_t _reserved_ : 6;
};

static_assert(sizeof(ContactorReport) == 1);

inline constexpr bool is_contactor_report(ContactorReport r) {
  return r._reserved_ == 0;
}

// Байт ручного уровня контактора в RPDO.
struct ContactorRequest {
  bool close : 1;
  uint8_t _reserved_ : 7;
};

static_assert(sizeof(ContactorRequest) == 1);

// Байт ветви в TPDO. stage — собственная нумерация прошивки: её показывают, но
// по ней не ветвятся — для этого есть connected.
struct BranchReport {
  uint8_t stage : 4;
  bool connected : 1; // главный замкнут, предзарядный снят
  uint8_t _reserved_ : 3;
};

static_assert(sizeof(BranchReport) == 1);

inline constexpr bool is_branch_report(BranchReport r) {
  return r._reserved_ == 0;
}

// Байт запроса на подключение ветви в RPDO.
struct BranchRequest {
  bool connect : 1;
  uint8_t _reserved_ : 7;
};

static_assert(sizeof(BranchRequest) == 1);

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
  int16_t fuelcell_voltage; // 0.1 V/LSB

  int16_t fuelcell_current; // 0.1 A/LSB

  ContactorReport fuelcell_main;

  ContactorReport fuelcell_precharge;

  BranchReport fuelcell_branch;

  uint8_t counter : 2;
  uint8_t _reserved1_ : 6;
};

inline constexpr bool is_valid(CobTpdo2 const& t) {
  return is_contactor_report(t.fuelcell_main)
      && is_contactor_report(t.fuelcell_precharge)
      && is_branch_report(t.fuelcell_branch)
      && t._reserved1_ == 0;
}

struct CobTpdo3 {
  int16_t inverter_voltage; // 0.1 V/LSB

  int16_t inverter_current; // 0.1 A/LSB

  ContactorReport inverter_main;

  ContactorReport inverter_precharge;

  BranchReport inverter_branch;

  uint8_t counter : 2;
  uint8_t _reserved1_ : 6;
};

inline constexpr bool is_valid(CobTpdo3 const& t) {
  return is_contactor_report(t.inverter_main)
      && is_contactor_report(t.inverter_precharge)
      && is_branch_report(t.inverter_branch)
      && t._reserved1_ == 0;
}

struct CobTpdo4 {
  int16_t battery_voltage; // 0.1 V/LSB

  int16_t mcu_temperature; // 0.1 degC/LSB

  uint8_t _reserved1_;

  uint8_t _reserved2_;

  uint8_t _reserved3_;

  uint8_t counter : 2;
  uint8_t _reserved4_ : 6;
};

inline constexpr bool is_valid(CobTpdo4 const& t) {
  return t._reserved1_ == 0
      && t._reserved2_ == 0
      && t._reserved3_ == 0
      && t._reserved4_ == 0;
}

static_assert(sizeof(CobTpdo1) == 8);
static_assert(status::status_count <= 48);
static_assert(sizeof(CobTpdo2) == 8);
static_assert(sizeof(CobTpdo3) == 8);
static_assert(sizeof(CobTpdo4) == 8);

// Режим установки, объявленный мастером, и запрос на подключение ветви
// инвертора. Ручные уровни контакторов действуют, только пока установка в
// продувке. PDU отбрасывает кадр с ненулевыми резервными битами или со
// сбившимся счётчиком.
struct CobRpdo1 {
  uint8_t mode;                        // Byte 0: режим установки (Mode)

  BranchRequest inverter_request;      // Byte 1

  ContactorRequest fuelcell_main;      // Byte 2: только продувка
  ContactorRequest fuelcell_precharge; // Byte 3: только продувка
  ContactorRequest inverter_main;      // Byte 4: только продувка
  ContactorRequest inverter_precharge; // Byte 5: только продувка

  uint8_t _reserved1_;                 // Byte 6

  // Byte 7
  uint8_t counter : 2;
  uint8_t _reserved2_ : 6;
};

static_assert(sizeof(CobRpdo1) == 8);

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

inline std::unordered_map<Mode, std::string_view> const mode_names_upper = {
    {Mode::idle, "ОЖИДАНИЕ"},
    {Mode::filling, "ЗАПРАВКА"},
    {Mode::generation, "ГЕНЕРАЦИЯ"},
    {Mode::purge, "ПРОДУВКА"}};

// Mirrors control::state::id (app/pdu/control/fsm.hpp). На проводе — поле
// BranchReport::stage (до 16 значений).
enum class BranchState : uint8_t {
  disconnected, // оба контактора разомкнуты
  precharging,  // предзарядный замкнут: ёмкость ветви набирает напряжение
  switchover,   // главный замкнут, идёт выдержка перед снятием предзаряда
  connected,    // главный замкнут, предзарядный разомкнут
  lockout,      // попытка не удалась: ждём, пока запрос снимут
  manual,       // контакторы отданы уровням из кадра (продувка)
};

inline std::unordered_map<BranchState, std::string_view> const
    branch_state_names = {
        {BranchState::disconnected, "отключена"},
        {BranchState::precharging, "предзаряд"},
        {BranchState::switchover, "переключение"},
        {BranchState::connected, "подключена"},
        {BranchState::lockout, "блокировка"},
        {BranchState::manual, "ручной режим"},
};

inline std::unordered_map<BranchState, std::string_view> const
    branch_state_names_upper = {
        {BranchState::disconnected, "ОТКЛЮЧЕНА"},
        {BranchState::precharging, "ПРЕДЗАРЯД"},
        {BranchState::switchover, "ПЕРЕКЛЮЧЕНИЕ"},
        {BranchState::connected, "ПОДКЛЮЧЕНА"},
        {BranchState::lockout, "БЛОКИРОВКА"},
        {BranchState::manual, "РУЧНОЙ РЕЖИМ"},
};

// Mirrors contract::contactor_position (common/contract/position.hpp).
enum class ContactorPosition : uint8_t {
  open,
  closed
};

inline std::unordered_map<ContactorPosition, std::string_view> const
    contactor_position_names = {
        {ContactorPosition::open, "разомкнут"},
        {ContactorPosition::closed, "замкнут"},
};

inline constexpr ContactorPosition unpack_contactor_bit(bool b) {
  return b ? ContactorPosition::closed : ContactorPosition::open;
}

inline constexpr bool pack_contactor_bit(ContactorPosition p) {
  return p == ContactorPosition::closed;
}

inline constexpr ContactorRequest pack_contactor_request(ContactorPosition p) {
  return {.close = pack_contactor_bit(p), ._reserved_ = 0};
}

inline constexpr BranchRequest pack_branch_request(bool b) {
  return {.connect = b, ._reserved_ = 0};
}

} // namespace pdu
