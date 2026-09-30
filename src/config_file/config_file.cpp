#include "config_file.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <charconv>
#include <cmath>
#include <fstream>
#include <map>
#include <system_error>
#include <type_traits>


namespace config_file {


namespace {


std::string_view trim(std::string_view text) {
    constexpr std::string_view blanks = " \t\r";
    auto begin = text.find_first_not_of(blanks);
    if (begin == std::string_view::npos) {
        return {};
    }
    auto end = text.find_last_not_of(blanks);
    return text.substr(begin, end - begin + 1);
}


// A comment begins at the start of a line or after a blank, so that a value
// with `#` inside, as a branch name may have, keeps it.
std::string_view strip_comment(std::string_view text) {
    for (size_t i = 0; i < text.size(); ++i) {
        bool const starts_word = (i == 0) || (text[i - 1] == ' ') || (text[i - 1] == '\t');
        if ((text[i] == '#' || text[i] == ';') && starts_word) {
            return text.substr(0, i);
        }
    }
    return text;
}


std::string at_line(int line, std::string_view message) {
    return "строка " + std::to_string(line) + ": " + std::string(message);
}


std::string one_line(std::string_view text) {
    std::string ret(text);
    std::replace(ret.begin(), ret.end(), '\n', ' ');
    std::replace(ret.begin(), ret.end(), '\r', ' ');
    return ret;
}


std::string padded(std::string_view text, size_t width) {
    std::string ret(text);
    if (ret.size() < width) {
        ret.append(width - ret.size(), ' ');
    }
    return ret;
}


// from_chars takes no plus sign, people write one now and then.
std::string_view strip_plus(std::string_view text) {
    if (text.size() > 1 && text[0] == '+' && text[1] != '-' && text[1] != '+') {
        text.remove_prefix(1);
    }
    return text;
}


template<typename T>
std::optional<T> parse_number(std::string_view text, int base = 10) {
    T value{};
    auto const end = text.data() + text.size();
    std::from_chars_result result;
    if constexpr (std::is_floating_point_v<T>) {
        result = std::from_chars(text.data(), end, value);
    } else {
        result = std::from_chars(text.data(), end, value, base);
    }
    if (text.empty() || result.ec != std::errc() || result.ptr != end) {
        return std::nullopt;
    }
    return value;
}


template<typename T>
std::optional<ucanopen::ExpeditedSdoData> parse_signed(std::string_view text) {
    auto value = parse_number<T>(strip_plus(text));
    if (!value.has_value()) {
        return std::nullopt;
    }
    return ucanopen::ExpeditedSdoData(*value);
}


template<typename T>
std::optional<ucanopen::ExpeditedSdoData> parse_unsigned(std::string_view text) {
    std::optional<T> value;
    if (text.starts_with("0x") || text.starts_with("0X")) {
        value = parse_number<T>(text.substr(2), 16);
    } else {
        value = parse_number<T>(strip_plus(text));
    }
    if (!value.has_value()) {
        return std::nullopt;
    }
    return ucanopen::ExpeditedSdoData(*value);
}


std::optional<ucanopen::ExpeditedSdoData> parse_bool(std::string_view text) {
    std::string lower(text);
    std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) { return std::tolower(c); });
    if (lower == "true" || lower == "on" || lower == "1") {
        return ucanopen::ExpeditedSdoData(true);
    }
    if (lower == "false" || lower == "off" || lower == "0") {
        return ucanopen::ExpeditedSdoData(false);
    }
    return std::nullopt;
}


template<typename T>
std::string format_number(T value, int base = 10) {
    std::array<char, 32> buf;
    std::to_chars_result result;
    if constexpr (std::is_floating_point_v<T>) {
        // the shortest form that reads back as the same float
        result = std::to_chars(buf.data(), buf.data() + buf.size(), value);
    } else {
        result = std::to_chars(buf.data(), buf.data() + buf.size(), value, base);
    }
    return std::string(buf.data(), result.ptr);
}


