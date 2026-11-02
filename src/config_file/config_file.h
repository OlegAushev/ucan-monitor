#pragma once


#include <ucanopen/od/description.h>
#include <ucanopen/ucanopen_def.h>
#include <expected>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>


// A server's settings as a text file. Header lines come first, then a
// `[group]` section for each group of parameters, a `name = value` line for
// each parameter. `#` and `;` begin a comment at the start of a line or after
// a blank:
//
//   server = adpt-etk-inverter
//   serial_number = 1234
//
//   [model]
//   torque_slope = 1.5   # о. е./с
//
// Values are written the way they read back: floats in the shortest form that
// gives back the same number, with a point whatever the locale, and a value of
// a table by its name. Parameters are matched by group and name, their path,
// so a file outlives a dictionary whose objects move to other indices.
namespace config_file {


using Header = std::vector<std::pair<std::string, std::string>>;


// A parameter as the file has it; the value is text until the description
// says what type it is.
struct Parameter {
    std::string group;
    std::string name;
    std::string value;
    int line;
};


struct Contents {
    Header header;
    std::vector<Parameter> parameters; // each group and name once
    std::vector<std::string> warnings; // lines skipped or overridden, and why
};


std::expected<Contents, std::string> read(const std::filesystem::path& path);


// A parameter as it goes into the file: its section and name, what its value
// is, whether a load would write it, and the value, if it could be read.
struct Value {
    std::string group;
    std::string name;
    ucanopen::od::Item item; // the type, the table and the unit of the value
    bool writable;
    std::optional<ucanopen::ExpeditedSdoData> data;
};


// Writes the values grouped by section, in the order they come. Those that a
// load would not write and those that could not be read go in as comments:
// they are there to be seen, not to be loaded.
std::expected<void, std::string> write(const std::filesystem::path& path,
                                       const Header& header,
                                       const std::vector<Value>& values);


std::string format_value(const ucanopen::od::Item& item, ucanopen::ExpeditedSdoData data);

std::optional<ucanopen::ExpeditedSdoData> parse_value(const ucanopen::od::Item& item, std::string_view text);

// Floats that differ in the last bits count as the same: a server that keeps a
// parameter in other units gives back a neighbour of what it was sent.
bool same_value(const ucanopen::od::Item& item, ucanopen::ExpeditedSdoData a, ucanopen::ExpeditedSdoData b);


} // namespace config_file
