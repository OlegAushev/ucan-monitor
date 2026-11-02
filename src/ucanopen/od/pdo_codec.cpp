#include "pdo_codec.h"

#include <algorithm>
#include <cmath>
#include <format>
#include <limits>
#include <numeric>
#include <string_view>


namespace ucanopen::od {


namespace {


uint64_t mask_of(unsigned width) {
    return (width >= 64) ? ~uint64_t(0) : ((uint64_t(1) << width) - 1);
}


uint64_t word_of(std::span<const uint8_t> data) {
    uint64_t word = 0;
    for (size_t i = 0; i < 8; ++i) {
        word |= uint64_t(data[i]) << (8 * i);
    }
    return word;
}


// The range of the raw values of a signal.
int64_t min_of(const Signal& signal) {
    if (signal.encoding != Encoding::signed_int) {
        return 0;
    }
    if (signal.width >= 64) {
        return std::numeric_limits<int64_t>::min();
    }
    return -(int64_t(1) << (signal.width - 1));
}


int64_t max_of(const Signal& signal) {
    if (signal.width >= 64) {
        return std::numeric_limits<int64_t>::max();
    }
    if (signal.encoding == Encoding::signed_int) {
        return (int64_t(1) << (signal.width - 1)) - 1;
    }
    return static_cast<int64_t>(mask_of(signal.width));
}


bool decodable(const Signal& signal) {
    return signal.role != Role::unknown && signal.encoding != Encoding::unknown;
}


// Where a signal is, for messages: its path, or its bits if it has no name.
std::string where(const Pdo& pdo, const Signal& signal) {
    if (signal.name.empty()) {
        return std::format("в битах {}–{} PDO «{}»", signal.start, signal.start + signal.width - 1, pdo.name);
    }
    return "«" + pdo.name + "." + signal.name + "»";
}


std::string join(const std::vector<std::string>& parts, std::string_view separator) {
    std::string text;
    for (size_t i = 0; i < parts.size(); ++i) {
        if (i > 0) {
            text += separator;
        }
        text += parts[i];
    }
    return text;
}


std::string counted(size_t count, std::string_view noun) {
    return std::format("{} {}{}", count, noun, (count == 1) ? "" : "s");
}


std::optional<uint32_t> cob_id_of(const Pdo& pdo, unsigned node_id) {
    if (pdo.cob_id.has_value()) {
        return *pdo.cob_id;
    }
    if (pdo.cob_base.has_value()) {
        return *pdo.cob_base + node_id;
    }
    return std::nullopt;
}


ExpeditedSdoData data_of_sam(std::span<const uint8_t> data) {
    return ExpeditedSdoData(static_cast<uint32_t>(data[4] | (data[5] << 8) | (data[6] << 16) |
                                                  (uint32_t(data[7]) << 24)));
}


// The value of an MPDO as od_decode.py writes it.
std::string sam_text(const Description& description, unsigned node_id, std::span<const uint8_t> data) {
    if ((data[0] & 0x80) != 0) {
        return std::format("refused, byte 0 is {:02X}h: an MPDO in source address mode has bit 7 clear", data[0]);
    }
    if (data[0] != node_id) {
        return std::format("refused, producer {}: the node is {}", data[0], node_id);
    }
    Key const key{static_cast<uint16_t>(data[1] | (data[2] << 8)), data[3]};
    auto const key_text = std::format("{:04X}h:{:02X}", key.index, key.subindex);
    auto const item = description.resolve(key);
    if (!item.has_value()) {
        return key_text + " unknown";
    }

    // An element goes by the path of its array; the count of an array and a
    // highest subindex have none.
    std::string path;
    if (item->kind == Item::Kind::element) {
        path = item->array->group + "." + item->array->name;
    } else if (item->kind != Item::Kind::count && item->kind != Item::Kind::highest_subindex) {
        path = item->path;
    }

    auto const sdo = data_of_sam(data);
    std::optional<int64_t> raw;
    switch (item->datatype) {
    case DataType::boolean:
    case DataType::uint8:
        raw = sdo.u8();
        break;
    case DataType::int8:
        raw = sdo.i8();
        break;
    case DataType::int16:
        raw = sdo.i16();
        break;
    case DataType::uint16:
        raw = sdo.u16();
        break;
    case DataType::int32:
        raw = sdo.i32();
        break;
    case DataType::uint32:
        raw = sdo.u32();
        break;
    default:
        break;
    }

    std::string text;
    if (item->values != nullptr && raw.has_value()) {
        const auto* value = item->values->find(*raw);
        text = (value != nullptr) ? value->name : std::to_string(*raw) + "?";
    } else {
        text = (item->datatype == DataType::float32) ? std::format("{:.6g}", sdo.f32())
                                                     : (raw.has_value() ? std::to_string(*raw) : "?");
        if (!item->unit.en.empty()) {
            text += " " + item->unit.en;
        }
    }
    return path.empty() ? key_text + "=" + text : key_text + " " + path + "=" + text;
}


} // namespace


std::optional<DecodedPdo> decode_pdo(const Description& description, const Pdo& pdo,
                                     std::span<const uint8_t> data) {
    if (data.size() < 8 || pdo.mapping != Mapping::signals) {
        return std::nullopt;
    }

    auto const word = word_of(data);
    DecodedPdo decoded;
    uint64_t covered = 0;
    for (const auto& signal : pdo.signals) {
        auto const mask = mask_of(signal.width);
        covered |= mask << signal.start;
        if (!decodable(signal)) {
            continue;
        }
        auto const bits = (word >> signal.start) & mask;
        auto raw = static_cast<int64_t>(bits);
        if (signal.encoding == Encoding::signed_int && signal.width < 64 && ((bits >> (signal.width - 1)) & 1) != 0) {
            raw = static_cast<int64_t>(bits | ~mask);
        }
        SignalValue value{&signal, raw};
        if (const auto* table = description.value_table(signal.value_table); table != nullptr) {
            value.named = table->find(raw);
        }
        decoded.signals.push_back(value);
    }
    decoded.reserved = word & ~covered;
    return decoded;
}


double physical(const Signal& signal, int64_t raw) {
    return static_cast<double>(raw) * static_cast<double>(signal.scale_num) / static_cast<double>(signal.scale_den);
}


std::string physical_text(const Signal& signal, int64_t raw) {
    int64_t num = 0;
    if (__builtin_mul_overflow(raw, signal.scale_num, &num)) {
        return std::format("{:.6g}", physical(signal, raw));
    }
    int64_t den = signal.scale_den;
    if (auto const divisor = std::gcd(num, den); divisor > 1) {
        num /= divisor;
        den /= divisor;
    }

    // A decimal that ends: the denominator has no prime factor but 2 and 5.
    int64_t rest = den;
    int twos = 0;
    int fives = 0;
    while (rest % 2 == 0) {
        rest /= 2;
        ++twos;
    }
    while (rest % 5 == 0) {
        rest /= 5;
        ++fives;
    }
    int const digits = std::max(twos, fives);
    if (rest != 1 || digits > 18) {
        return std::format("{:.6g}", physical(signal, raw));
    }
    int64_t power = 1;
    for (int i = 0; i < digits; ++i) {
        power *= 10;
    }
    int64_t scaled = 0;
    if (__builtin_mul_overflow(num, power / den, &scaled)) {
        return std::format("{:.6g}", physical(signal, raw));
    }

    bool const negative = scaled < 0;
    auto text = std::to_string(negative ? -static_cast<uint64_t>(scaled) : static_cast<uint64_t>(scaled));
    if (digits > 0) {
        if (text.size() <= static_cast<size_t>(digits)) {
            text.insert(0, static_cast<size_t>(digits) - text.size() + 1, '0');
        }
        text.insert(text.size() - static_cast<size_t>(digits), ".");
    }
    return negative ? "-" + text : text;
}


std::optional<SamValue> decode_sam(std::span<const uint8_t> data, unsigned node_id) {
    if (data.size() < 8 || (data[0] & 0x80) != 0 || data[0] != node_id) {
        return std::nullopt;
    }
    return SamValue{{static_cast<uint16_t>(data[1] | (data[2] << 8)), data[3]}, data_of_sam(data)};
}


std::optional<std::string> encoding_refusal(const Pdo& pdo, const std::set<std::string, std::less<>>& names) {
    if (pdo.mapping != Mapping::signals) {
        return "PDO «" + pdo.name + "» не состоит из сигналов";
    }
    for (const auto& signal : pdo.signals) {
        if (signal.role == Role::unknown) {
            return "у сигнала " + where(pdo, signal) + " незнакомая роль";
        }
        if (signal.encoding == Encoding::unknown) {
            return "у сигнала " + where(pdo, signal) + " незнакомая кодировка";
        }
        if (signal.role == Role::value && !names.contains(signal.name)) {
            return "сигнал " + where(pdo, signal) + " программа не заполняет";
        }
    }
    for (const auto& name : names) {
        auto const found = std::any_of(pdo.signals.begin(), pdo.signals.end(), [&name](const Signal& signal) {
            return signal.role == Role::value && signal.name == name;
        });
        if (!found) {
            return "у PDO «" + pdo.name + "» нет сигнала «" + name + "»";
        }
    }
    return std::nullopt;
}


std::expected<EncodedPdo, std::string> encode_pdo(const Description& description, const Pdo& pdo,
                                                  const std::map<std::string, double, std::less<>>& values,
                                                  unsigned counter) {
    std::set<std::string, std::less<>> names;
    for (const auto& [name, value] : values) {
        names.insert(name);
    }
    if (auto refusal = encoding_refusal(pdo, names); refusal.has_value()) {
        return std::unexpected(*refusal);
    }

    EncodedPdo encoded{};
    uint64_t word = 0;
    for (const auto& signal : pdo.signals) {
        auto const mask = mask_of(signal.width);
        int64_t raw = 0;
        if (signal.role == Role::counter) {
            raw = static_cast<int64_t>(counter & mask);
        } else {
            auto const value = values.find(signal.name)->second;
            auto const path = pdo.name + "." + signal.name;
            if (std::isnan(value)) {
                return std::unexpected("сигнал «" + path + "»: не число");
            }
            if (const auto* table = description.value_table(signal.value_table); table != nullptr) {
                // a value of the table goes by its number
                bool const integral = std::fabs(value) < 9.0e18 && std::trunc(value) == value;
                raw = integral ? static_cast<int64_t>(value) : 0;
                if (!integral || table->find(raw) == nullptr) {
                    return std::unexpected(std::format("сигнал «{}»: значение {} таблица не называет", path, value));
                }
            } else {
                double const scaled = value * static_cast<double>(signal.scale_den) /
                                      static_cast<double>(signal.scale_num);
                auto const min = min_of(signal);
                auto const max = max_of(signal);
                if (scaled < static_cast<double>(min) || scaled > static_cast<double>(max)) {
                    raw = (scaled < static_cast<double>(min)) ? min : max;
                    encoded.saturated.push_back(path);
                } else {
                    raw = std::clamp(static_cast<int64_t>(std::llround(scaled)), min, max);
                }
            }
        }
        word |= (static_cast<uint64_t>(raw) & mask) << signal.start;
    }

    for (size_t i = 0; i < encoded.payload.size(); ++i) {
        encoded.payload[i] = static_cast<uint8_t>(word >> (8 * i));
    }
    return encoded;
}


std::optional<CobTpdo> tpdo_of(const Pdo& pdo) {
    if (pdo.direction != Direction::tx || pdo.cob_id.has_value() || pdo.number < 1 || pdo.number > 4 ||
        pdo.cob_base != 0x180 + 0x100 * (pdo.number - 1)) {
        return std::nullopt;
    }
    return static_cast<CobTpdo>(pdo.number - 1);
}


std::optional<CobRpdo> rpdo_of(const Pdo& pdo) {
    if (pdo.direction != Direction::rx || pdo.cob_id.has_value() || pdo.number < 1 || pdo.number > 4 ||
        pdo.cob_base != 0x200 + 0x100 * (pdo.number - 1)) {
        return std::nullopt;
    }
    return static_cast<CobRpdo>(pdo.number - 1);
}


std::string candump_line(const Description& description, unsigned node_id, uint32_t cob_id,
                         std::span<const uint8_t> data) {
    auto const name = std::format("{:03X}", cob_id);
    const auto& pdos = description.pdos();
    auto const pdo = std::find_if(pdos.begin(), pdos.end(), [&](const Pdo& p) {
        return cob_id_of(p, node_id) == cob_id;
    });
    if (pdo == pdos.end()) {
        return name + " unknown";
    }

    auto const head = std::format("{} {}{} {}", name, (pdo->direction == Direction::tx) ? "TPDO" : "RPDO",
                                  pdo->number, pdo->name);
    if (data.size() < 8) {
        return std::format("{}: refused, {}: a PDO takes 8", head, counted(data.size(), "byte"));
    }
    if (pdo->mapping == Mapping::sam) {
        return head + ": " + sam_text(description, node_id, data);
    }

    auto const decoded = decode_pdo(description, *pdo, data);
    std::vector<std::string> values;
    std::optional<std::string> unnamed;
    for (const auto& value : decoded->signals) {
        const auto& signal = *value.signal;
        auto const key = signal.name.empty() ? std::string("counter") : pdo->name + "." + signal.name;
        std::string text;
        if (signal.value_table.has_value()) {
            if (value.named != nullptr) {
                text = value.named->name;
            } else {
                text = std::to_string(value.raw) + "?";
                if (!unnamed.has_value()) {
                    unnamed = std::format("{} holds {}, which no label names", key, value.raw);
                }
            }
        } else {
            text = physical_text(signal, value.raw);
            if (!signal.unit.en.empty()) {
                text += " " + signal.unit.en;
            }
        }
        values.push_back(key + "=" + text);
    }

    auto line = head + ": " + join(values, ", ");
    if (pdo->direction == Direction::rx && decoded->reserved != 0) {
        std::vector<std::string> bits;
        for (unsigned bit = 0; bit < 64; ++bit) {
            if (((decoded->reserved >> bit) & 1) != 0) {
                bits.push_back(std::to_string(bit));
            }
        }
        line += std::format("; refused: reserved {} {}", (bits.size() > 1) ? "bits" : "bit", join(bits, ", "));
    } else if (pdo->direction == Direction::rx && unnamed.has_value()) {
        line += "; refused: " + *unnamed;
    }
    return line;
}


} // namespace ucanopen::od
