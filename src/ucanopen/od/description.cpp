#include "description.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <charconv>
#include <fstream>
#include <iterator>
#include <limits>
#include <set>
#include <utility>

// Boost.JSON without its compiled library: its sources compile here, in this
// translation unit and in no other.
#include <boost/json/src.hpp>


namespace ucanopen::od {


namespace {


namespace json = boost::json;


constexpr std::string_view format_name = "emb-canopen-od";
constexpr int64_t format_version = 1;

// Far above anything a description numbers, and still a size_t on any host.
constexpr int64_t max_number = 0xFFFF;

const Text highest_subindex_label{"Наибольший подындекс", "Highest subindex"};


// Why the reading stopped. The reader raises it wherever a rule breaks, and
// parse_description returns its message.
struct Refusal {
    std::string message;
};


std::string at_line(size_t line, std::string_view message) {
    return "строка " + std::to_string(line) + ": " + std::string(message);
}


[[noreturn]] void refuse(size_t line, std::string_view message) {
    throw Refusal{at_line(line, message)};
}


std::string in_quotes(std::string_view text) {
    return "«" + std::string(text) + "»";
}


std::string hex(uint32_t value, size_t digits) {
    std::array<char, 8> buf{};
    auto result = std::to_chars(buf.data(), buf.data() + buf.size(), value, 16);
    std::string ret(buf.data(), result.ptr);
    std::transform(ret.begin(), ret.end(), ret.begin(), [](unsigned char c) { return std::toupper(c); });
    if (ret.size() < digits) {
        ret.insert(0, digits - ret.size(), '0');
    }
    return ret;
}


std::string key_text(Key key) {
    return hex(key.index, 4) + "h:" + hex(key.subindex, 2);
}


std::string path_of(const std::string& group, const std::string& name) {
    return group + "." + name;
}


// A segment of a path: a letter, then letters, digits and underscores.
bool is_segment(std::string_view text) {
    if (text.empty() || !std::isalpha(static_cast<unsigned char>(text[0]))) {
        return false;
    }
    return std::all_of(text.begin(), text.end(), [](unsigned char c) { return std::isalnum(c) || c == '_'; });
}


template<typename Enum, size_t N>
Enum named(std::string_view text, const std::array<std::pair<std::string_view, Enum>, N>& names) {
    for (const auto& [name, value] : names) {
        if (name == text) {
            return value;
        }
    }
    return Enum::unknown;
}


DataType datatype_of(std::string_view text) {
    static constexpr std::array<std::pair<std::string_view, DataType>, 8> names = {{
            {"boolean", DataType::boolean},
            {"int8", DataType::int8},
            {"int16", DataType::int16},
            {"int32", DataType::int32},
            {"uint8", DataType::uint8},
            {"uint16", DataType::uint16},
            {"uint32", DataType::uint32},
            {"float32", DataType::float32}}};
    return named(text, names);
}


Access access_of(std::string_view text) {
    static constexpr std::array<std::pair<std::string_view, Access>, 4> names = {{
            {"rw", Access::rw},
            {"ro", Access::ro},
            {"wo", Access::wo},
            {"const", Access::constant}}};
    return named(text, names);
}


Class class_of(std::string_view text) {
    static constexpr std::array<std::pair<std::string_view, Class>, 5> names = {{
            {"about", Class::about},
            {"control", Class::control},
            {"parameters", Class::parameters},
            {"status", Class::status},
            {"watch", Class::watch}}};
    return named(text, names);
}


Direction direction_of(std::string_view text) {
    static constexpr std::array<std::pair<std::string_view, Direction>, 2> names = {{
            {"tx", Direction::tx},
            {"rx", Direction::rx}}};
    return named(text, names);
}


Mapping mapping_of(std::string_view text) {
    static constexpr std::array<std::pair<std::string_view, Mapping>, 2> names = {{
            {"signals", Mapping::signals},
            {"sam", Mapping::sam}}};
    return named(text, names);
}


Role role_of(std::string_view text) {
    static constexpr std::array<std::pair<std::string_view, Role>, 2> names = {{
            {"value", Role::value},
            {"counter", Role::counter}}};
    return named(text, names);
}


Encoding encoding_of(std::string_view text) {
    static constexpr std::array<std::pair<std::string_view, Encoding>, 2> names = {{
            {"unsigned", Encoding::unsigned_int},
            {"signed", Encoding::signed_int}}};
    return named(text, names);
}


bool known(const Item& item) {
    return item.datatype != DataType::unknown && item.access != Access::unknown && item.cls != Class::unknown;
}


// A line of the description: its object of JSON and its number, which every
// refusal names.
class Line {
public:
    Line(const json::object& object, size_t number)
            : _object(object)
            , _number(number) {}

