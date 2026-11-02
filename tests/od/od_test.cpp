// The checks of the reading of a description of the dictionary and of the
// frames of its PDOs, which CTest runs case by case:
//
//   od-test <case> <description> <directory of expectations>
//
// A check that fails says what it expected and what it got; the exit code is
// 0 only if every check of the case held. The cases are those of section 5.1
// of docs/canopen-rework-plan.ru.md.

#include <ucanopen/od/description.h>
#include <ucanopen/od/pdo_codec.h>
#include <ucanopen/od/value.h>

#include <algorithm>
#include <charconv>
#include <cstdio>
#include <filesystem>
#include <format>
#include <fstream>
#include <functional>
#include <iterator>
#include <map>
#include <string>
#include <string_view>
#include <vector>

using namespace ucanopen;

namespace {

int failures = 0;

void say(std::string_view text) {
    std::fprintf(stderr, "%.*s\n", static_cast<int>(text.size()), text.data());
}

void fail(std::string_view what) {
    say(std::format("FAILED: {}", what));
    ++failures;
}

void check(bool condition, std::string_view what) {
    if (!condition) {
        fail(what);
    }
}

template<typename T, typename U>
void check_equal(const T& got, const U& expected, std::string_view what) {
    if (!(got == expected)) {
        fail(std::format("{}: expected {}, got {}", what, expected, got));
    }
}

std::string read_file(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);
    return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

std::shared_ptr<const od::Description> load(const std::filesystem::path& path) {
    auto description = od::load(path);
    if (!description.has_value()) {
        fail(std::format("{} does not load: {}", path.string(), description.error()));
        return nullptr;
    }
    return *description;
}

std::string key_text(od::Key key) {
    return std::format("{:04X}h:{:02X}", key.index, key.subindex);
}

// The summary of a description as od_decode.py --check prints it.
std::string summary(const od::Description& description) {
    auto counted = [](size_t count, std::string_view noun) {
        return std::format("{} {}{}", count, noun, (count == 1) ? "" : "s");
    };
    size_t values = 0;
    for (const auto& table : description.value_tables()) {
        values += table.values.size();
    }
    size_t members = 0;
    for (const auto& layout : description.layouts()) {
        members += layout.fields.size();
    }
    size_t signals = 0;
    for (const auto& pdo : description.pdos()) {
        signals += pdo.signals.size();
    }
    return std::format("{}, fingerprint 0x{:08X}: {}, {} of {}, {} with {}, {}, {}, {} and {} with {}",
                       description.device(),
                       description.fingerprint(),
                       counted(description.groups().size(), "group"),
                       counted(description.value_tables().size(), "table"),
                       counted(values, "value"),
                       counted(description.layouts().size(), "layout"),
                       counted(members, "member"),
                       counted(description.objects().size(), "object"),
                       counted(description.arrays().size(), "array"),
                       counted(description.records().size(), "record"),
                       counted(description.pdos().size(), "PDO"),
                       counted(signals, "signal"));
}

// The text with `old` replaced by `replacement`, which it must hold once.
std::string replaced(std::string text, std::string_view old, std::string_view replacement) {
    auto const at = text.find(old);
    if (at == std::string::npos || text.find(old, at + 1) != std::string::npos) {
        fail(std::format("the description does not hold {} once", old));
        return text;
    }
    return text.replace(at, old.size(), replacement);
}

std::vector<std::string> read_lines(const std::filesystem::path& path) {
    std::vector<std::string> lines;
    std::ifstream file(path);
    for (std::string line; std::getline(file, line);) {
        lines.push_back(line);
    }
    return lines;
}

// A frame of a log of candump as candump -L writes it, "can0 181#0102".
struct Frame {
    uint32_t id;
    std::vector<uint8_t> data;
};

std::vector<Frame> read_candump(const std::filesystem::path& path) {
    std::vector<Frame> frames;
    auto const lines = read_lines(path);
    for (size_t i = 0; i < lines.size(); ++i) {
        std::string_view line = lines[i];
        while (!line.empty() && (line.back() == ' ' || line.back() == '\r')) {
            line.remove_suffix(1);
        }
        if (line.empty() || line.front() == '#') {
            continue;
        }
        auto const token = line.substr(line.find_last_of(' ') + 1);
        auto const hash = token.find('#');
        Frame frame{};
        auto result = std::from_chars(token.data(), token.data() + hash, frame.id, 16);
        bool parsed = hash != std::string_view::npos && result.ptr == token.data() + hash &&
                      (token.size() - hash - 1) % 2 == 0;
        for (size_t at = hash + 1; parsed && at < token.size(); at += 2) {
            uint8_t byte = 0;
            parsed = std::from_chars(token.data() + at, token.data() + at + 2, byte, 16).ptr == token.data() + at + 2;
            frame.data.push_back(byte);
        }
        if (!parsed) {
            fail(std::format("{}, line {}: not a frame of candump -L", path.string(), i + 1));
            continue;
        }
        frames.push_back(std::move(frame));
    }
    return frames;
}

std::string hex_of(const std::vector<uint8_t>& data) {
    std::string text;
    for (auto byte : data) {
        text += std::format("{:02X}", byte);
    }
    return text;
}

// The text with the fingerprint of its header made the hash of its body again.
std::string refingerprinted(std::string text) {
    auto const header_end = text.find('\n');
    auto const hash = od::fnv1a(std::string_view(text).substr(header_end + 1));
    std::string_view const key = "\"fingerprint\":\"0x";
    auto const at = text.find(key);
    if (at == std::string::npos || at > header_end) {
        fail("the header has no fingerprint");
        return text;
    }
    return text.replace(at + key.size(), 8, std::format("{:08X}", hash));
}

//----------------------------------------------------------------------------------------------------------------------
// 1. The description reads, its counts are those od_decode.py --check gave
// when update_od.sh took it, and its fingerprint is the hash of its body.
void check_description(const std::filesystem::path& path, const std::filesystem::path& expectations) {
    auto description = load(path);
    if (!description) {
        return;
    }

    auto expected = read_file(expectations / "description.expected");
    while (!expected.empty() && (expected.back() == '\n' || expected.back() == '\r')) {
        expected.pop_back();
    }
    check_equal(summary(*description), expected, "the summary");

    auto const text = read_file(path);
    auto const hash = od::fnv1a(std::string_view(text).substr(text.find('\n') + 1));
    check_equal(hash, description->fingerprint(), "the hash of the body");
}

// 2. What stands at the keys of each kind.
void check_resolve(const std::filesystem::path& path, const std::filesystem::path&) {
    auto description = load(path);
    if (!description) {
        return;
    }

    auto expect = [&](od::Key key, std::string_view path, od::Item::Kind kind) -> std::optional<od::Item> {
        auto item = description->resolve(key);
        if (!item.has_value()) {
            fail(std::format("{} resolves to nothing, expected {}", key_text(key), path));
            return std::nullopt;
        }
        check_equal(item->path, path, std::format("the path at {}", key_text(key)));
        check(item->kind == kind, std::format("the kind of {}", key_text(key)));
        check(item->available, std::format("{} is available", key_text(key)));
        return item;
    };

    if (auto item = expect({0x3161, 0x02}, "motor.R.applied", od::Item::Kind::member)) {
        check(item->datatype == od::DataType::float32, "motor.R.applied is a float32");
        check(item->access == od::Access::ro, "motor.R.applied is read-only");
        check_equal(item->unit.ru, "Ом", "the unit of motor.R.applied");
        check(item->record != nullptr && item->record->name == "R", "motor.R.applied is a member of motor.R");
    }

    if (auto item = expect({0x3185, 0x01}, "model.pwm_mode.requested", od::Item::Kind::member)) {
        check(item->datatype == od::DataType::uint32, "model.pwm_mode.requested is a uint32");
        check(item->access == od::Access::rw, "model.pwm_mode.requested is written");
        check(item->values != nullptr && item->values->values.size() == 9, "the PWM modes are a table of 9");
        auto dpwm0 = od::parse_value(*item, "dpwm0");
        check(dpwm0.has_value() && dpwm0->u32() == 3, "dpwm0 reads as 3");
        check(!od::parse_value(*item, "9").has_value(), "9 is no PWM mode");
        check_equal(od::format_value(*item, ExpeditedSdoData(uint32_t(8))), "GDPWM", "the label of the mode 8");
        check_equal(od::format_value(*item, ExpeditedSdoData(uint32_t(12))), "12?", "the mode 12, which no label names");
    }

    if (auto item = expect({0x3161, 0x07}, "motor.R.policy", od::Item::Kind::member)) {
        check(item->datatype == od::DataType::uint8, "motor.R.policy is a uint8 of its own");
        check(item->values != nullptr && item->values->find("on_restart") != nullptr,
              "the policy names on_restart");
    }

    if (auto item = expect({0x3100, 0x08}, "canopen.node_id.apply", od::Item::Kind::member)) {
        check(item->command, "canopen.node_id.apply is a command");
        check(item->access == od::Access::wo, "canopen.node_id.apply is write-only");
    }

    if (auto item = expect({0x4000, 0x17}, "trouble.statuses.ov_dc", od::Item::Kind::member)) {
        check(item->cls == od::Class::status, "the statuses are of the class status");
        check(item->datatype == od::DataType::uint8, "a status is a uint8");
        check(item->values != nullptr && item->values->values.size() == 5, "the severities are a table of 5");
        check_equal(od::format_value(*item, ExpeditedSdoData(uint8_t(3))), "Критическая ошибка",
                    "the label of the severity 3");
    }

    if (auto item = expect({0x2001, 0x00}, "device.name", od::Item::Kind::count)) {
        check(item->datatype == od::DataType::uint8 && item->access == od::Access::constant,
              "the length of a text is a const uint8");
        check(item->array != nullptr && item->array->text, "device.name is a text");
    }

    if (auto item = expect({0x5021, 0x03}, "temperature.Tpwr_sensors[3]", od::Item::Kind::element)) {
        check(item->datatype == od::DataType::float32, "a temperature is a float32");
        check_equal(item->element, 3u, "the element of temperature.Tpwr_sensors[3]");
        check_equal(item->unit.ru, "°C", "the unit of a temperature");
    }

    if (auto item = expect({0x3000, 0x00}, "", od::Item::Kind::highest_subindex)) {
        check(item->datatype == od::DataType::uint8 && item->access == od::Access::constant,
              "the highest subindex is a const uint8");
    }

    check(!description->resolve({0x3000, 0x14}).has_value(), "3000h:14 resolves to nothing");
    check(!description->resolve({0x3161, 0x0A}).has_value(), "3161h:0A resolves to nothing, motor.R has no actual");
    check(!description->resolve({0x1000, 0x00}).has_value(), "1000h:00 resolves to nothing");
}

// 3. The keys of paths.
void check_find(const std::filesystem::path& path, const std::filesystem::path&) {
    auto description = load(path);
    if (!description) {
        return;
    }

    auto expect = [&](std::string_view path, od::Key key) {
        auto found = description->find(path);
        if (!found.has_value()) {
            fail(std::format("{} is not found, expected at {}", path, key_text(key)));
            return;
        }
        check_equal(key_text(*found), key_text(key), std::format("the key of {}", path));
    };

    expect("electrical.Vdc", {0x5010, 0x01});
    expect("settings.save_all_parameters", {0x2101, 0x01});
    expect("motor.R.stored", {0x3161, 0x03});
    expect("temperature.Tpwr_sensors", {0x5021, 0x00});
    expect("temperature.Tpwr_sensors[2]", {0x5021, 0x02});

    if (auto key = description->find("settings.save_all_parameters")) {
        auto item = description->resolve(*key);
        check(item.has_value() && item->command, "settings.save_all_parameters is a command");
        check(item.has_value() && item->signature == 0x65766173u, "settings.save_all_parameters takes \"save\"");
    }

    check(!description->find("motor.R").has_value(), "a record has no key of its own");
    check(!description->find("motor.R.nothing").has_value(), "motor.R.nothing is not found");
    check(!description->find("temperature.Tpwr_sensors[0]").has_value(), "an array has no element 0");
    check(!description->find("electrical.Vdc[1]").has_value(), "an object has no elements");
}

// 4. What the reader refuses and what it skips.
void check_refusals(const std::filesystem::path& path, const std::filesystem::path&) {
    auto const text = read_file(path);
    auto const original = od::parse_description(text);
    if (!original.has_value()) {
        fail("the description does not read: " + original.error());
        return;
    }

    auto refused = [](const std::string& variant, std::string_view what) {
        auto description = od::parse_description(variant);
        check(!description.has_value(), std::format("{} is refused", what));
    };

    auto const vdc = std::string(R"("index":"0x5010","sub":"0x01","group":"electrical","name":"Vdc")");
    refused(replaced(text, vdc, R"("index":"0x5010","sub":"0x01","group":"electrical","name":"Vdd")"),
            "a changed byte of the body");
    refused(replaced(text, R"("version":1)", R"("version":2)"), "the version 2");
    refused(refingerprinted(replaced(text, R"("value_table":3,"layout":0)", R"("value_table":99,"layout":0)")),
            "a reference to a table that is not stated");

    // A kind of line and a key the reader does not know: skipped.
    auto newer = replaced(text, vdc, vdc + R"(,"future_key":"x")");
    newer += "{\"type\":\"future_kind\",\"name\":\"x\"}\n";
    auto described = od::parse_description(refingerprinted(newer));
    if (!described.has_value()) {
        fail("a description with an unknown kind of line and key is refused: " + described.error());
    } else {
        // the counts, after the fingerprint, which the change does change
        auto counts = [](const std::string& summary) { return summary.substr(summary.find(": ") + 2); };
        check_equal(counts(summary(**described)), counts(summary(**original)), "the description read without them");
    }

    // A datatype the reader does not know: the object is there, unavailable.
    auto other = replaced(text, vdc + R"(,"label_en":"DC voltage","label_ru":"Напряжение DC","class":"watch","datatype":"float32")",
                          vdc + R"(,"label_en":"DC voltage","label_ru":"Напряжение DC","class":"watch","datatype":"float64")");
    auto typed = od::parse_description(refingerprinted(other));
    if (!typed.has_value()) {
        fail("a description with an unknown datatype is refused: " + typed.error());
    } else {
        auto item = (*typed)->resolve({0x5010, 0x01});
        check(item.has_value() && !item->available, "electrical.Vdc of an unknown datatype is unavailable");
    }
}

// 5. The firmware's example of frames decodes, for node 1, as od_decode.py
// --candump decodes it.
void check_frames(const std::filesystem::path& path, const std::filesystem::path& expectations) {
    auto description = load(path);
    if (!description) {
        return;
    }

    auto const frames = read_candump(expectations / "pdo_example.candump");
    auto const expected = read_lines(expectations / "pdo_example.expected");
    check_equal(frames.size(), expected.size(), "the count of frames");
    for (size_t i = 0; i < std::min(frames.size(), expected.size()); ++i) {
        auto const line = od::candump_line(*description, 1, frames[i].id, frames[i].data);
        if (line != expected[i]) {
            fail(std::format("frame {}:\n  expected {}\n  got      {}", i + 1, expected[i], line));
        }
    }
}

// 6. The frames of RPDOs in the example that the device takes build back from
// their values byte for byte; values that no table names do not build.
void check_rpdo(const std::filesystem::path& path, const std::filesystem::path& expectations) {
    auto description = load(path);
    if (!description) {
        return;
    }

    size_t taken = 0;
    for (const auto& frame : read_candump(expectations / "pdo_example.candump")) {
        const auto& pdos = description->pdos();
        auto const pdo = std::find_if(pdos.begin(), pdos.end(), [&frame](const od::Pdo& p) {
            return p.direction == od::Direction::rx && p.cob_base.has_value() && *p.cob_base + 1u == frame.id;
        });
        if (pdo == pdos.end() || frame.data.size() < 8) {
            continue;
        }

        auto const decoded = od::decode_pdo(*description, *pdo, frame.data);
        bool const refused = decoded->reserved != 0 ||
                             std::any_of(decoded->signals.begin(), decoded->signals.end(), [](const auto& value) {
                                 return value.signal->value_table.has_value() && value.named == nullptr;
                             });
        if (refused) {
            continue;
        }
        ++taken;

        std::map<std::string, double, std::less<>> values;
        unsigned counter = 0;
        for (const auto& value : decoded->signals) {
            if (value.signal->role == od::Role::counter) {
                counter = static_cast<unsigned>(value.raw);
            } else if (value.signal->value_table.has_value()) {
                values[value.signal->name] = static_cast<double>(value.raw);
            } else {
                values[value.signal->name] = od::physical(*value.signal, value.raw);
            }
        }
        auto const encoded = od::encode_pdo(*description, *pdo, values, counter);
        auto const what = std::format("{:03X}#{}", frame.id, hex_of(frame.data));
        if (!encoded.has_value()) {
            fail(std::format("{} does not build back: {}", what, encoded.error()));
            continue;
        }
        auto const built = std::vector<uint8_t>(encoded->payload.begin(), encoded->payload.end());
        check_equal(hex_of(built), hex_of(frame.data), std::format("{} built back", what));
        check(encoded->saturated.empty(), std::format("{} builds without saturation", what));
    }
    check_equal(taken, 5u, "the frames of RPDOs that the device takes");

    const auto* drive_command = description->pdo("drive_command");
    const auto* model_command = description->pdo("model_command");
    if (drive_command == nullptr || model_command == nullptr) {
        fail("the description has no drive_command or model_command");
        return;
    }
    std::map<std::string, double, std::less<>> drive = {
            {"emergency", 0}, {"power", 1}, {"start", 0}, {"control_mode", 0}, {"torque_ref", 0}, {"speed_ref", 0}};
    std::map<std::string, double, std::less<>> model = {
            {"angle_ref", 0}, {"current_ref", 0}, {"voltage_ref", 0}, {"model_mode", 0}};

    auto with = [](auto values, std::string_view name, double value) {
        values[std::string(name)] = value;
        return values;
    };
    check(!od::encode_pdo(*description, *drive_command, with(drive, "control_mode", 3), 0).has_value(),
          "the control mode 3, which no label names, does not build");
    check(!od::encode_pdo(*description, *model_command, with(model, "model_mode", 6), 0).has_value(),
          "the model mode 6 does not build");
    check(!od::encode_pdo(*description, *model_command, with(model, "model_mode", 7), 0).has_value(),
          "the model mode 7 does not build");

    auto saturated = od::encode_pdo(*description, *drive_command, with(drive, "torque_ref", 5.0), 0);
    check(saturated.has_value() && saturated->saturated == std::vector<std::string>{"drive_command.torque_ref"},
          "a torque of 5 pu saturates");
    if (saturated.has_value()) {
        check_equal(std::format("{:02X}{:02X}", saturated->payload[3], saturated->payload[2]), "7FFF",
                    "the saturated torque");
    }

    auto unfilled = drive;
    unfilled.erase("speed_ref");
    check(!od::encode_pdo(*description, *drive_command, unfilled, 0).has_value(),
          "a value signal left unfilled does not build");
    check(!od::encode_pdo(*description, *drive_command, with(drive, "nothing", 1), 0).has_value(),
          "a value of no signal does not build");
    check(od::encoding_refusal(*drive_command, {"emergency", "power", "start", "control_mode", "torque_ref",
                                                "speed_ref"}) == std::nullopt,
          "drive_command builds from its six signals");
}

} // namespace

int main(int argc, char** argv) {
    if (argc != 4) {
        say("usage: od-test <case> <description> <directory of expectations>");
        return 2;
    }

    using Case = std::function<void(const std::filesystem::path&, const std::filesystem::path&)>;
    const std::map<std::string_view, Case> cases = {
            {"description", check_description},
            {"resolve", check_resolve},
            {"find", check_find},
            {"refusals", check_refusals},
            {"frames", check_frames},
            {"rpdo", check_rpdo}};

    auto const name = std::string_view(argv[1]);
    auto const found = cases.find(name);
    if (found == cases.end()) {
        say(std::format("od-test: no case {}", name));
        return 2;
    }
    found->second(argv[2], argv[3]);
    return (failures == 0) ? 0 : 1;
}
