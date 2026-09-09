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

struct CobTpdo1 {
  uint64_t flags : status::status_count;

  uint64_t _reserved1_ : 64 - status::status_count - 8 - 8;

  uint64_t level : 2;
  uint64_t _reserved2_ : 6;

  uint64_t counter : 2;
  uint64_t _reserved3_ : 6;
};

struct CobTpdo2 {
  int16_t fuelcell_voltage;   // 0.1 V/LSB

  int16_t fuelcell_current;   // 0.1 A/LSB

  uint8_t fuelcell_main;      // bit0 command, bit1 feedback (position)

  uint8_t fuelcell_precharge; // bit0 command, bit1 feedback

  uint8_t fuelcell_state;

  uint8_t counter : 2;
  uint8_t _reserved1_ : 6;
};

struct CobTpdo3 {
  int16_t inverter_voltage;   // 0.1 V/LSB

  int16_t inverter_current;   // 0.1 A/LSB

  uint8_t inverter_main;      // bit0 command, bit1 feedback (position)

  uint8_t inverter_precharge; // bit0 command, bit1 feedback

  uint8_t inverter_state;

  uint8_t counter : 2;
  uint8_t _reserved1_ : 6;
};

struct CobTpdo4 {
  int16_t battery_voltage; // 0.1 V/LSB

  int16_t mcu_temperature; // 0.1 degC/LSB

  uint8_t _reserved1_;

  uint8_t _reserved2_;

  uint8_t _reserved3_;

  uint8_t counter : 2;
  uint8_t _reserved4_ : 6;
};

static_assert(sizeof(CobTpdo1) == 8);
static_assert(sizeof(CobTpdo2) == 8);
static_assert(sizeof(CobTpdo3) == 8);
static_assert(sizeof(CobTpdo4) == 8);

// Режим установки, объявленный мастером, и запрос на подключение ветви
// инвертора. Ручные уровни контакторов действуют, только пока установка в
// продувке.
struct CobRpdo1 {
  uint8_t mode;               // Byte 0: режим установки (Mode)

  uint8_t inverter_request;   // Byte 1: bit0 = подключить ветвь инвертора

  uint8_t fuelcell_main;      // Byte 2: bit0 = замкнуть; только продувка
  uint8_t fuelcell_precharge; // Byte 3: bit0 = замкнуть; только продувка
  uint8_t inverter_main;      // Byte 4: bit0 = замкнуть; только продувка
  uint8_t inverter_precharge; // Byte 5: bit0 = замкнуть; только продувка

  uint8_t _reserved1_;        // Byte 6

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

// Mirrors control::state::id (app/pdu/control/fsm.hpp).
enum class BranchState : uint8_t {
  disconnected, // оба контактора разомкнуты
  precharging,  // предзарядный замкнут: ёмкость ветви набирает напряжение
  switchover,   // главный замкнут, идёт выдержка перед снятием предзаряда
  connected,    // главный замкнут, предзарядный разомкнут
  lockout,      // попытка не удалась: ждём, пока запрос снимут
  manual,       // контакторы отданы уровням из кадра (продувка)
};

inline std::unordered_set<int> const branch_state_values = {
    std::to_underlying(BranchState::disconnected),
    std::to_underlying(BranchState::precharging),
    std::to_underlying(BranchState::switchover),
    std::to_underlying(BranchState::connected),
    std::to_underlying(BranchState::lockout),
    std::to_underlying(BranchState::manual)};

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

// Заданное и фактическое положения контактора упакованы в один байт:
// бит0 — команда «замкнут», бит1 — обратная связь «замкнут».
inline ContactorPosition unpack_contactor_command(uint8_t byte) {
  return (byte & 0b01) != 0 ? ContactorPosition::closed
                            : ContactorPosition::open;
}

inline ContactorPosition unpack_contactor_feedback(uint8_t byte) {
  return (byte & 0b10) != 0 ? ContactorPosition::closed
                            : ContactorPosition::open;
}

inline uint8_t pack_contactor_request(ContactorPosition position) {
  return position == ContactorPosition::closed ? uint8_t{0b01} : uint8_t{0};
}

} // namespace pdu