    size_t number() const { return _number; }

    [[noreturn]] void refuse(std::string_view message) const {
        od::refuse(_number, message);
    }

    std::optional<std::string> optional_text(std::string_view key) const {
        const auto* value = _object.if_contains(key);
        if (value == nullptr) {
            return std::nullopt;
        }
        if (!value->is_string()) {
            refuse(in_quotes(key) + " — не строка");
        }
        const auto& text = value->get_string();
        return std::string(text.data(), text.size());
    }

    std::string text(std::string_view key) const {
        auto value = optional_text(key);
        if (!value.has_value()) {
            refuse("нет ключа " + in_quotes(key));
        }
        return *value;
    }

    // A name that is a segment of a path.
    std::string segment(std::string_view key) const {
        auto value = text(key);
        if (!is_segment(value)) {
            refuse(in_quotes(key) + " — не имя: " + in_quotes(value));
        }
        return value;
    }

    std::optional<int64_t> optional_integer(std::string_view key, int64_t min, int64_t max) const {
        const auto* value = _object.if_contains(key);
        if (value == nullptr) {
            return std::nullopt;
        }
        std::optional<int64_t> number;
        if (value->is_int64()) {
            number = value->get_int64();
        } else if (value->is_uint64() && value->get_uint64() <= uint64_t(std::numeric_limits<int64_t>::max())) {
            number = int64_t(value->get_uint64());
        }
        if (!number.has_value() || *number < min || *number > max) {
            refuse(in_quotes(key) + " — не целое число от " + std::to_string(min) + " до " + std::to_string(max));
        }
        return number;
    }

    int64_t integer(std::string_view key, int64_t min, int64_t max) const {
        auto value = optional_integer(key, min, max);
        if (!value.has_value()) {
            refuse("нет ключа " + in_quotes(key));
        }
        return *value;
    }

    // A hexadecimal number as the format writes one, "0x" and the digits.
    std::optional<uint32_t> optional_hexadecimal(std::string_view key, uint32_t max) const {
        auto value = optional_text(key);
        if (!value.has_value()) {
            return std::nullopt;
        }
        if (value->size() <= 2 || !value->starts_with("0x")) {
            refuse(in_quotes(key) + " — не число вида 0x…: " + in_quotes(*value));
        }
        uint32_t number = 0;
        const char* end = value->data() + value->size();
        auto result = std::from_chars(value->data() + 2, end, number, 16);
        if (result.ec != std::errc() || result.ptr != end || number > max) {
            refuse(in_quotes(key) + " — не число вида 0x… до 0x" + hex(max, 0) + ": " + in_quotes(*value));
        }
        return number;
    }

    uint32_t hexadecimal(std::string_view key, uint32_t max) const {
        auto value = optional_hexadecimal(key, max);
        if (!value.has_value()) {
            refuse("нет ключа " + in_quotes(key));
        }
        return *value;
    }

    bool flag(std::string_view key) const {
        const auto* value = _object.if_contains(key);
        if (value == nullptr) {
            return false;
        }
        if (!value->is_bool()) {
            refuse(in_quotes(key) + " — не true");
        }
        return value->get_bool();
    }

    Text label() const {
        return {optional_text("label_ru").value_or(""), optional_text("label_en").value_or("")};
    }

    Text unit() const {
        return {optional_text("unit_ru").value_or(""), optional_text("unit_en").value_or("")};
    }
private:
    const json::object& _object;
    size_t _number;
};


} // namespace


// Reads the lines of a description into it one by one, and links what they
// refer to once all of them are read, so that their order does not matter.
class DescriptionReader {
public:
    explicit DescriptionReader(std::string_view text)
            : _text(text)
            , _description(new Description()) {}

    std::shared_ptr<const Description> read();
private:
    std::string_view _text;
    std::shared_ptr<Description> _description;

