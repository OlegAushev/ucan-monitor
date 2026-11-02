#pragma once


#include <cstddef>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>


// The description of a device's object dictionary: the .od.jsonl that a
// firmware built on emblib extracts from its ELF, an object of JSON per line
// (emblib, docs/canopen-od-algorithm.ru.md, section 3.9). It names every
// object, array and record by its path, with labels and units, lays out the
// members of the records, lists the tables of values and states the PDOs with
// their signals. The host reads the device and decodes its frames by it;
// tools/od_decode.py of emblib is the reference reader of the format.
//
// A description is read once and never changes: the threads of the program
// share it without locks.
namespace ucanopen::od {


enum class DataType { boolean, int8, int16, int32, uint8, uint16, uint32, float32, unknown };
enum class Access { rw, ro, wo, constant, unknown };
enum class Class { about, control, parameters, status, watch, unknown };


// A text for people in each language: a label, or the symbol of a unit.
struct Text {
    std::string ru;
    std::string en;

    bool empty() const { return ru.empty() && en.empty(); }
};


struct Key {
    uint16_t index;
    uint8_t subindex;

    friend auto operator<=>(const Key&, const Key&) = default;
};


// A value of a table, named. A table is closed: a value it does not name is
// not one the device takes.
struct Value {
    int64_t value;
    std::string name;
    Text label;
};


struct ValueTable {
    std::vector<Value> values; // by value

    const Value* find(int64_t value) const;
    const Value* find(std::string_view name) const;
};


struct Group {
    std::string name;
    Text label;
};


// A member of a layout, which the records of one kind share. A member that
// inherits carries the value of its record and takes the type, the unit and
// the table of values of the record.
struct Field {
    uint8_t subindex;
    std::string name;
    Text label;
    DataType datatype; // unknown when it inherits
    Access access;
    std::optional<size_t> value_table;
    bool command;
    std::optional<uint32_t> signature;
    bool inherits;
};


struct Layout {
    std::vector<Field> fields; // by subindex

    const Field* find(uint8_t subindex) const;
};


struct Object {
    Key key;
    std::string group;
    std::string name;
    Text label;
    Class cls;
    DataType datatype;
    Access access;
    Text unit;
    std::optional<size_t> value_table;
    bool command;
    std::optional<uint32_t> signature; // the word a command takes
};


// Its subindex 00h holds the count of elements, which only the device knows:
// the description does not state it.
struct Array {
    uint16_t index;
    std::string group;
    std::string name;
    Text label;
    Class cls;
    DataType datatype; // of the elements
    Access access;     // of the elements
    Text unit;
    std::optional<size_t> value_table;
    bool text; // the bytes of a text in UTF-8
};


struct Record {
    uint16_t index;
    std::string group;
    std::string name;
    Text label;
    Class cls;
    DataType datatype; // of the members that inherit, unknown if none does
    Text unit;
    std::optional<size_t> value_table;
    size_t layout;
};


enum class Direction { tx, rx, unknown };
enum class Mapping { signals, sam, unknown };
enum class Role { value, counter, unknown };
enum class Encoding { unsigned_int, signed_int, unknown };


struct Signal {
    unsigned start; // the first bit of the frame, bit 8j of byte j
    unsigned width;
    Role role;
    std::string name; // empty for a counter
    Text label;
    Encoding encoding;
    int64_t scale_num{1}; // a raw unit is scale_num / scale_den physical ones
    int64_t scale_den{1};
    Text unit;
    std::optional<size_t> value_table;
};


struct Pdo {
    std::string name;
    Text label;
    Direction direction;
    unsigned number;
    Mapping mapping; // sam: an MPDO in source address mode, without signals
    std::optional<uint16_t> cob_id;   // a custom COB-ID
    std::optional<uint16_t> cob_base; // or the base of the predefined one, to add the node-ID to
    unsigned period_ms{0};            // of a TPDO
    unsigned timeout_ms{0};           // of an RPDO, 0 if the device does not watch it
    std::vector<Signal> signals;      // by first bit
};


// What a key of the dictionary stands for, all a host needs to read, write
// and show it.
struct Item {
    enum class Kind {
        object,
        member,          // of a record
        count,           // of an array, at 00h
        element,         // of an array
        highest_subindex // 00h of an index of objects, which the dictionary adds
    };

    Key key;
    Kind kind;
    std::string path; // electrical.Vdc, motor.R.applied, temperature.Tpwr_sensors[2]; empty at a highest subindex
    Text label;
    Text unit;
    Class cls;
    DataType datatype;
    Access access;
    const ValueTable* values{nullptr};
    bool command{false};
    std::optional<uint32_t> signature;
    bool available{true}; // false: of a datatype, class or access this host does not know
    const Object* object{nullptr};
    const Array* array{nullptr};
    const Record* record{nullptr};
    const Field* field{nullptr};
    unsigned element{0}; // of an array, from 1
};


class Description {
    friend class DescriptionReader;
public:
    const std::string& device() const { return _device; }
    uint32_t fingerprint() const { return _fingerprint; }

    const std::vector<Group>& groups() const { return _groups; }
    const std::vector<ValueTable>& value_tables() const { return _value_tables; }
    const std::vector<Layout>& layouts() const { return _layouts; }
    const std::map<Key, Object>& objects() const { return _objects; }
    const std::map<uint16_t, Array>& arrays() const { return _arrays; }
    const std::map<uint16_t, Record>& records() const { return _records; }
    const std::vector<Pdo>& pdos() const { return _pdos; }

    const Group* group(std::string_view name) const;
    const Pdo* pdo(std::string_view name) const;
    const ValueTable* value_table(std::optional<size_t> index) const;

    // Whatever stands at `key`: an object, a member of a record, the count or
    // an element of an array, or the highest subindex of an index of objects.
    std::optional<Item> resolve(Key key) const;

    // The key of an object, a member or an array by its path:
    // electrical.Vdc, motor.R.stored; an array gives its count, and
    // temperature.Tpwr_sensors[2] its element.
    std::optional<Key> find(std::string_view path) const;
private:
    Description() = default;

    std::string _device;
    uint32_t _fingerprint{0};
    std::vector<Group> _groups;
    std::vector<ValueTable> _value_tables;
    std::vector<Layout> _layouts;
    std::map<Key, Object> _objects;
    std::map<uint16_t, Array> _arrays;
    std::map<uint16_t, Record> _records;
    std::vector<Pdo> _pdos;
    std::map<std::string, Key, std::less<>> _paths;
};


// Reads the description in the file `path`, or tells why it cannot.
std::expected<std::shared_ptr<const Description>, std::string>
load(const std::filesystem::path& path);

// Reads a description from its text. It refuses a text whose first line is
// no header of the format emb-canopen-od, version 1, whose fingerprint is not
// the hash of its body, with a line that is no object of JSON, lacks a key of
// its kind or refers to what the text does not state, and a key or a path
// stated twice. A kind of line or a key it does not know it skips, as the
// format asks; the canonical form it leaves to od_decode.py --check.
std::expected<std::shared_ptr<const Description>, std::string>
parse_description(std::string_view text);

// The 32-bit FNV-1a hash: the fingerprint of a description is the hash of
// every byte after the line of its header.
uint32_t fnv1a(std::string_view data);


} // namespace ucanopen::od
