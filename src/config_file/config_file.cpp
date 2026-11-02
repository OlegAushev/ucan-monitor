#include "config_file.h"

#include <ucanopen/od/value.h>

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
    // Groups in the order they first come, so that the file follows the
    // dictionary rather than the alphabet.
    std::vector<std::string_view> groups;
    std::map<std::string_view, std::vector<const Value*>> sections;
    for (const auto& value : values) {
        auto& section = sections[value.group];
        if (section.empty()) {
            groups.push_back(value.group);
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

    for (auto group : groups) {
        std::vector<Line> lines;
        size_t name_width = 0;
        size_t value_width = 0;
        for (const auto* value : sections[group]) {
            const auto& unit = value->item.unit.ru.empty() ? value->item.unit.en : value->item.unit.ru;
            Line line{value->data.has_value() && value->writable,
                      value->name,
                      value->data.has_value() ? config_file::format_value(value->item, *value->data) : std::string{},
                      unit};
            if (!value->data.has_value()) {
                line.comment += line.comment.empty() ? "не прочитано" : ", не прочитано";
            }
            name_width = std::max(name_width, line.name.size());
            if (!line.comment.empty()) {
                value_width = std::max(value_width, line.value.size());
            }
            lines.push_back(std::move(line));
        }

        file << "\n[" << group << "]\n";
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


std::string format_value(const ucanopen::od::Item& item, ucanopen::ExpeditedSdoData data) {
    using ucanopen::od::DataType;
    if (item.values != nullptr && item.datatype != DataType::float32) {
        // a value of a table by its name, which reads back whatever the labels
        auto const raw = static_cast<int64_t>(ucanopen::od::to_double(item, data));
        const auto* value = item.values->find(raw);
        return (value != nullptr) ? value->name : std::to_string(raw);
    }
    switch (item.datatype) {
    case DataType::boolean:
        return data.u8() ? "true" : "false";
    case DataType::int8:
        return format_number(data.i8());
    case DataType::int16:
        return format_number(data.i16());
    case DataType::int32:
        return format_number(data.i32());
    case DataType::uint8:
        return format_number(data.u8());
    case DataType::uint16:
        return format_number(data.u16());
    case DataType::uint32:
        return format_number(data.u32());
    case DataType::float32:
        return format_number(data.f32());
    default:
        return {};
    }
}


std::optional<ucanopen::ExpeditedSdoData> parse_value(const ucanopen::od::Item& item, std::string_view text) {
    return ucanopen::od::parse_value(item, text);
}


bool same_value(const ucanopen::od::Item& item, ucanopen::ExpeditedSdoData a, ucanopen::ExpeditedSdoData b) {
    using ucanopen::od::DataType;
    switch (item.datatype) {
    case DataType::boolean:
        return (a.u8() != 0) == (b.u8() != 0);
    case DataType::int8:
        return a.i8() == b.i8();
    case DataType::int16:
        return a.i16() == b.i16();
    case DataType::uint8:
        return a.u8() == b.u8();
    case DataType::uint16:
        return a.u16() == b.u16();
    case DataType::float32: {
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
