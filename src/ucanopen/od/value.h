#pragma once


#include <ucanopen/od/description.h>
#include <ucanopen/ucanopen_def.h>

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>


// The values of the items of a description: their text, their number, and
// their reading from text. The datatype of an item says how the four bytes of
// an expedited SDO, or of an MPDO, hold its value.
namespace ucanopen::od {


// The bytes a value of `type` takes on the bus, 0 for a type this program
// does not know.
size_t size_of(DataType type);

// The text of a value as the program shows it: a number, true or false, the
// label of a named value, or "N?" for a value that the item's table does not
// name. A float shows `precision` digits after the point, or as many
// significant ones when it is under 0.01 in magnitude.
std::string format_value(const Item& item, ExpeditedSdoData data, int precision = 6);

// The value as a number, for plots and CSV, whatever its type; NaN for a type
// this program does not know.
double to_double(const Item& item, ExpeditedSdoData data);

// The value a person or a file writes as `text`: a number, true or false,
// and for an item with a table also the name of one of its values. A value
// out of the range of the type, or one that the table does not name, is none.
std::optional<ExpeditedSdoData> parse_value(const Item& item, std::string_view text);


} // namespace ucanopen::od
