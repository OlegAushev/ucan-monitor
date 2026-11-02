#pragma once


#include <ucanopen/od/description.h>
#include <ucanopen/ucanopen_def.h>

#include <cstdint>
#include <expected>
#include <functional>
#include <map>
#include <optional>
#include <set>
#include <span>
#include <string>
#include <vector>


// The frames of the PDOs of a description, by the rules of section 3.12 of
// emblib's docs/canopen-od-algorithm.ru.md: a frame is a 64-bit little-endian
// word, bit i of a signal is bit start + i of it, a signed signal extends its
// sign from its width, and a raw unit is scale_num / scale_den physical ones.
namespace ucanopen::od {


// A signal as a frame carries it.
struct SignalValue {
    const Signal* signal;
    int64_t raw;
    const Value* named{nullptr}; // the value of its table, null without a table or if the table does not name it
};


struct DecodedPdo {
    // In the order of the description, the counter included; a signal of a
    // role or an encoding this program does not know carries no data here.
    std::vector<SignalValue> signals;
    uint64_t reserved{0}; // the set bits that no signal covers: an RPDO with one is refused
};


// The signals of a frame of `pdo`, which maps signals; nothing for a frame
// shorter than the 8 bytes of a PDO.
std::optional<DecodedPdo> decode_pdo(const Description& description, const Pdo& pdo,
                                     std::span<const uint8_t> data);

// The physical value of a raw one of `signal`.
double physical(const Signal& signal, int64_t raw);

// The text of the physical value, as od_decode.py writes it: every digit if
// the decimal ends, six significant ones if it does not.
std::string physical_text(const Signal& signal, int64_t raw);


// A frame of an MPDO in source address mode: the key of the object it carries
// and the bytes of the value, as an expedited SDO carries them.
struct SamValue {
    Key key;
    ExpeditedSdoData data;
};

// The object of a frame of an MPDO in source address mode from the node
// `node_id`; nothing for a frame shorter than 8 bytes, in destination address
// mode or from another producer.
std::optional<SamValue> decode_sam(std::span<const uint8_t> data, unsigned node_id);


// Why the program cannot build frames of `pdo` from the values of the signals
// it fills, `names`: a signal of a role or an encoding it does not know, a
// value signal it does not fill, or a name the PDO has no signal of. Nothing
// if it can.
std::optional<std::string> encoding_refusal(const Pdo& pdo, const std::set<std::string, std::less<>>& names);

struct EncodedPdo {
    can_payload payload;
    std::vector<std::string> saturated; // the signals whose values did not fit and were saturated
};

// The frame of `pdo` numbered `counter`, from the physical values of its
// value signals by name; a signal with a table takes the number of a value
// the table names. The bits that no signal covers are 0. A value that does
// not fit its signal saturates; one that the table does not name, NaN, or
// values that encoding_refusal refuses give no frame.
std::expected<EncodedPdo, std::string> encode_pdo(const Description& description, const Pdo& pdo,
                                                  const std::map<std::string, double, std::less<>>& values,
                                                  unsigned counter);


// The slot of a TPDO or an RPDO in ucanopen: one of the first four of the
// predefined connection set, whose COB-ID follows from the node-ID.
std::optional<CobTpdo> tpdo_of(const Pdo& pdo);
std::optional<CobRpdo> rpdo_of(const Pdo& pdo);


// A frame as od_decode.py --candump reads it from the node `node_id`, for the
// checks: the values of the signals of a PDO by path, the object of an MPDO,
// why the node refuses an RPDO, or "unknown".
std::string candump_line(const Description& description, unsigned node_id, uint32_t cob_id,
                         std::span<const uint8_t> data);


} // namespace ucanopen::od
