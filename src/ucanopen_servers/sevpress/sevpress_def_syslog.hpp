#pragma once

#include <array>
#include <bit>
#include <cstdint>
#include <cstring>
#include <string_view>
#include <vector>

namespace sevpress {

class syslog {
private:
  static constexpr size_t domain_count{7};
public:
  static constexpr std::array<std::string_view, domain_count> domains{
      "Система",
      "Настройки",
      "CAN-сеть",
      "Датчики",
      "Двигатель",
      "Инвертор",
      "ДПР"};
  static constexpr std::array<std::string_view, 6> levels{
      "      ", "[info]", "[ ok ]", "[warn]", "[fail]", "[crit]"};
private:
  // general
  static inline std::vector<std::string_view> const sys_errors_{
      "аварийный останов",
      "прогр. сторожевой таймер",
      "недопустимые настройки",
      "нештатное отключение"};
  static inline std::vector<std::string_view> const sys_warnings_{};

  // settings
  static inline std::vector<std::string_view> const settings_errors_{
      "отсутствует внеш. память", "ошибка чтения", "ошибка записи"};
  static inline std::vector<std::string_view> const settings_warnings_{};

  // can
  static inline std::vector<std::string_view> const can_errors_{
      "КВУ: потеря связи"};
  static inline std::vector<std::string_view> const can_warnings_{
      "ошибки в сети",
      "потеря сообщений",
      "КВУ: потеря связи",
      "КВУ: ошибка контрольной суммы",
      "КВУ: ошибка циклического счётчика RPD01",
      "КВУ: ошибка циклического счётчика RPD02",
      "КВУ: неверный формат RPDO1",
      "КВУ: неверный формат RPDO2"};

  // sensors
  static inline std::vector<std::string_view> const sensors_errors_{
      "отказ датчика напряжения", "отказ датчика тока"};
  static inline std::vector<std::string_view> const sensors_warnings_{};

  // motor
  static inline std::vector<std::string_view> const motor_errors_{"перегрев"};
  static inline std::vector<std::string_view> const motor_warnings_{"перегрев"};

  // converter
  static inline std::vector<std::string_view> const converter_errors_{
      "низкое напряжение ЗПТ",
      "высокое напряжение ЗПТ",
      "сверхток ЗПТ",
      "сверхток ОЯ",
      "сверхток ОЯ-ф.A",
      "сверхток ОЯ-ф.B",
      "сверхток ОЯ-ф.C",
      "ошибка модуля ОЯ",
      "ошибка модуля ОЯ-AH",
      "ошибка модуля ОЯ-AL",
      "ошибка модуля ОЯ-BH",
      "ошибка модуля ОЯ-BL",
      "ошибка модуля ОЯ-CH",
      "ошибка модуля ОЯ-CL",
      "перегрев силового модуля",
      "перегрев МК"};
  static inline std::vector<std::string_view> const converter_warnings_{
      "перегрев силового модуля", "перегрев МК"};

