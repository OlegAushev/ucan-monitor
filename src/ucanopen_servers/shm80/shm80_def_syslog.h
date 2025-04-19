#pragma once

#include <array>
#include <bit>
#include <cstdint>
#include <cstring>
#include <string_view>
#include <vector>

namespace shm80 {

class syslog {
private:
    static constexpr size_t domain_count = 11;
public:
    static constexpr std::array<std::string_view, domain_count> domains = {
        "Система",
        "Настройки",
        "CAN-сеть",
        "Датчики",
        "Двигатель",
        "Инвертор",
        "Модель",
        "ДПР",
        "Питание",
        "Изоляция",
        "Контроллер газа"};
    static constexpr std::array<std::string_view, 6> levels =
            {"      ", "[info]", "[ ok ]", "[warn]", "[fail]", "[crit]"};
private:
    // general
    static inline const std::vector<std::string_view> sys_errors_ = {
        "аварийный останов",
        "прогр. сторожевой таймер",
        "недопустимые настройки"};
    static inline const std::vector<std::string_view> sys_warnings_ = {};

    // settings
    static inline const std::vector<std::string_view> settings_errors_ = {
        "отсутствует внеш. память",
        "ошибка чтения",
        "ошибка записи"};
    static inline const std::vector<std::string_view> settings_warnings_ = {};

    // can
    static inline const std::vector<std::string_view> can_errors_ = {
        "потеря связи с КВУ"};
    static inline const std::vector<std::string_view> can_warnings_ = {
        "ошибки в сети",
        "потеря сообщений",
        "потеря связи с КВУ",
        "ошибка контрольной суммы",
        "ошибка циклического счётчика"};

    // sensors
    static inline const std::vector<std::string_view> sensors_errors_ = {
        "отказ датчика напряжения",
        "отказ датчика тока ОЯ",
        "отказ датчика тока ОВ"};
    static inline const std::vector<std::string_view> sensors_warnings_ = {};

    // motor
    static inline const std::vector<std::string_view> motor_errors_ = {
        "перегрев"
        "перегрев ОЯ",
        "перегрев ОВ"};
    static inline const std::vector<std::string_view> motor_warnings_ = {
        "перегрев",
        "перегрев ОЯ",
        "перегрев ОВ"};

    // converter
    static inline const std::vector<std::string_view> converter_errors_ = {
        "низкое напряжение ЗПТ",
        "высокое напряжение ЗПТ",
        "сверхток ОЯ",
        "сверхток ОЯ-ф.A",
        "сверхток ОЯ-ф.B",
        "сверхток ОЯ-ф.C",
        "сверхток ОВ",
        "ошибка модуля ОЯ",
        "ошибка модуля ОЯ-ф.A",
        "ошибка модуля ОЯ-ф.B",
        "ошибка модуля ОЯ-ф.C",
        "ошибка модуля ОВ",
        "перегрев модуля ОЯ",
        "перегрев модуля ОЯ-ф.A",
        "перегрев модуля ОЯ-ф.B",
        "перегрев модуля ОЯ-ф.C",
        "перегрев модуля ОВ",
        "перегрев ПП"};
    static inline const std::vector<std::string_view> converter_warnings_ = {
        "перегрев модуля ОЯ",
        "перегрев модуля ОЯ-ф.A",
        "перегрев модуля ОЯ-ф.B",
        "перегрев модуля ОЯ-ф.C",
        "перегрев модуля ОВ",
        "перегрев ПП"};

    // model
    static inline const std::vector<std::string_view> model_errors_{};
    static inline const std::vector<std::string_view> model_warnings_{};

    // resolver
    static inline const std::vector<std::string_view> resolver_errors_{
        "ошибка наблюдателя"};
    static inline const std::vector<std::string_view> resolver_warnings_{
        "калибровка, этап 1",
        "калибровка, этап 2",
        "калибровка, этап 3"};

    // pdu
    static inline const std::vector<std::string_view> pdu_errors_{
        "ошибка заряда ЗПТ",
        "превышено время заряда ЗПТ",
        "превышено время разряда ЗПТ",
        "некорректное состояние контакторов",
        "ошибка устройства",
        "потеря связи",
        "низкое напряжение АБ",
        "высокое напряжение АБ"};
    static inline const std::vector<std::string_view> pdu_warnings_{
        "ЗПТ отключено",
        "заряд ЗПТ",
        "заряд ЗПТ, выдержка",
        "разряд ЗПТ",
        "ошибка циклического счётчика"};

