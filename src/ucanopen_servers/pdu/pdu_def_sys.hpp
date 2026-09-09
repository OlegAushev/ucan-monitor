#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

// Mirrors src/common/trouble/common_status.hpp (общая база) and
// src/common/contract/pdu/faults.hpp (специфичные для PDU) of the h2-hess
// firmware. Bit position == index in status_list, so a new status is appended,
// never inserted. Keep both in sync.

namespace pdu {

namespace trouble {

inline constexpr size_t level_count = 4;

enum class level : uint8_t { warning, error, critical, emergency };

} // namespace trouble

namespace status {

using id_type = uint8_t;

struct i18n {
  std::string_view en;
  std::string_view ru;
};

// ---------------------------------------------------------------------------
// Общая база: неизменный префикс status_list любого устройства h2-hess.
// ---------------------------------------------------------------------------

struct emergency {
  static constexpr id_type id = 0;
  static constexpr trouble::level level_min = trouble::level::emergency;
  static constexpr trouble::level level_max = trouble::level::emergency;
  static constexpr i18n description = {"emergency", "авария"};
};

struct watchdog_timeout {
  static constexpr id_type id = 1;
  static constexpr trouble::level level_min = trouble::level::critical;
  static constexpr trouble::level level_max = trouble::level::critical;
  static constexpr i18n description = {"watchdog timeout", "таймаут watchdog"};
};

struct invalid_config {
  static constexpr id_type id = 2;
  static constexpr trouble::level level_min = trouble::level::critical;
  static constexpr trouble::level level_max = trouble::level::critical;
  static constexpr i18n description = {
      "invalid configuration",
      "некорректная конфигурация"
  };
};

struct no_nvm {
  static constexpr id_type id = 3;
  static constexpr trouble::level level_min = trouble::level::critical;
  static constexpr trouble::level level_max = trouble::level::critical;
  static constexpr i18n description = {
      "no non-volatile memory",
      "нет энергонезависимой памяти"
  };
};

struct nvm_read_error {
  static constexpr id_type id = 4;
  static constexpr trouble::level level_min = trouble::level::critical;
  static constexpr trouble::level level_max = trouble::level::critical;
  static constexpr i18n description = {"NVM read error", "ошибка чтения NVM"};
};

struct nvm_write_error {
  static constexpr id_type id = 5;
  static constexpr trouble::level level_min = trouble::level::critical;
  static constexpr trouble::level level_max = trouble::level::critical;
  static constexpr i18n description = {"NVM write error", "ошибка записи NVM"};
};

struct can_bus_error {
  static constexpr id_type id = 6;
  static constexpr trouble::level level_min = trouble::level::warning;
  static constexpr trouble::level level_max = trouble::level::error;
  static constexpr i18n description = {"CAN bus error", "ошибка шины CAN"};
};

struct can_overrun {
  static constexpr id_type id = 7;
  static constexpr trouble::level level_min = trouble::level::warning;
  static constexpr trouble::level level_max = trouble::level::warning;
  static constexpr i18n description = {"CAN overrun", "переполнение CAN"};
};

struct can_master_connection_lost {
  static constexpr id_type id = 8;
  static constexpr trouble::level level_min = trouble::level::warning;
  static constexpr trouble::level level_max = trouble::level::critical;
  static constexpr i18n description = {
      "master connection lost",
      "потеря связи с КВУ"
  };
};

struct can_master_invalid_frame {
  static constexpr id_type id = 9;
  static constexpr trouble::level level_min = trouble::level::warning;
  static constexpr trouble::level level_max = trouble::level::error;
  static constexpr i18n description = {
      "master invalid frame",
      "некорректный кадр КВУ"
  };
};

struct ot_mcu {
  static constexpr id_type id = 10;
  static constexpr trouble::level level_min = trouble::level::warning;
  static constexpr trouble::level level_max = trouble::level::critical;
  static constexpr i18n description = {"MCU overtemperature", "перегрев МК"};
};

// ---------------------------------------------------------------------------
// Неисправности, специфичные для PDU.
// ---------------------------------------------------------------------------

struct contactor_weld {
  static constexpr id_type id = 11;
  static constexpr trouble::level level_min = trouble::level::critical;
  static constexpr trouble::level level_max = trouble::level::emergency;
  static constexpr i18n description = {
      "contactor welded",
      "залипание контактора"
  };
};

struct contactor_no_feedback {
  static constexpr id_type id = 12;
  static constexpr trouble::level level_min = trouble::level::error;
  static constexpr trouble::level level_max = trouble::level::critical;
  static constexpr i18n description = {
      "contactor feedback mismatch",
      "несоответствие обратной связи контактора"
  };
};

struct sensor_failure_current {
  static constexpr id_type id = 13;
  static constexpr trouble::level level_min = trouble::level::error;
  static constexpr trouble::level level_max = trouble::level::critical;
  static constexpr i18n description = {
      "current sensor failure",
      "отказ датчика тока"
  };
};

struct sensor_failure_voltage {
  static constexpr id_type id = 14;
  static constexpr trouble::level level_min = trouble::level::error;
  static constexpr trouble::level level_max = trouble::level::critical;
  static constexpr i18n description = {
      "voltage sensor failure",
      "отказ датчика напряжения"
  };
};

struct precharge_timeout {
  static constexpr id_type id = 15;
  static constexpr trouble::level level_min = trouble::level::error;
  static constexpr trouble::level level_max = trouble::level::critical;
  static constexpr i18n description = {
      "precharge timeout",
      "таймаут предзаряда"
  };
};

inline constexpr size_t status_count = 16;

inline constexpr std::array<std::string_view, status_count> names_ru = {
    emergency::description.ru,
    watchdog_timeout::description.ru,
    invalid_config::description.ru,
    no_nvm::description.ru,
    nvm_read_error::description.ru,
    nvm_write_error::description.ru,
    can_bus_error::description.ru,
    can_overrun::description.ru,
    can_master_connection_lost::description.ru,
    can_master_invalid_frame::description.ru,
    ot_mcu::description.ru,
    contactor_weld::description.ru,
    contactor_no_feedback::description.ru,
    sensor_failure_current::description.ru,
    sensor_failure_voltage::description.ru,
    precharge_timeout::description.ru
};

} // namespace status

} // namespace pdu