template<typename T>
std::string format_unsigned(T value, bool hex) {
    if (!hex) {
        return format_number(value);
    }
    auto digits = format_number(value, 16);
    std::transform(digits.begin(), digits.end(), digits.begin(), [](unsigned char c) { return std::toupper(c); });
    return "0x" + std::string(2 * sizeof(T) - digits.size(), '0') + digits;
}


} // namespace


std::expected<Contents, std::string> read(const std::filesystem::path& path) {
    std::ifstream file(path);
    if (!file) {
        return std::unexpected("не удалось открыть файл");
    }

    Contents contents;
    std::map<std::pair<std::string, std::string>, size_t> seen; // parameter -> its place in contents
    std::optional<std::string> section; // none before the first one, the header's lines
    bool section_valid = true;

    std::string buf;
    int line = 0;
    while (std::getline(file, buf)) {
        ++line;
        std::string_view text = buf;
        if (line == 1 && text.starts_with("\xEF\xBB\xBF")) {
            text.remove_prefix(3); // a BOM left by a Windows editor
        }
        text = trim(strip_comment(text));
        if (text.empty()) {
            continue;
        }

        if (text.front() == '[') {
            auto name = (text.back() == ']') ? trim(text.substr(1, text.size() - 2)) : std::string_view{};
            section_valid = !name.empty();
            if (!section_valid) {
                contents.warnings.push_back(at_line(line, "раздел не распознан, его параметры пропущены"));
            }
            section = std::string(name);
            continue;
        }

        if (!section_valid) {
            continue;
        }

        auto eq = text.find('=');
        if (eq == std::string_view::npos || trim(text.substr(0, eq)).empty()) {
            contents.warnings.push_back(at_line(line, "ожидалось «имя = значение», строка пропущена"));
            continue;
        }
        std::string key(trim(text.substr(0, eq)));
        std::string value(trim(text.substr(eq + 1)));

        if (!section.has_value()) {
            contents.header.emplace_back(std::move(key), std::move(value));
            continue;
        }

        auto [iter, inserted] = seen.try_emplace({*section, key}, contents.parameters.size());
        if (!inserted) {
            auto& parameter = contents.parameters[iter->second];
            contents.warnings.push_back(at_line(line,
                    "[" + *section + "] " + key + " уже задан в строке " + std::to_string(parameter.line)
                    + ", берётся это значение"));
            parameter.value = std::move(value);
            parameter.line = line;
            continue;
        }
        contents.parameters.push_back({*section, std::move(key), std::move(value), line});
    }

    if (file.bad()) {
        return std::unexpected("ошибка чтения файла");
    }
    return contents;
}


std::expected<void, std::string> write(const std::filesystem::path& path,
                                       const Header& header,
                                       const std::vector<Value>& values) {
    // Subcategories in the order they first come, so that the file follows
    // the dictionary rather than the alphabet.
    std::vector<std::string_view> subcategories;
    std::map<std::string_view, std::vector<const Value*>> sections;
    for (const auto& value : values) {
        auto& section = sections[value.object->subcategory];
        if (section.empty()) {
            subcategories.push_back(value.object->subcategory);
        }
        section.push_back(&value);
    }

    std::ofstream file(path);
    if (!file) {
        return std::unexpected("не удалось создать файл");
    }

    file << "# Настройки сервера, сохранённые uCAN Monitor.\n"
            "# Параметры только для чтения и непрочитанные закомментированы:\n"
            "# при загрузке они не записываются.\n";
    for (const auto& [key, value] : header) {
        file << key << " = " << one_line(value) << '\n';
    }

    struct Line {
        bool active;
        std::string_view name;
        std::string value;
        std::string comment;
    };

    for (auto subcategory : subcategories) {
        std::vector<Line> lines;
        size_t name_width = 0;
        size_t value_width = 0;
        for (const auto* value : sections[subcategory]) {
            const auto& object = *value->object;
            Line line{value->data.has_value() && object.has_write_permission(),
                      object.name,
                      value->data.has_value() ? format_value(object, *value->data) : std::string{},
                      (object.unit == "hex") ? std::string{} : object.unit}; // 0x says it already
            if (!value->data.has_value()) {
                line.comment += line.comment.empty() ? "не прочитано" : ", не прочитано";
            }
            name_width = std::max(name_width, line.name.size());
            if (!line.comment.empty()) {
                value_width = std::max(value_width, line.value.size());
            }
            lines.push_back(std::move(line));
        }

        file << "\n[" << subcategory << "]\n";
        for (const auto& line : lines) {
            file << (line.active ? "" : "# ") << padded(line.name, name_width) << " = ";
            if (line.comment.empty()) {
                file << line.value << '\n';
            } else {
                file << padded(line.value, value_width) << "  # " << line.comment << '\n';
            }
        }
    }

    file.close();
    if (!file) {
        return std::unexpected("ошибка записи файла");
    }
    return {};
}