    // insulation
    static inline const std::vector<std::string_view> insulation_errors_{};
    static inline const std::vector<std::string_view> insulation_warnings_{
        "низкое значение",
        "ошибка измерения"};

    // throttle
    static inline const std::vector<std::string_view> throtlle_errors_{
        "потеря связи"};
    static inline const std::vector<std::string_view> throttle_warnings_{
        "ошибка старта",
        "неверное сообщение"};
public:
    static inline const std::array<std::vector<std::string_view>,
                                   domains.size()>
            errors = {sys_errors_,
                      settings_errors_,
                      can_errors_,
                      sensors_errors_,
                      motor_errors_,
                      converter_errors_,
                      model_errors_,
                      resolver_errors_,
                      pdu_errors_,
                      insulation_errors_,
                      throtlle_errors_};
    static inline const std::array<std::vector<std::string_view>,
                                   domains.size()>
            warnings = {sys_warnings_,
                        settings_warnings_,
                        can_warnings_,
                        sensors_warnings_,
                        motor_warnings_,
                        converter_warnings_,
                        model_warnings_,
                        resolver_warnings_,
                        pdu_warnings_,
                        insulation_warnings_,
                        throttle_warnings_};
private:
    static inline const std::vector<std::string_view> sys_messages_ = {
        "",
        "Boot CPU1...",
        "Boot CPU2...",
        "CPU1 is ready.",
        "CPU2 is ready.",
        "Device is ready.",
        "Cannot perform operation - device is busy.",
        "Device is in error state.",
        "Device is being rebooted..."};
    static inline const std::vector<std::string_view> settings_messages_ = {
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

        "Read precharge config - ok.",
        "Read precharge config - fail.",
        "Load precharge default config.",
        "Write precharge config - ok.",
        "Write precharge config - fail.",

        "Read phase current sensor config - ok.",
        "Read phase current sensor config - fail.",
        "Load phase current sensor default config.",
        "Write phase current sensor config - ok.",
        "Write phase current sensor config - fail.",

        "Read field current sensor config - ok.",
        "Read field current sensor config - fail.",
        "Load field current sensor default config.",
        "Write field current sensor config - ok.",
        "Write field current sensor config - fail.",

        "Read dc voltage sensor config - ok.",
        "Read dc voltage sensor config - fail.",
        "Load dc voltage sensor default config.",
        "Write dc voltage sensor config - ok.",
        "Write dc voltage sensor config - fail.",

        "Read resolver config - ok.",
        "Read resolver config - fail.",
        "Load resolver default config.",
        "Write resolver config - ok.",
        "Write resolver config - fail.",

        "Read resolver calibration config - ok.",
        "Read resolver calibration config - fail.",
        "Load resolver calibration default config.",
        "Write resolver calibration config - ok.",
        "Write resolver calibration config - fail.",

        "Read insulation controller config - ok.",
        "Read insulation controller config - fail.",
        "Load insulation controller default config.",
        "Write insulation controller config - ok.",
        "Write insulation controller config - fail.",

        "Read throttle controller config - ok.",
        "Read throttle controller config - fail.",
        "Load throttle controller default config.",
        "Write throttle controller config - ok.",
        "Write throttle controller config - fail.",

        "Read CAN-PDU config - ok.",
        "Read CAN-PDU config - fail.",
        "Load CAN-PDU default config.",
        "Write CAN-PDU config - ok.",
        "Write CAN-PDU config - fail.",

        "Read DIO controller config - ok.",
        "Read DIO controller config - fail.",
        "Load DIO controller default config.",
        "Write DIO controller config - ok.",
        "Write DIO controller config - fail.",
    };
    static inline const std::vector<std::string_view> can_messages_ = {
        "CAN SDO-request is lost."};
    static inline const std::vector<std::string_view> sensors_messages_ = {};
    static inline const std::vector<std::string_view> motor_messages_ = {};
    static inline const std::vector<std::string_view> converter_messages_ = {};
    static inline const std::vector<std::string_view> model_messages_{};
    static inline const std::vector<std::string_view> resolver_messages_{};
    static inline const std::vector<std::string_view> pdu_messages_{};
    static inline const std::vector<std::string_view> isolation_messages_{};
public:
    static inline const std::array<std::vector<std::string_view>,
                                   domains.size()>
            messages = {sys_messages_,
                        settings_messages_,
                        can_messages_,
                        sensors_messages_,
                        motor_messages_,
                        converter_messages_,
                        model_messages_,
                        resolver_messages_,
                        pdu_messages_,
                        isolation_messages_};
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
        if (domain_ >= syslog::domains.size() ||
            level_ >= syslog::levels.size()) {
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

} // namespace shm80