    std::map<size_t, std::vector<Value>> _tables;
    std::map<size_t, std::vector<Field>> _layouts;
    std::map<std::string, std::vector<std::pair<size_t, Signal>>> _signals; // by PDO, with their lines
    std::set<std::string, std::less<>> _owner_paths; // of objects, arrays and records

    // What the lines refer to, with the lines, to check once all are read.
    std::vector<std::pair<size_t, size_t>> _table_refs;
    std::vector<std::pair<size_t, size_t>> _layout_refs;
    std::vector<std::pair<size_t, std::string>> _group_refs;

    void _read_header(std::string_view text);
    void _read_line(std::string_view text, size_t number);
    void _read_group(const Line& line);
    void _read_value(const Line& line);
    void _read_field(const Line& line);
    void _read_object(const Line& line);
    void _read_array(const Line& line);
    void _read_record(const Line& line);
    void _read_pdo(const Line& line);
    void _read_signal(const Line& line);
    void _link();

    std::optional<size_t> _table_ref(const Line& line);
    std::string _group_ref(const Line& line);
    void _claim_key(const Line& line, Key key) const;
    void _claim_index(const Line& line, uint16_t index) const;
    void _claim_path(const Line& line, const std::string& path);
};


std::shared_ptr<const Description> DescriptionReader::read() {
    if (_text.empty()) {
        throw Refusal{"описание пусто"};
    }
    if (_text.starts_with("\xEF\xBB\xBF")) {
        throw Refusal{"описание начинается с метки порядка байтов"};
    }

    auto const header_end = _text.find('\n');
    _read_header(_text.substr(0, header_end));

    // the fingerprint is the hash of every byte after the line of the header
    auto const body = (header_end == std::string_view::npos) ? std::string_view{} : _text.substr(header_end + 1);
    auto const hash = fnv1a(body);
    if (hash != _description->_fingerprint) {
        throw Refusal{"отпечаток заголовка 0x" + hex(_description->_fingerprint, 8) + " не равен хэшу тела 0x" +
                      hex(hash, 8) + ": описание правлено или повреждено"};
    }

    size_t number = 1;
    size_t begin = 0;
    while (begin < body.size()) {
        auto end = body.find('\n', begin);
        if (end == std::string_view::npos) {
            end = body.size();
        }
        _read_line(body.substr(begin, end - begin), ++number);
        begin = end + 1;
    }

    _link();
    return _description;
}


void DescriptionReader::_read_header(std::string_view text) {
    boost::system::error_code ec;
    auto value = json::parse(text, ec);
    if (ec || !value.is_object()) {
        refuse(1, "не объект JSON");
    }
    Line line(value.as_object(), 1);
    if (line.optional_text("type") != "header") {
        line.refuse("первая строка — не заголовок описания");
    }
    auto const format = line.text("format");
    if (format != format_name) {
        line.refuse("формат " + in_quotes(format) + ", а не " + in_quotes(format_name));
    }
    auto const version = line.integer("version", 0, max_number);
    if (version != format_version) {
        line.refuse("версия формата " + std::to_string(version) + ", а программа читает версию " +
                    std::to_string(format_version));
    }
    _description->_device = line.text("device");
    _description->_fingerprint = line.hexadecimal("fingerprint", 0xFFFFFFFF);
}


void DescriptionReader::_read_line(std::string_view text, size_t number) {
    if (text.empty()) {
        return;
    }
    boost::system::error_code ec;
    auto value = json::parse(text, ec);
    if (ec || !value.is_object()) {
        refuse(number, "не объект JSON");
    }
    Line line(value.as_object(), number);
    auto const type = line.text("type");
    if (type == "group") {
        _read_group(line);
    } else if (type == "value") {
        _read_value(line);
    } else if (type == "field") {
        _read_field(line);
    } else if (type == "object") {
        _read_object(line);
    } else if (type == "array") {
        _read_array(line);
    } else if (type == "record") {
        _read_record(line);
    } else if (type == "pdo") {
        _read_pdo(line);
    } else if (type == "signal") {
        _read_signal(line);
    } else if (type == "header") {
        line.refuse("второй заголовок");
    }
    // A kind of line this program does not know is skipped, as the format asks.
}


void DescriptionReader::_read_group(const Line& line) {
    Group group{line.segment("name"), line.label()};
    if (_description->group(group.name) != nullptr) {
        line.refuse("группа " + in_quotes(group.name) + " уже описана");
    }
    _description->_groups.push_back(std::move(group));
}


void DescriptionReader::_read_value(const Line& line) {
    auto const table = static_cast<size_t>(line.integer("value_table", 0, max_number));
    Value value{line.integer("value", std::numeric_limits<int64_t>::min(), std::numeric_limits<int64_t>::max()),
                line.segment("name"),
                line.label()};
    auto& values = _tables[table];
    for (const auto& other : values) {
        if (other.value == value.value || other.name == value.name) {
            line.refuse("значение " + std::to_string(value.value) + " " + in_quotes(value.name) +
                        " уже есть в таблице " + std::to_string(table));
        }
    }
    values.push_back(std::move(value));
}


void DescriptionReader::_read_field(const Line& line) {
    auto const layout = static_cast<size_t>(line.integer("layout", 0, max_number));
    Field field;
    field.subindex = static_cast<uint8_t>(line.hexadecimal("sub", 0xFF));
    field.name = line.segment("name");
    field.label = line.label();
    field.inherits = line.flag("inherits");
    if (auto datatype = line.optional_text("datatype"); datatype.has_value()) {
        field.datatype = datatype_of(*datatype);
    } else if (field.inherits) {
        field.datatype = DataType::unknown;
    } else {
        line.refuse("нет ключа «datatype» у члена, который не наследует значение");
    }
    field.access = access_of(line.text("access"));
    field.value_table = _table_ref(line);
    field.command = line.flag("command");
    field.signature = line.optional_hexadecimal("signature", 0xFFFFFFFF);

    auto& fields = _layouts[layout];
    for (const auto& other : fields) {
        if (other.subindex == field.subindex || other.name == field.name) {
            line.refuse("член " + hex(field.subindex, 2) + "h " + in_quotes(field.name) + " уже есть в раскладке " +
                        std::to_string(layout));
        }
    }
    fields.push_back(std::move(field));
}


void DescriptionReader::_read_object(const Line& line) {
    Object object;
    object.key = {static_cast<uint16_t>(line.hexadecimal("index", 0xFFFF)),
                  static_cast<uint8_t>(line.hexadecimal("sub", 0xFF))};
    object.group = _group_ref(line);
    object.name = line.segment("name");
    object.label = line.label();
    object.cls = class_of(line.text("class"));
    object.datatype = datatype_of(line.text("datatype"));
    object.access = access_of(line.text("access"));
    object.unit = line.unit();
    object.value_table = _table_ref(line);
    object.command = line.flag("command");
    object.signature = line.optional_hexadecimal("signature", 0xFFFFFFFF);

    _claim_key(line, object.key);
    auto path = path_of(object.group, object.name);
    _claim_path(line, path);
    _description->_paths.emplace(std::move(path), object.key);
    _description->_objects.emplace(object.key, std::move(object));
}


void DescriptionReader::_read_array(const Line& line) {
    Array array;
    array.index = static_cast<uint16_t>(line.hexadecimal("index", 0xFFFF));
    array.group = _group_ref(line);
    array.name = line.segment("name");
    array.label = line.label();
    array.cls = class_of(line.text("class"));
    array.datatype = datatype_of(line.text("datatype"));
    array.access = access_of(line.text("access"));
    array.unit = line.unit();
    array.value_table = _table_ref(line);
    array.text = line.flag("text");

    _claim_index(line, array.index);
    auto path = path_of(array.group, array.name);
    _claim_path(line, path);
    _description->_paths.emplace(std::move(path), Key{array.index, 0x00});
    _description->_arrays.emplace(array.index, std::move(array));
}


void DescriptionReader::_read_record(const Line& line) {
    Record record;
    record.index = static_cast<uint16_t>(line.hexadecimal("index", 0xFFFF));
    record.group = _group_ref(line);
    record.name = line.segment("name");
    record.label = line.label();
    record.cls = class_of(line.text("class"));
    record.datatype = datatype_of(line.optional_text("datatype").value_or(""));
    record.unit = line.unit();
    record.value_table = _table_ref(line);
    record.layout = static_cast<size_t>(line.integer("layout", 0, max_number));
    _layout_refs.emplace_back(line.number(), record.layout);

    _claim_index(line, record.index);
    _claim_path(line, path_of(record.group, record.name));
    _description->_records.emplace(record.index, std::move(record));
}


void DescriptionReader::_read_pdo(const Line& line) {
    Pdo pdo;
    pdo.name = line.segment("name");
    pdo.label = line.label();
    pdo.direction = direction_of(line.text("direction"));
    pdo.number = static_cast<unsigned>(line.integer("number", 1, 512));
    pdo.mapping = mapping_of(line.text("mapping"));
    if (auto cob_id = line.optional_hexadecimal("cob_id", 0x7FF); cob_id.has_value()) {
        pdo.cob_id = static_cast<uint16_t>(*cob_id);
    }
    if (auto cob_base = line.optional_hexadecimal("cob_base", 0x7FF); cob_base.has_value()) {
        pdo.cob_base = static_cast<uint16_t>(*cob_base);
    }
    pdo.period_ms = static_cast<unsigned>(line.optional_integer("period_ms", 0, max_number).value_or(0));
    pdo.timeout_ms = static_cast<unsigned>(line.optional_integer("timeout_ms", 0, max_number).value_or(0));

    if (_description->pdo(pdo.name) != nullptr) {
        line.refuse("PDO " + in_quotes(pdo.name) + " уже описан");
    }
    _description->_pdos.push_back(std::move(pdo));
}


void DescriptionReader::_read_signal(const Line& line) {
    auto pdo = line.segment("pdo");
    Signal signal;
    signal.start = static_cast<unsigned>(line.integer("start", 0, 63));
    signal.width = static_cast<unsigned>(line.integer("width", 1, 64));
    if (signal.start + signal.width > 64) {
        line.refuse("сигнал выходит за 64 бита кадра");
    }
    signal.role = role_of(line.text("role"));
    if (auto name = line.optional_text("name"); name.has_value()) {
        if (!is_segment(*name)) {
            line.refuse("«name» — не имя: " + in_quotes(*name));
        }
        signal.name = std::move(*name);
    }
    signal.label = line.label();
    signal.encoding = encoding_of(line.text("encoding"));
    if (auto scale = line.optional_text("scale"); scale.has_value()) {
        // "num/den", a positive fraction
        auto const slash = scale->find('/');
        int64_t num = 0;
        int64_t den = 0;
        bool parsed = slash != std::string::npos;
        if (parsed) {
            const char* end = scale->data() + scale->size();
            auto num_result = std::from_chars(scale->data(), scale->data() + slash, num);
            auto den_result = std::from_chars(scale->data() + slash + 1, end, den);
            parsed = num_result.ec == std::errc() && num_result.ptr == scale->data() + slash &&
                     den_result.ec == std::errc() && den_result.ptr == end && num > 0 && den > 0;
        }
        if (!parsed) {
            line.refuse("«scale» — не дробь вида num/den: " + in_quotes(*scale));
        }
        signal.scale_num = num;
        signal.scale_den = den;
    }
    signal.unit = line.unit();
    signal.value_table = _table_ref(line);
    _signals[pdo].emplace_back(line.number(), std::move(signal));
}


void DescriptionReader::_link() {
    auto& description = *_description;

    for (const auto& [line, table] : _table_refs) {
        if (!_tables.contains(table)) {
            refuse(line, "таблица значений " + std::to_string(table) + " не описана");
        }
    }
    if (!_tables.empty()) {
        description._value_tables.resize(_tables.rbegin()->first + 1);
        for (auto& [number, values] : _tables) {
            std::sort(values.begin(), values.end(), [](const Value& a, const Value& b) { return a.value < b.value; });
            description._value_tables[number].values = std::move(values);
        }
    }

    for (const auto& [line, layout] : _layout_refs) {
        if (!_layouts.contains(layout)) {
            refuse(line, "раскладка " + std::to_string(layout) + " не описана");
        }
    }
    if (!_layouts.empty()) {
        description._layouts.resize(_layouts.rbegin()->first + 1);
        for (auto& [number, fields] : _layouts) {
            std::sort(fields.begin(), fields.end(), [](const Field& a, const Field& b) {
                return a.subindex < b.subindex;
            });
            description._layouts[number].fields = std::move(fields);
        }
    }

    for (const auto& [line, group] : _group_refs) {
        if (description.group(group) == nullptr) {
            refuse(line, "группа " + in_quotes(group) + " не описана");
        }
    }

    for (auto& [name, signals] : _signals) {
        auto pdo = std::find_if(description._pdos.begin(), description._pdos.end(), [&name](const Pdo& p) {
            return p.name == name;
        });
        if (pdo == description._pdos.end()) {
            refuse(signals.front().first, "PDO " + in_quotes(name) + " не описан");
        }
        for (auto& [line, signal] : signals) {
            pdo->signals.push_back(std::move(signal));
        }
        std::sort(pdo->signals.begin(), pdo->signals.end(), [](const Signal& a, const Signal& b) {
            return a.start < b.start;
        });
    }

    // The members of the records by their paths, now that the layouts are known.
    for (const auto& [index, record] : description._records) {
        for (const auto& field : description._layouts[record.layout].fields) {
            description._paths.emplace(path_of(record.group, record.name) + "." + field.name,
                                       Key{index, field.subindex});
        }
    }
}


std::optional<size_t> DescriptionReader::_table_ref(const Line& line) {
    auto table = line.optional_integer("value_table", 0, max_number);
    if (!table.has_value()) {
        return std::nullopt;
    }
    _table_refs.emplace_back(line.number(), static_cast<size_t>(*table));
    return static_cast<size_t>(*table);
}


std::string DescriptionReader::_group_ref(const Line& line) {
    auto group = line.segment("group");
    _group_refs.emplace_back(line.number(), group);
    return group;
}


void DescriptionReader::_claim_key(const Line& line, Key key) const {
    if (_description->_objects.contains(key) || _description->_arrays.contains(key.index) ||
        _description->_records.contains(key.index)) {
        line.refuse("ключ " + key_text(key) + " уже описан");
    }
}


void DescriptionReader::_claim_index(const Line& line, uint16_t index) const {
    auto const object = _description->_objects.lower_bound(Key{index, 0x00});
    bool const has_objects = (object != _description->_objects.end()) && (object->first.index == index);
    if (has_objects || _description->_arrays.contains(index) || _description->_records.contains(index)) {
        line.refuse("индекс " + hex(index, 4) + "h уже описан");
    }
}


void DescriptionReader::_claim_path(const Line& line, const std::string& path) {
    if (!_owner_paths.insert(path).second) {
        line.refuse("путь " + in_quotes(path) + " уже описан");
    }
}


//----------------------------------------------------------------------------------------------------------------------
const Value* ValueTable::find(int64_t value) const {
    auto iter = std::lower_bound(values.begin(), values.end(), value, [](const Value& v, int64_t x) {
        return v.value < x;
    });
    if (iter == values.end() || iter->value != value) {
        return nullptr;
    }
    return &*iter;
}


const Value* ValueTable::find(std::string_view name) const {
    auto iter = std::find_if(values.begin(), values.end(), [name](const Value& v) { return v.name == name; });
    return (iter == values.end()) ? nullptr : &*iter;
}


const Field* Layout::find(uint8_t subindex) const {
    auto iter = std::find_if(fields.begin(), fields.end(), [subindex](const Field& f) {
        return f.subindex == subindex;
    });
    return (iter == fields.end()) ? nullptr : &*iter;
}


//----------------------------------------------------------------------------------------------------------------------
const Group* Description::group(std::string_view name) const {
    auto iter = std::find_if(_groups.begin(), _groups.end(), [name](const Group& g) { return g.name == name; });
    return (iter == _groups.end()) ? nullptr : &*iter;
}


const Pdo* Description::pdo(std::string_view name) const {
    auto iter = std::find_if(_pdos.begin(), _pdos.end(), [name](const Pdo& p) { return p.name == name; });
    return (iter == _pdos.end()) ? nullptr : &*iter;
}


const ValueTable* Description::value_table(std::optional<size_t> index) const {
    if (!index.has_value() || *index >= _value_tables.size()) {
        return nullptr;
    }
    return &_value_tables[*index];
}


std::optional<Item> Description::resolve(Key key) const {
    Item item;
    item.key = key;

    if (auto iter = _objects.find(key); iter != _objects.end()) {
        const auto& object = iter->second;
        item.kind = Item::Kind::object;
        item.path = path_of(object.group, object.name);
        item.label = object.label;
        item.unit = object.unit;
        item.cls = object.cls;
        item.datatype = object.datatype;
        item.access = object.access;
        item.values = value_table(object.value_table);
        item.command = object.command;
        item.signature = object.signature;
        item.object = &object;
        item.available = known(item);
        return item;
    }

    if (auto iter = _records.find(key.index); iter != _records.end()) {
        const auto& record = iter->second;
        const auto* field = _layouts[record.layout].find(key.subindex);
        if (field == nullptr) {
            return std::nullopt;
        }
        item.kind = Item::Kind::member;
        item.path = path_of(record.group, record.name) + "." + field->name;
        item.label = field->label;
        item.cls = record.cls;
        if (field->inherits) {
            item.datatype = record.datatype;
            item.unit = record.unit;
            item.values = value_table(record.value_table);
        } else {
            item.datatype = field->datatype;
            item.values = value_table(field->value_table);
        }
        item.access = field->access;
        item.command = field->command;
        item.signature = field->signature;
        item.record = &record;
        item.field = field;
        item.available = known(item);
        return item;
    }

    if (auto iter = _arrays.find(key.index); iter != _arrays.end()) {
        const auto& array = iter->second;
        item.label = array.label;
        item.cls = array.cls;
        item.array = &array;
        if (key.subindex == 0x00) {
            item.kind = Item::Kind::count;
            item.path = path_of(array.group, array.name);
            item.datatype = DataType::uint8;
            item.access = Access::constant;
        } else {
            item.kind = Item::Kind::element;
            item.path = path_of(array.group, array.name) + "[" + std::to_string(key.subindex) + "]";
            item.datatype = array.datatype;
            item.access = array.access;
            item.unit = array.unit;
            item.values = value_table(array.value_table);
            item.element = key.subindex;
        }
        item.available = known(item);
        return item;
    }

    if (key.subindex == 0x00) {
        auto object = _objects.lower_bound(key);
        if (object != _objects.end() && object->first.index == key.index) {
            item.kind = Item::Kind::highest_subindex;
            item.label = highest_subindex_label;
            item.cls = object->second.cls;
            item.datatype = DataType::uint8;
            item.access = Access::constant;
            item.available = known(item);
            return item;
        }
    }

    return std::nullopt;
}


std::optional<Key> Description::find(std::string_view path) const {
    if (path.ends_with(']')) {
        auto const open = path.rfind('[');
        if (open == std::string_view::npos) {
            return std::nullopt;
        }
        unsigned element = 0;
        auto const digits = path.substr(open + 1, path.size() - open - 2);
        auto result = std::from_chars(digits.data(), digits.data() + digits.size(), element);
        if (digits.empty() || result.ec != std::errc() || result.ptr != digits.data() + digits.size() ||
            element == 0 || element > 0xFF) {
            return std::nullopt;
        }
        auto array = find(path.substr(0, open));
        if (!array.has_value() || array->subindex != 0x00 || !_arrays.contains(array->index)) {
            return std::nullopt;
        }
        return Key{array->index, static_cast<uint8_t>(element)};
    }

    auto iter = _paths.find(path);
    if (iter == _paths.end()) {
        return std::nullopt;
    }
    return iter->second;
}


//----------------------------------------------------------------------------------------------------------------------
std::expected<std::shared_ptr<const Description>, std::string> parse_description(std::string_view text) {
    try {
        return DescriptionReader(text).read();
    } catch (const Refusal& refusal) {
        return std::unexpected(refusal.message);
    }
}


std::expected<std::shared_ptr<const Description>, std::string> load(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return std::unexpected("не удалось открыть файл");
    }
    std::string text((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    if (file.bad()) {
        return std::unexpected("ошибка чтения файла");
    }
    return parse_description(text);
}


uint32_t fnv1a(std::string_view data) {
    uint32_t hash = 0x811C9DC5;
    for (unsigned char byte : data) {
        hash = (hash ^ byte) * 0x01000193;
    }
    return hash;
}


} // namespace ucanopen::od
