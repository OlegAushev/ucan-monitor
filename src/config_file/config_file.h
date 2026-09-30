#pragma once


#include <ucanopen/ucanopen_def.h>
#include <expected>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>


// A server's settings as a text file. Header lines come first, then a
// `[subcategory]` section for each group of config parameters, a
// `name = value` line for each parameter. `#` and `;` begin a comment at the
// start of a line or after a blank:
//
//   server = adpt-etk-inverter
//   serial_number = 1234
//
//   [drive]
//   torque_slope = 1.5   # pu/s
//
// Values are written the way they read back: floats in the shortest form that
// gives back the same number, with a point whatever the locale. Parameters
// are matched by subcategory and name, so a file outlives a dictionary whose
// objects move to other indices.
namespace config_file {


using Header = std::vector<std::pair<std::string, std::string>>;


// A parameter as the file has it; the value is text until the dictionary says
// what type it is.
struct Parameter {
    std::string subcategory;
    std::string name;
    std::string value;
    int line;
};


struct Contents {
    Header header;
    std::vector<Parameter> parameters; // each subcategory and name once
    std::vector<std::string> warnings; // lines skipped or overridden, and why
};


std::expected<Contents, std::string> read(const std::filesystem::path& path);


// A parameter as it goes into the file: without data it could not be read.
struct Value {
    const ucanopen::ODObject* object;
    std::optional<ucanopen::ExpeditedSdoData> data;
};


// Writes the values grouped by subcategory, in the order they come. Read-only
// parameters and those that could not be read go in as comments: they are
// there to be seen, not to be loaded.
std::expected<void, std::string> write(const std::filesystem::path& path,
                                       const Header& header,
                                       const std::vector<Value>& values);


std::string format_value(const ucanopen::ODObject& object, ucanopen::ExpeditedSdoData data);

std::optional<ucanopen::ExpeditedSdoData> parse_value(const ucanopen::ODObject& object, std::string_view text);

// Floats that differ in the last bits count as the same: a server that keeps a
// parameter in other units gives back a neighbour of what it was sent.
bool same_value(const ucanopen::ODObject& object, ucanopen::ExpeditedSdoData a, ucanopen::ExpeditedSdoData b);


} // namespace config_file