  // hall
  static inline std::vector<std::string_view> const hall_errors_{
      "отказ"
  };
  static inline std::vector<std::string_view> const hall_warnings_{
      "отказ"
  };

public:
  static inline std::array<std::vector<std::string_view>, domains.size()> const
      errors = {
          sys_errors_,
          settings_errors_,
          can_errors_,
          sensors_errors_,
          motor_errors_,
          converter_errors_};
  static inline std::array<std::vector<std::string_view>, domains.size()> const
      warnings = {
          sys_warnings_,
          settings_warnings_,
          can_warnings_,
          sensors_warnings_,
          motor_warnings_,
          converter_warnings_};
private:
  static inline std::vector<std::string_view> const sys_messages_ = {
      "",
      "Boot...",
      "Device is ready.",
      "Cannot perform operation - device is busy.",
      "Device is in error state.",
      "Device is being rebooted..."};
  static inline std::vector<std::string_view> const settings_messages_ = {
      "No non-volatile memory detected.",
      "Settings read timeout.",
      "Settings read error occurred",
      "Read all configs - ok.",
      "Apply all configs - ok.",
      "Apply all configs - fail.",
      "Restore all configs -ok.",
      "Restore all configs - fail.",
      "Load all default configs.",

      "Read ucanopen server config - ok.",
      "Read ucanopen server config - fail.",
      "Load ucanopen server default config.",
      "Write ucanopen server config - ok.",
      "Write ucanopen server config - fail.",

      "Read syslog config - ok.",
      "Read syslog config - fail.",
      "Load syslog default config.",
      "Write syslog config - ok.",
      "Write syslog config - fail.",

      "Read drive config - ok.",
      "Read drive config - fail.",
      "Load drive default config.",
      "Write drive config - ok.",
      "Write drive config - fail.",

      "Read motor config - ok.",
      "Read motor config - fail.",
      "Load motor default config.",
      "Write motor config - ok.",
      "Write motor config - fail.",

      "Read converter config - ok.",
      "Read converter config - fail.",
      "Load converter default config.",
      "Write converter config - ok.",
      "Write converter config - fail.",

      "Read model basic config - ok.",
      "Read model basic config - fail.",
      "Load model basic default config.",
      "Write model basic config - ok.",
      "Write model basic config - fail.",

      "Read model controllers config - ok.",
      "Read model controllers config - fail.",
      "Load model controllers default config.",
      "Write model controllers config - ok.",
      "Write model controllers config - fail.",

      "Read mras config - ok.",
      "Read mras config - fail.",
      "Load mras default config.",
      "Write mras config - ok.",
      "Write mras config - fail.",

      "Read phase current sensor config - ok.",
      "Read phase current sensor config - fail.",
      "Load phase current sensor default config.",
      "Write phase current sensor config - ok.",
      "Write phase current sensor config - fail.",

      "Read dc voltage sensor config - ok.",
      "Read dc voltage sensor config - fail.",
      "Load dc voltage sensor default config.",
      "Write dc voltage sensor config - ok.",
      "Write dc voltage sensor config - fail.",

      "Read position sensor config - ok.",
      "Read position sensor config - fail.",
      "Load position sensor default config.",
      "Write position sensor config - ok.",
      "Write position sensor config - fail.",

      "Read position sensor emulator config - ok.",
      "Read position sensor emulator config - fail.",
      "Load position sensor emulator default config.",
      "Write position sensor emulator config - ok.",
      "Write position sensor emulator config - fail.",
  };
  static inline std::vector<std::string_view> const can_messages_ = {
      "CAN SDO-request is lost."};
  static inline std::vector<std::string_view> const sensors_messages_ = {};
  static inline std::vector<std::string_view> const motor_messages_ = {};
  static inline std::vector<std::string_view> const converter_messages_ = {};
  static inline std::vector<std::string_view> const model_messages_{};
  static inline std::vector<std::string_view> const resolver_messages_{};
  static inline std::vector<std::string_view> const pdu_messages_{};
  static inline std::vector<std::string_view> const isolation_messages_{};
public:
  static inline std::array<std::vector<std::string_view>, domains.size()> const
      messages = {
          sys_messages_,
          settings_messages_,
          can_messages_,
          sensors_messages_,
          motor_messages_,
          converter_messages_};
private:
  syslog();
public:
  static syslog& instance() {
    static syslog s;
    return s;
  }
};

class SyslogMessage {
private:
  uint8_t domain_;
  uint8_t level_;
  uint16_t id_;
public:
  SyslogMessage(uint32_t data) { *this = std::bit_cast<SyslogMessage>(data); }

  bool valid() const {
    if (domain_ == 0 && id_ == 0) {
      return false;
    }
    if (domain_ >= syslog::domains.size() || level_ >= syslog::levels.size()) {
      return false;
    }
    if (id_ >= syslog::messages[domain_].size()) {
      return false;
    }
    return true;
  }

  std::string_view domain() const { return syslog::domains[domain_]; }

  std::string_view level() const { return syslog::levels[level_]; }

  std::string_view message() const { return syslog::messages[domain_][id_]; }
};

} // namespace sevpress
