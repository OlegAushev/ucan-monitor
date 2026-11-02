#include "value.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <charconv>
#include <cmath>
#include <cstdint>
#include <limits>
#include <system_error>
#include <type_traits>


namespace ucanopen::od {


namespace {


// The value of an item of an integer type as a number, which its table names.
std::optional<int64_t> integer_of(DataType type, ExpeditedSdoData data) {
    switch (type) {
    case DataType::boolean:
        return (data.u8() != 0) ? 1 : 0;
    case DataType::int8:
        return data.i8();
    case DataType::int16:
        return data.i16();
    case DataType::int32:
        return data.i32();
    case DataType::uint8:
        return data.u8();
    case DataType::uint16:
        return data.u16();
    case DataType::uint32:
        return data.u32();
    default:
        return std::nullopt;
    }
}


template<typename T>
bool fits(int64_t value) {
    return value >= std::numeric_limits<T>::min() && value <= std::numeric_limits<T>::max();
}


// The number as a value of an integer type, if it fits the type.
std::optional<ExpeditedSdoData> data_of(DataType type, int64_t value) {
    switch (type) {
    case DataType::boolean:
        if (value == 0 || value == 1) {
            return ExpeditedSdoData(value == 1);
        }
        break;
    case DataType::int8:
        if (fits<int8_t>(value)) {
            return ExpeditedSdoData(static_cast<int8_t>(value));
        }
        break;
    case DataType::int16:
        if (fits<int16_t>(value)) {
            return ExpeditedSdoData(static_cast<int16_t>(value));
        }
        break;
    case DataType::int32:
        if (fits<int32_t>(value)) {
            return ExpeditedSdoData(static_cast<int32_t>(value));
        }
        break;
    case DataType::uint8:
        if (fits<uint8_t>(value)) {
            return ExpeditedSdoData(static_cast<uint8_t>(value));
        }
        break;
    case DataType::uint16:
        if (fits<uint16_t>(value)) {
            return ExpeditedSdoData(static_cast<uint16_t>(value));
        }
        break;
    case DataType::uint32:
        if (fits<uint32_t>(value)) {
            return ExpeditedSdoData(static_cast<uint32_t>(value));
        }
        break;
    default:
        break;
    }
    return std::nullopt;
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


// A decimal number with a sign, or a hexadecimal one after 0x.
std::optional<int64_t> parse_integer(std::string_view text) {
    if (text.starts_with("0x") || text.starts_with("0X")) {
        auto value = parse_number<uint64_t>(text.substr(2), 16);
        if (!value.has_value() || *value > uint64_t(std::numeric_limits<int64_t>::max())) {
            return std::nullopt;
        }
        return static_cast<int64_t>(*value);
    }
    return parse_number<int64_t>(strip_plus(text));
}


std::optional<ExpeditedSdoData> parse_bool(std::string_view text) {
    std::string lower(text);
    std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) { return std::tolower(c); });
    if (lower == "true" || lower == "on" || lower == "1") {
        return ExpeditedSdoData(true);
    }
    if (lower == "false" || lower == "off" || lower == "0") {
        return ExpeditedSdoData(false);
    }
    return std::nullopt;
}


} // namespace


size_t size_of(DataType type) {
    switch (type) {
    case DataType::boolean:
    case DataType::int8:
    case DataType::uint8:
        return 1;
    case DataType::int16:
    case DataType::uint16:
        return 2;
    case DataType::int32:
    case DataType::uint32:
    case DataType::float32:
        return 4;
    default:
        return 0;
    }
}


std::string format_value(const Item& item, ExpeditedSdoData data, int precision) {
    if (item.values != nullptr) {
        if (auto raw = integer_of(item.datatype, data); raw.has_value()) {
            const auto* value = item.values->find(*raw);
            if (value == nullptr) {
                return std::to_string(*raw) + "?";
            }
            return value->label.ru.empty() ? value->name : value->label.ru;
        }
    }

    switch (item.datatype) {
    case DataType::boolean:
        return (data.u8() != 0) ? "да" : "нет";
    case DataType::int8:
        return std::to_string(data.i8());
    case DataType::int16:
        return std::to_string(data.i16());
    case DataType::int32:
        return std::to_string(data.i32());
    case DataType::uint8:
        return std::to_string(data.u8());
    case DataType::uint16:
        return std::to_string(data.u16());
    case DataType::uint32:
        return std::to_string(data.u32());
    case DataType::float32: {
        float const value = data.f32();
        auto const format = (std::fabs(value) >= 0.01f) ? std::chars_format::fixed : std::chars_format::general;
        std::array<char, 64> buf;
        auto result = std::to_chars(buf.data(), buf.data() + buf.size(), value, format, precision);
        if (result.ec != std::errc()) {
            return std::make_error_code(result.ec).message();
        }
        return std::string(buf.data(), result.ptr);
    }
    default:
        return {};
    }
}


double to_double(const Item& item, ExpeditedSdoData data) {
    switch (item.datatype) {
    case DataType::boolean:
        return (data.u8() != 0) ? 1.0 : 0.0;
    case DataType::int8:
        return data.i8();
    case DataType::int16:
        return data.i16();
    case DataType::int32:
        return data.i32();
    case DataType::uint8:
        return data.u8();
    case DataType::uint16:
        return data.u16();
    case DataType::uint32:
        return data.u32();
    case DataType::float32:
        return data.f32();
    default:
        return std::nan("");
    }
}


std::optional<ExpeditedSdoData> parse_value(const Item& item, std::string_view text) {
    if (item.values != nullptr) {
        std::optional<int64_t> raw;
        if (const auto* value = item.values->find(text); value != nullptr) {
            raw = value->value;
        } else {
            raw = parse_integer(text);
        }
        if (!raw.has_value() || item.values->find(*raw) == nullptr) {
            return std::nullopt;
        }
        return data_of(item.datatype, *raw);
    }

    switch (item.datatype) {
    case DataType::boolean:
        return parse_bool(text);
    case DataType::int8:
    case DataType::int16:
    case DataType::int32:
    case DataType::uint8:
    case DataType::uint16:
    case DataType::uint32: {
        auto raw = parse_integer(text);
        if (!raw.has_value()) {
            return std::nullopt;
        }
        return data_of(item.datatype, *raw);
    }
    case DataType::float32: {
        auto value = parse_number<float>(strip_plus(text));
        if (!value.has_value()) {
            return std::nullopt;
        }
        return ExpeditedSdoData(*value);
    }
    default:
        return std::nullopt;
    }
}


} // namespace ucanopen::od
