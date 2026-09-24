#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

// Mirrors src/common/trouble/common_status.hpp (общая база) and
// src/common/contract/cshpp/faults.hpp (специфичные для БКСГЭУ) of the h2-hess
// firmware. Bit position == index in status_list, so a new status is appended,
// never inserted. Keep both in sync.

namespace cshpp {

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
// Неисправности, специфичные для БКСГЭУ.
// ---------------------------------------------------------------------------

struct can_chss_connection_lost {
  static constexpr id_type id = 11;
  static constexpr trouble::level level_min = trouble::level::warning;
  static constexpr trouble::level level_max = trouble::level::critical;
  static constexpr i18n description = {
      "CHSS connection lost",
      "потеря связи с СХКВ"
  };
};

struct can_chss_invalid_frame {
  static constexpr id_type id = 12;
  static constexpr trouble::level level_min = trouble::level::warning;
  static constexpr trouble::level level_max = trouble::level::warning;
  static constexpr i18n description = {
      "CHSS invalid frame",
      "некорректный кадр СХКВ"
  };
};

struct can_pdu_connection_lost {
  static constexpr id_type id = 13;
  static constexpr trouble::level level_min = trouble::level::warning;
  static constexpr trouble::level level_max = trouble::level::critical;
  static constexpr i18n description = {
      "PDU connection lost",
      "потеря связи с PDU"
  };
};

struct can_pdu_invalid_frame {
  static constexpr id_type id = 14;
  static constexpr trouble::level level_min = trouble::level::warning;
  static constexpr trouble::level level_max = trouble::level::warning;
  static constexpr i18n description = {
      "PDU invalid frame",
      "некорректный кадр PDU"
  };
};

struct can_gpmu_connection_lost {
  static constexpr id_type id = 15;
  static constexpr trouble::level level_min = trouble::level::warning;
  static constexpr trouble::level level_max = trouble::level::critical;
  static constexpr i18n description = {
      "GPMU connection lost",
      "потеря связи с БВВИМ"
  };
};

struct can_gpmu_invalid_frame {
  static constexpr id_type id = 16;
  static constexpr trouble::level level_min = trouble::level::warning;
  static constexpr trouble::level level_max = trouble::level::warning;
  static constexpr i18n description = {
      "GPMU invalid frame",
      "некорректный кадр БВВИМ"
  };
};

struct gen_start_pressure_low {
  static constexpr id_type id = 17;
  static constexpr trouble::level level_min = trouble::level::error;
  static constexpr trouble::level level_max = trouble::level::error;
  static constexpr i18n description = {
      "generation start pressure low",
      "низкое давление Р1-1 для запуска генерации"
  };
};

struct fill_start_pressure_high {
  static constexpr id_type id = 18;
  static constexpr trouble::level level_min = trouble::level::error;
  static constexpr trouble::level level_max = trouble::level::error;
  static constexpr i18n description = {
      "fill start pressure high",
      "давление Р1.1 выше разрешающего заправку"
  };
};

struct fill_receiver_full {
  static constexpr id_type id = 19;
  static constexpr trouble::level level_min = trouble::level::warning;
  static constexpr trouble::level level_max = trouble::level::warning;
  static constexpr i18n description = {"receiver full", "ресивер заполнен"};
};

struct storage_supply_timeout {
  static constexpr id_type id = 20;
  static constexpr trouble::level level_min = trouble::level::error;
  static constexpr trouble::level level_max = trouble::level::error;
  static constexpr i18n description = {
      "storage supply timeout",
      "СХКВ не вышла на режим выдачи"
  };
};

struct hydrogen_leak {
  static constexpr id_type id = 21;
  static constexpr trouble::level level_min = trouble::level::critical;
  static constexpr trouble::level level_max = trouble::level::emergency;
  static constexpr i18n description = {"hydrogen leak", "утечка водорода"};
};

struct hydrogen_pressure_mismatch {
  static constexpr id_type id = 22;
  static constexpr trouble::level level_min = trouble::level::critical;
  static constexpr trouble::level level_max = trouble::level::critical;
  static constexpr i18n description = {
      "Р1-1/Р3 pressure mismatch",
      "расхождение давлений Р1-1 и Р3"
  };
};

struct mode_command_rejected {
  static constexpr id_type id = 23;
  static constexpr trouble::level level_min = trouble::level::warning;
  static constexpr trouble::level level_max = trouble::level::warning;
  static constexpr i18n description = {
      "mode command rejected",
      "команда режима отклонена"
  };
};

struct fuelcell_stop_timeout {
  static constexpr id_type id = 24;
  static constexpr trouble::level level_min = trouble::level::error;
  static constexpr trouble::level level_max = trouble::level::error;
  static constexpr i18n description = {
      "fuel cell stop timeout",
      "ЭХГ не подтвердил остановку"
  };
};

struct storage_supply_lost {
  static constexpr id_type id = 25;
  static constexpr trouble::level level_min = trouble::level::error;
  static constexpr trouble::level level_max = trouble::level::error;
  static constexpr i18n description = {
      "storage supply lost",
      "СХКВ сняла готовность к выдаче"
  };
};

struct sensor_bypassed {
  static constexpr id_type id = 26;
  static constexpr trouble::level level_min = trouble::level::warning;
  static constexpr trouble::level level_max = trouble::level::warning;
  static constexpr i18n description = {
      "sensor bypassed",
      "датчик выведен из работы"
  };
};

struct can_substitute_connection_lost {
  static constexpr id_type id = 27;
  static constexpr trouble::level level_min = trouble::level::warning;
  static constexpr trouble::level level_max = trouble::level::critical;
  static constexpr i18n description = {
      "substitute stream lost",
      "потеря потока подстановок"
  };
};

struct can_substitute_invalid_frame {
  static constexpr id_type id = 28;
  static constexpr trouble::level level_min = trouble::level::warning;
  static constexpr trouble::level level_max = trouble::level::warning;
  static constexpr i18n description = {
      "substitute invalid frame",
      "некорректный кадр подстановок"
  };
};

inline constexpr size_t status_count = 29;

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
    can_chss_connection_lost::description.ru,
    can_chss_invalid_frame::description.ru,
    can_pdu_connection_lost::description.ru,
    can_pdu_invalid_frame::description.ru,
    can_gpmu_connection_lost::description.ru,
    can_gpmu_invalid_frame::description.ru,
    gen_start_pressure_low::description.ru,
    fill_start_pressure_high::description.ru,
    fill_receiver_full::description.ru,
    storage_supply_timeout::description.ru,
    hydrogen_leak::description.ru,
    hydrogen_pressure_mismatch::description.ru,
    mode_command_rejected::description.ru,
    fuelcell_stop_timeout::description.ru,
    storage_supply_lost::description.ru,
    sensor_bypassed::description.ru,
    can_substitute_connection_lost::description.ru,
    can_substitute_invalid_frame::description.ru
};

} // namespace status

} // namespace cshpp
