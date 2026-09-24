#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

// Mirrors src/common/trouble/common_status.hpp (общая база) and
// src/common/contract/chss/faults.hpp (специфичные для СХКВ) of the h2-hess
// firmware. Bit position == index in status_list, so a new status is appended,
// never inserted. Keep both in sync.

namespace chss {

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
  static constexpr trouble::level level_max = trouble::level::warning;
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
// Неисправности, специфичные для СХКВ.
// ---------------------------------------------------------------------------

struct sensor_failure_receiver_pressure {
  static constexpr id_type id = 11;
  static constexpr trouble::level level_min = trouble::level::emergency;
  static constexpr trouble::level level_max = trouble::level::emergency;
  static constexpr i18n description = {
      "CHSS-P1-1 sensor failure",
      "отказ датчика давления СХКВ-P1-1"
  };
};

struct sensor_failure_fill_line_pressure {
  static constexpr id_type id = 12;
  static constexpr trouble::level level_min = trouble::level::emergency;
  static constexpr trouble::level level_max = trouble::level::emergency;
  static constexpr i18n description = {
      "CHSS-P1 sensor failure",
      "отказ датчика давления СХКВ-P1"
  };
};

struct sensor_failure_pressure_before_reducer {
  static constexpr id_type id = 13;
  static constexpr trouble::level level_min = trouble::level::emergency;
  static constexpr trouble::level level_max = trouble::level::emergency;
  static constexpr i18n description = {
      "CHSS-P2 sensor failure",
      "отказ датчика давления СХКВ-P2"
  };
};

struct sensor_failure_pressure_after_reducer {
  static constexpr id_type id = 14;
  static constexpr trouble::level level_min = trouble::level::emergency;
  static constexpr trouble::level level_max = trouble::level::emergency;
  static constexpr i18n description = {
      "CHSS-P3 sensor failure",
      "отказ датчика давления СХКВ-P3"
  };
};

struct sensor_failure_inflow_rate {
  static constexpr id_type id = 15;
  static constexpr trouble::level level_min = trouble::level::emergency;
  static constexpr trouble::level level_max = trouble::level::emergency;
  static constexpr i18n description = {
      "CHSS-FT1 sensor failure",
      "отказ расходомера СХКВ-ИР1"
  };
};

struct pressure_before_reducer_high {
  static constexpr id_type id = 16;
  static constexpr trouble::level level_min = trouble::level::critical;
  static constexpr trouble::level level_max = trouble::level::critical;
  static constexpr i18n description = {
      "CHSS-P2 pressure high",
      "давление СХКВ-Р2 выше нормы"
  };
};

struct pressure_after_reducer_high {
  static constexpr id_type id = 17;
  static constexpr trouble::level level_min = trouble::level::critical;
  static constexpr trouble::level level_max = trouble::level::critical;
  static constexpr i18n description = {
      "CHSS-P3 pressure high",
      "давление СХКВ-Р3 выше нормы"
  };
};

struct storage_leak {
  static constexpr id_type id = 18;
  static constexpr trouble::level level_min = trouble::level::critical;
  static constexpr trouble::level level_max = trouble::level::emergency;
  static constexpr i18n description = {"storage leak", "утечка при хранении"};
};

struct supply_filter_fouled {
  static constexpr id_type id = 19;
  static constexpr trouble::level level_min = trouble::level::warning;
  static constexpr trouble::level level_max = trouble::level::warning;
  static constexpr i18n description = {
      "Ф2 filter fouled",
      "загрязнение фильтра Ф2"
  };
};

struct fill_start_pressure_high {
  static constexpr id_type id = 20;
  static constexpr trouble::level level_min = trouble::level::warning;
  static constexpr trouble::level level_max = trouble::level::warning;
  static constexpr i18n description = {
      "fill start pressure high",
      "давление СХКВ-Р1-1 выше разрешающего заправку"
  };
};

struct pressure_before_reducer_low {
  static constexpr id_type id = 21;
  static constexpr trouble::level level_min = trouble::level::error;
  static constexpr trouble::level level_max = trouble::level::error;
  static constexpr i18n description = {
      "CHSS-P2 pressure low",
      "давление СХКВ-Р2 ниже нормы"
  };
};

struct pressure_after_reducer_low {
  static constexpr id_type id = 22;
  static constexpr trouble::level level_min = trouble::level::error;
  static constexpr trouble::level level_max = trouble::level::error;
  static constexpr i18n description = {
      "CHSS-P3 pressure low",
      "давление СХКВ-Р3 ниже нормы"
  };
};

struct sensor_bypassed {
  static constexpr id_type id = 23;
  static constexpr trouble::level level_min = trouble::level::warning;
  static constexpr trouble::level level_max = trouble::level::warning;
  static constexpr i18n description = {
      "sensor bypassed",
      "датчик выведен из работы"
  };
};

struct can_substitute_connection_lost {
  static constexpr id_type id = 24;
  static constexpr trouble::level level_min = trouble::level::warning;
  static constexpr trouble::level level_max = trouble::level::critical;
  static constexpr i18n description = {
      "substitute stream lost",
      "потеря потока подстановок"
  };
};

struct can_substitute_invalid_frame {
  static constexpr id_type id = 25;
  static constexpr trouble::level level_min = trouble::level::warning;
  static constexpr trouble::level level_max = trouble::level::warning;
  static constexpr i18n description = {
      "substitute invalid frame",
      "некорректный кадр подстановок"
  };
};

inline constexpr size_t status_count = 26;

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
    sensor_failure_receiver_pressure::description.ru,
    sensor_failure_fill_line_pressure::description.ru,
    sensor_failure_pressure_before_reducer::description.ru,
    sensor_failure_pressure_after_reducer::description.ru,
    sensor_failure_inflow_rate::description.ru,
    pressure_before_reducer_high::description.ru,
    pressure_after_reducer_high::description.ru,
    storage_leak::description.ru,
    supply_filter_fouled::description.ru,
    fill_start_pressure_high::description.ru,
    pressure_before_reducer_low::description.ru,
    pressure_after_reducer_low::description.ru,
    sensor_bypassed::description.ru,
    can_substitute_connection_lost::description.ru,
    can_substitute_invalid_frame::description.ru
};

} // namespace status

} // namespace chss
