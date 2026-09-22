#pragma once

#include <array>
#include <cstdint>
#include <string_view>

namespace psfb {

namespace sys::diag {

inline constexpr size_t level_count = 4;

enum class level : uint8_t { warning, error, critical, emergency };

} // namespace sys::diag

namespace sys::status {

using id_type = uint8_t;

struct i18n {
  std::string_view en;
  std::string_view ru;
};

struct emergency {
  static constexpr id_type id = 0;
  static constexpr sys::diag::level level_min = sys::diag::level::emergency;
  static constexpr sys::diag::level level_max = sys::diag::level::emergency;
  static constexpr i18n description = {"Emergency", "Авария"};
};

struct watchdog_timeout {
  static constexpr id_type id = 1;
  static constexpr sys::diag::level level_min = sys::diag::level::critical;
  static constexpr sys::diag::level level_max = sys::diag::level::critical;
  static constexpr i18n description = {"Watchdog timeout", "Таймаут watchdog"};
};

struct invalid_config {
  static constexpr id_type id = 2;
  static constexpr sys::diag::level level_min = sys::diag::level::critical;
  static constexpr sys::diag::level level_max = sys::diag::level::critical;
  static constexpr i18n description = {
      "Invalid configuration",
      "Некорректная конфигурация"
  };
};

struct abnormal_shutdown {
  static constexpr id_type id = 3;
  static constexpr sys::diag::level level_min = sys::diag::level::warning;
  static constexpr sys::diag::level level_max = sys::diag::level::warning;
  static constexpr i18n description = {
      "Abnormal shutdown",
      "Аварийное выключение"
  };
};

struct no_nvm {
  static constexpr id_type id = 4;
  static constexpr sys::diag::level level_min = sys::diag::level::critical;
  static constexpr sys::diag::level level_max = sys::diag::level::critical;
  static constexpr i18n description = {
      "No non-volatile memory",
      "Нет энергонезависимой памяти"
  };
};

struct nvm_read_error {
  static constexpr id_type id = 5;
  static constexpr sys::diag::level level_min = sys::diag::level::critical;
  static constexpr sys::diag::level level_max = sys::diag::level::critical;
  static constexpr i18n description = {"NVM read error", "Ошибка чтения NVM"};
};

struct nvm_write_error {
  static constexpr id_type id = 6;
  static constexpr sys::diag::level level_min = sys::diag::level::critical;
  static constexpr sys::diag::level level_max = sys::diag::level::critical;
  static constexpr i18n description = {"NVM write error", "Ошибка записи NVM"};
};

struct can_bus_error {
  static constexpr id_type id = 7;
  static constexpr sys::diag::level level_min = sys::diag::level::warning;
  static constexpr sys::diag::level level_max = sys::diag::level::error;
  static constexpr i18n description = {"CAN bus error", "Ошибка шины CAN"};
};

struct can_overrun {
  static constexpr id_type id = 8;
  static constexpr sys::diag::level level_min = sys::diag::level::warning;
  static constexpr sys::diag::level level_max = sys::diag::level::warning;
  static constexpr i18n description = {"CAN overrun", "Переполнение CAN"};
};

struct bridge_break_ch1 {
  static constexpr id_type id = 9;
  static constexpr sys::diag::level level_min = sys::diag::level::emergency;
  static constexpr sys::diag::level level_max = sys::diag::level::emergency;
  static constexpr i18n description = {"Bridge 1 break", "Срыв моста 1"};
};

struct bridge_break_ch2 {
  static constexpr id_type id = 10;
  static constexpr sys::diag::level level_min = sys::diag::level::emergency;
  static constexpr sys::diag::level level_max = sys::diag::level::emergency;
  static constexpr i18n description = {"Bridge 2 break", "Срыв моста 2"};
};

struct gate_driver_fault_ch1 {
  static constexpr id_type id = 11;
  static constexpr sys::diag::level level_min = sys::diag::level::critical;
  static constexpr sys::diag::level level_max = sys::diag::level::critical;
  static constexpr i18n description = {
      "Gate driver 1 fault",
      "Неисправность драйвера 1"
  };
};

struct gate_driver_fault_ch2 {
  static constexpr id_type id = 12;
  static constexpr sys::diag::level level_min = sys::diag::level::critical;
  static constexpr sys::diag::level level_max = sys::diag::level::critical;
  static constexpr i18n description = {
      "Gate driver 2 fault",
      "Неисправность драйвера 2"
  };
};

// The firmware carries trouble::id_space bits in TPDO4 — one past the largest
// id. While the ids stay contiguous it equals the number of statuses below.
inline constexpr size_t status_count = 13;

inline constexpr std::array<std::string_view, status_count> names_ru = {
    emergency::description.ru,
    watchdog_timeout::description.ru,
    invalid_config::description.ru,
    abnormal_shutdown::description.ru,
    no_nvm::description.ru,
    nvm_read_error::description.ru,
    nvm_write_error::description.ru,
    can_bus_error::description.ru,
    can_overrun::description.ru,
    bridge_break_ch1::description.ru,
    bridge_break_ch2::description.ru,
    gate_driver_fault_ch1::description.ru,
    gate_driver_fault_ch2::description.ru
};

} // namespace sys::status

} // namespace psfb