std::string format_value(const ucanopen::ODObject& object, ucanopen::ExpeditedSdoData data) {
    bool const hex = (object.unit == "hex");
    switch (object.data_type) {
    case ucanopen::OD_BOOL:
        return data.u8() ? "true" : "false";
    case ucanopen::OD_INT8:
        return format_number(data.i8());
    case ucanopen::OD_INT16:
        return format_number(data.i16());
    case ucanopen::OD_INT32:
        return format_number(data.i32());
    case ucanopen::OD_UINT8:
        return format_unsigned(data.u8(), hex);
    case ucanopen::OD_UINT16:
        return format_unsigned(data.u16(), hex);
    case ucanopen::OD_UINT32:
        return format_unsigned(data.u32(), hex);
    case ucanopen::OD_FLOAT32:
        return format_number(data.f32());
    default:
        return {};
    }
}


std::optional<ucanopen::ExpeditedSdoData> parse_value(const ucanopen::ODObject& object, std::string_view text) {
    switch (object.data_type) {
    case ucanopen::OD_BOOL:
        return parse_bool(text);
    case ucanopen::OD_INT8:
        return parse_signed<int8_t>(text);
    case ucanopen::OD_INT16:
        return parse_signed<int16_t>(text);
    case ucanopen::OD_INT32:
        return parse_signed<int32_t>(text);
    case ucanopen::OD_UINT8:
        return parse_unsigned<uint8_t>(text);
    case ucanopen::OD_UINT16:
        return parse_unsigned<uint16_t>(text);
    case ucanopen::OD_UINT32:
        return parse_unsigned<uint32_t>(text);
    case ucanopen::OD_FLOAT32:
        return parse_signed<float>(text);
    default:
        return std::nullopt;
    }
}


bool same_value(const ucanopen::ODObject& object, ucanopen::ExpeditedSdoData a, ucanopen::ExpeditedSdoData b) {
    switch (object.data_type) {
    case ucanopen::OD_BOOL:
        return (a.u8() != 0) == (b.u8() != 0);
    case ucanopen::OD_INT8:
        return a.i8() == b.i8();
    case ucanopen::OD_INT16:
        return a.i16() == b.i16();
    case ucanopen::OD_UINT8:
        return a.u8() == b.u8();
    case ucanopen::OD_UINT16:
        return a.u16() == b.u16();
    case ucanopen::OD_FLOAT32: {
        float x = a.f32();
        float y = b.f32();
        if (x == y) {
            return true;
        }
        if (!std::isfinite(x) || !std::isfinite(y)) {
            return std::isnan(x) && std::isnan(y);
        }
        return std::fabs(x - y) <= 1e-6f * std::max(std::fabs(x), std::fabs(y));
    }
    default:
        return a.u32() == b.u32();
    }
}


} // namespace config_file
