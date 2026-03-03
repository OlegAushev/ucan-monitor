#pragma once

#include <array>
#include <cstdint>
#include <string_view>

namespace sevpress {

namespace sys::diag {

inline constexpr size_t level_count = 4;

enum class level : uint8_t { notice, warning, error, critical };

} // namespace sys::diag

namespace sys::status {

using id_type = uint8_t;

struct i18n {
  std::string_view en;
  std::string_view ru;
};

struct emergency {
  static constexpr id_type id = 0;
  static constexpr sys::diag::level level_min = sys::diag::level::critical;
  static constexpr sys::diag::level level_max = sys::diag::level::critical;
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
  static constexpr sys::diag::level level_min = sys::diag::level::notice;
  static constexpr sys::diag::level level_max = sys::diag::level::error;
  static constexpr i18n description = {"CAN bus error", "Ошибка шины CAN"};
};

struct can_overrun {
  static constexpr id_type id = 8;
  static constexpr sys::diag::level level_min = sys::diag::level::notice;
  static constexpr sys::diag::level level_max = sys::diag::level::warning;
  static constexpr i18n description = {"CAN overrun", "Переполнение CAN"};
};

struct can_vcu_connection_lost {
  static constexpr id_type id = 9;
  static constexpr sys::diag::level level_min = sys::diag::level::warning;
  static constexpr sys::diag::level level_max = sys::diag::level::critical;
  static constexpr i18n description = {
      "VCU connection lost",
      "Потеря связи с VCU"
  };
};

struct can_vcu_checksum_mismatch {
  static constexpr id_type id = 10;
  static constexpr sys::diag::level level_min = sys::diag::level::warning;
  static constexpr sys::diag::level level_max = sys::diag::level::error;
  static constexpr i18n description = {
      "VCU checksum mismatch",
      "Несовпадение контрольной суммы VCU"
  };
};

struct can_vcu_counter_freeze_rpdo1 {
  static constexpr id_type id = 11;
  static constexpr sys::diag::level level_min = sys::diag::level::warning;
  static constexpr sys::diag::level level_max = sys::diag::level::error;
  static constexpr i18n description = {
      "VCU RPDO1 counter freeze",
      "Зависание счётчика RPDO1 VCU"
  };
};

struct can_vcu_invalid_rpdo1 {
  static constexpr id_type id = 12;
  static constexpr sys::diag::level level_min = sys::diag::level::warning;
  static constexpr sys::diag::level level_max = sys::diag::level::error;
  static constexpr i18n description = {
      "VCU invalid RPDO1",
      "Некорректный RPDO1 VCU"
  };
};

struct vdc_sensor_failure {
  static constexpr id_type id = 13;
  static constexpr sys::diag::level level_min = sys::diag::level::critical;
  static constexpr sys::diag::level level_max = sys::diag::level::critical;
  static constexpr i18n description = {
      "DC voltage sensor failure",
      "Отказ датчика напряжения DC"
  };
};

struct iph_sensor_failure {
  static constexpr id_type id = 14;
  static constexpr sys::diag::level level_min = sys::diag::level::critical;
  static constexpr sys::diag::level level_max = sys::diag::level::critical;
  static constexpr i18n description = {
      "Phase current sensor failure",
      "Отказ датчика фазного тока"
  };
};

struct ot_motor {
  static constexpr id_type id = 15;
  static constexpr sys::diag::level level_min = sys::diag::level::warning;
  static constexpr sys::diag::level level_max = sys::diag::level::critical;
  static constexpr i18n description = {
      "Motor overtemperature",
      "Перегрев двигателя"
  };
};

struct ot_pwr {
  static constexpr id_type id = 16;
  static constexpr sys::diag::level level_min = sys::diag::level::warning;
  static constexpr sys::diag::level level_max = sys::diag::level::critical;
  static constexpr i18n description = {
      "Power stage overtemperature",
      "Перегрев силовой части"
  };
};

struct ot_mcu {
  static constexpr id_type id = 17;
  static constexpr sys::diag::level level_min = sys::diag::level::warning;
  static constexpr sys::diag::level level_max = sys::diag::level::critical;
  static constexpr i18n description = {"MCU overtemperature", "Перегрев MCU"};
};

struct uv_dc {
  static constexpr id_type id = 18;
  static constexpr sys::diag::level level_min = sys::diag::level::warning;
  static constexpr sys::diag::level level_max = sys::diag::level::critical;
  static constexpr i18n description = {
      "DC undervoltage",
      "Пониженное напряжение DC"
  };
};

struct ov_dc {
  static constexpr id_type id = 19;
  static constexpr sys::diag::level level_min = sys::diag::level::warning;
  static constexpr sys::diag::level level_max = sys::diag::level::critical;
  static constexpr i18n description = {
      "DC overvoltage",
      "Повышенное напряжение DC"
  };
};

struct oc_dc {
  static constexpr id_type id = 20;
  static constexpr sys::diag::level level_min = sys::diag::level::warning;
  static constexpr sys::diag::level level_max = sys::diag::level::critical;
  static constexpr i18n description = {"DC overcurrent", "Превышение тока DC"};
};

struct oc_phase {
  static constexpr id_type id = 21;
  static constexpr sys::diag::level level_min = sys::diag::level::warning;
  static constexpr sys::diag::level level_max = sys::diag::level::critical;
  static constexpr i18n description = {
      "Phase overcurrent",
      "Превышение фазного тока"
  };
};

struct oc_phase_a {
  static constexpr id_type id = 22;
  static constexpr sys::diag::level level_min = sys::diag::level::warning;
  static constexpr sys::diag::level level_max = sys::diag::level::critical;
  static constexpr i18n description = {
      "Phase A overcurrent",
      "Превышение тока фазы A"
  };
};

struct oc_phase_b {
  static constexpr id_type id = 23;
  static constexpr sys::diag::level level_min = sys::diag::level::warning;
  static constexpr sys::diag::level level_max = sys::diag::level::critical;
  static constexpr i18n description = {
      "Phase B overcurrent",
      "Превышение тока фазы B"
  };
};

struct oc_phase_c {
  static constexpr id_type id = 24;
  static constexpr sys::diag::level level_min = sys::diag::level::warning;
  static constexpr sys::diag::level level_max = sys::diag::level::critical;
  static constexpr i18n description = {
      "Phase C overcurrent",
      "Превышение тока фазы C"
  };
};

struct flt_pwr {
  static constexpr id_type id = 25;
  static constexpr sys::diag::level level_min = sys::diag::level::critical;
  static constexpr sys::diag::level level_max = sys::diag::level::critical;
  static constexpr i18n description = {
      "Power module fault",
      "Неисправность силового модуля"
  };
};

struct flt_pwr_ah {
  static constexpr id_type id = 26;
  static constexpr sys::diag::level level_min = sys::diag::level::critical;
  static constexpr sys::diag::level level_max = sys::diag::level::critical;
  static constexpr i18n description = {
      "Power module fault: A high",
      "Неисправность силового модуля: A high"
  };
};

struct flt_pwr_al {
  static constexpr id_type id = 27;
  static constexpr sys::diag::level level_min = sys::diag::level::critical;
  static constexpr sys::diag::level level_max = sys::diag::level::critical;
  static constexpr i18n description = {
      "Power module fault: A low",
      "Неисправность силового модуля: A low"
  };
};

struct flt_pwr_bh {
  static constexpr id_type id = 28;
  static constexpr sys::diag::level level_min = sys::diag::level::critical;
  static constexpr sys::diag::level level_max = sys::diag::level::critical;
  static constexpr i18n description = {
      "Power module fault: B high",
      "Неисправность силового модуля: B high"
  };
};

struct flt_pwr_bl {
  static constexpr id_type id = 29;
  static constexpr sys::diag::level level_min = sys::diag::level::critical;
  static constexpr sys::diag::level level_max = sys::diag::level::critical;
  static constexpr i18n description = {
      "Power module fault: B low",
      "Неисправность силового модуля: B low"
  };
};

struct flt_pwr_ch {
  static constexpr id_type id = 30;
  static constexpr sys::diag::level level_min = sys::diag::level::critical;
  static constexpr sys::diag::level level_max = sys::diag::level::critical;
  static constexpr i18n description = {
      "Power module fault: C high",
      "Неисправность силового модуля: C high"
  };
};

struct flt_pwr_cl {
  static constexpr id_type id = 31;
  static constexpr sys::diag::level level_min = sys::diag::level::critical;
  static constexpr sys::diag::level level_max = sys::diag::level::critical;
  static constexpr i18n description = {
      "Power module fault: C low",
      "Неисправность силового модуля: C low"
  };
};

struct hall_invalid_input {
  static constexpr id_type id = 32;
  static constexpr sys::diag::level level_min = sys::diag::level::warning;
  static constexpr sys::diag::level level_max = sys::diag::level::critical;
  static constexpr i18n description = {
      "Hall invalid input state",
      "Некорректное состояние датчиков Холла"
  };
};

struct hall_invalid_calibration {
  static constexpr id_type id = 33;
  static constexpr sys::diag::level level_min = sys::diag::level::critical;
  static constexpr sys::diag::level level_max = sys::diag::level::critical;
  static constexpr i18n description = {
      "Hall invalid calibration",
      "Некорректная калибровка датчиков Холла"
  };
};

struct hall_calibration_terminated {
  static constexpr id_type id = 34;
  static constexpr sys::diag::level level_min = sys::diag::level::critical;
  static constexpr sys::diag::level level_max = sys::diag::level::critical;
  static constexpr i18n description = {
      "Hall calibration terminated",
      "Калибровка датчиков Холла прервана"
  };
};
inline constexpr size_t status_count = 35;

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
    can_vcu_connection_lost::description.ru,
    can_vcu_checksum_mismatch::description.ru,
    can_vcu_counter_freeze_rpdo1::description.ru,
    can_vcu_invalid_rpdo1::description.ru,
    vdc_sensor_failure::description.ru,
    iph_sensor_failure::description.ru,
    ot_motor::description.ru,
    ot_pwr::description.ru,
    ot_mcu::description.ru,
    uv_dc::description.ru,
    ov_dc::description.ru,
    oc_dc::description.ru,
    oc_phase::description.ru,
    oc_phase_a::description.ru,
    oc_phase_b::description.ru,
    oc_phase_c::description.ru,
    flt_pwr::description.ru,
    flt_pwr_ah::description.ru,
    flt_pwr_al::description.ru,
    flt_pwr_bh::description.ru,
    flt_pwr_bl::description.ru,
    flt_pwr_ch::description.ru,
    flt_pwr_cl::description.ru,
    hall_invalid_input::description.ru,
    hall_invalid_calibration::description.ru,
    hall_calibration_terminated::description.ru,
};

} // namespace sys::status

} // namespace sevpress
