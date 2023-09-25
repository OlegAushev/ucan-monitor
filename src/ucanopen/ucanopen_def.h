#pragma once


#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <string_view>
#include <sstream>
#include <charconv>
#include <array>
#include <ios>
#include <map>
#include <cmath>

#include <linux/can.h>


namespace ucanopen {

using can_payload = std::array<uint8_t, 8>;


template <typename T>
inline can_payload to_payload(const T& message) {
    static_assert(sizeof(T) <= 8);
    can_payload payload{};
    memcpy(payload.data(), &message, sizeof(T));
    return payload;
}


template <typename T>
inline T from_payload(const can_payload& payload) {
    static_assert(sizeof(T) <= 8);
    T message;
    memcpy(&message, payload.data(), sizeof(T));
    return message;
}


class NodeId {
private:
    unsigned int _id;
public:
    explicit NodeId(unsigned int id) : _id(id) {}
    unsigned int get() const { return _id; }
    bool valid() const { return (_id >= 1) && (_id <= 127); }
};


inline bool operator==(const NodeId& lhs, const NodeId& rhs) { return lhs.get() == rhs.get(); }


enum class NmtState {
    initialization = 0x00,
    stopped = 0x04,
    operational = 0x05,
    pre_operational = 0x7F
};


enum class CobType {
    nmt,
    sync,
    emcy,
    time,
    tpdo1,
    rpdo1,
    tpdo2,
    rpdo2,
    tpdo3,
    rpdo3,
    tpdo4,
    rpdo4,
    tsdo,
    rsdo,
    heartbeat
};


constexpr int cob_type_count = 15;


constexpr std::array<canid_t, cob_type_count> cob_function_codes = {
    0x000,  // NMT
    0x080,  // SYNC
    0x080,  // EMCY
    0x100,  // TIME
    0x180,  // TPDO1
    0x200,  // RPDO1
    0x280,  // TPDO2
    0x300,  // RPDO2
    0x380,  // TPDO3
    0x400,  // RPDO3
    0x480,  // TPDO4
    0x500,  // RPDO4
    0x580,  // TSDO
    0x600,  // RSDO
    0x700   // HEARTBEAT
};


inline canid_t calculate_cob_id(CobType cob_type, NodeId node_id) {
    if ((cob_type == CobType::nmt) || (cob_type == CobType::sync) || (cob_type == CobType::time)) {
        return cob_function_codes[static_cast<int>(cob_type)];
    }
    return cob_function_codes[static_cast<int>(cob_type)] + node_id.get();
}


constexpr std::array<int, cob_type_count> cob_sizes = {
    2,  // NMT
    0,  // SYNC
    2,  // EMCY
    6,  // TIME
    8,  // TPDO1
    8,  // RPDO1
    8,  // TPDO2
    8,  // RPDO2
    8,  // TPDO3
    8,  // RPDO3
    8,  // TPDO4
    8,  // RPDO4
    8,  // TSDO
    8,  // RSDO
    1   // HEARTBEAT
};


enum class TpdoType {
    tpdo1,
    tpdo2,
    tpdo3,
    tpdo4,
};


inline CobType to_cob_type(TpdoType tpdo_type) {
    return static_cast<CobType>(
        static_cast<unsigned int>(CobType::tpdo1) + 2 * static_cast<unsigned int>(tpdo_type)
    );
}


enum class RpdoType {
    rpdo1,
    rpdo2,
    rpdo3,
    rpdo4,	
};


inline CobType to_cob_type(RpdoType rpdo_type) {
    return static_cast<CobType>(
        static_cast<unsigned int>(CobType::rpdo1) + 2 * static_cast<unsigned int>(rpdo_type)
    );
}


inline RpdoType opposite_pdo(TpdoType type) {
    return static_cast<RpdoType>(type);
}


inline TpdoType opposite_pdo(RpdoType type) {
    return static_cast<TpdoType>(type);
}


enum ODObjectType {
    OD_BOOL,
    OD_INT16,
    OD_INT32,
    OD_UINT16,
    OD_UINT32,
    OD_FLOAT32,
    OD_ENUM16,
    OD_EXEC,
    OD_STRING
};


constexpr int od_object_type_sizes[9] = {sizeof(bool), sizeof(int16_t), sizeof(int32_t),
                                         sizeof(uint16_t), sizeof(uint32_t), sizeof(float),
                                         sizeof(uint16_t), 0, 0};


namespace sdo_cs_codes {
const uint32_t client_init_write = 1;
const uint32_t server_init_write = 3;
const uint32_t client_init_read = 2;
const uint32_t server_init_read = 2;

const uint32_t abort = 4;
}


inline uint32_t get_cs_code(const can_frame& frame) {
    return frame.data[0] >> 5;
}


class ExpeditedSdoData {
private:
    uint32_t _data = 0;
public:
    ExpeditedSdoData() = default;
    ExpeditedSdoData(bool val) { memcpy(&_data, &val, sizeof(val)); }
    ExpeditedSdoData(int16_t val) { memcpy(&_data, &val, sizeof(val)); }
    ExpeditedSdoData(int32_t val) { memcpy(&_data, &val, sizeof(_data)); }
    ExpeditedSdoData(uint16_t val) { memcpy(&_data, &val, sizeof(val)); }
    ExpeditedSdoData(uint32_t val) { memcpy(&_data, &val, sizeof(_data)); }
    ExpeditedSdoData(float val) { memcpy(&_data, &val, sizeof(_data)); }

    bool bool32() const {
        return _data;
    }

    int16_t i16() const {
        int16_t val;
        memcpy(&val, &_data, sizeof(int16_t));
        return val;
    }

    int32_t i32() const {
        int32_t val;
        memcpy(&val, &_data, sizeof(int32_t));
        return val;
    }

    uint16_t u16() const { 
        uint16_t val;
        memcpy(&val, &_data, sizeof(uint16_t));
        return val;
    }

    uint32_t u32() const { 
        uint32_t val;
        memcpy(&val, &_data, sizeof(uint32_t));
        return val;
    }
    
    float f32() const { 
        float val;
        memcpy(&val, &_data, sizeof(float));
        return val;
    }

    std::string to_string(ODObjectType type, int precision = 6) const {
        switch (type) {
        case ucanopen::OD_BOOL:
            return bool32() ? "true" : "false";
        case ucanopen::OD_INT16:
            return std::to_string(i16());
        case ucanopen::OD_INT32:
            return std::to_string(i32());
        case ucanopen::OD_UINT16:
            return std::to_string(u16());
        case ucanopen::OD_UINT32:
            return std::to_string(u32());
        case ucanopen::OD_FLOAT32: {
            std::chars_format format = (fabsf(f32()) >= 0.01) ? std::chars_format::fixed : std::chars_format::general;
            std::array<char, 16> buf;
            if (auto [ptr, ec] = std::to_chars(buf.begin(), buf.end(), f32(), format, precision);
                    ec == std::errc()) {
                return std::string(buf.begin(), ptr);
            } else {
                return std::make_error_code(ec).message();
            }
        }
        case ucanopen::OD_ENUM16:
            return std::to_string(u16());
        case ucanopen::OD_EXEC:
            return std::string();
        case ucanopen::OD_STRING: {
            char ret[5] = {0};
            memcpy(ret, &_data, sizeof(_data));
            return std::string(ret);
        }
        }
        return std::string();
    }
};


struct ExpeditedSdo {
    uint32_t data_size_indicated : 1;
    uint32_t expedited_transfer : 1;
    uint32_t data_empty_bytes : 2;
    uint32_t _reserved : 1;
    uint32_t cs : 3;
    uint32_t index : 16;
    uint32_t subindex : 8;
    ExpeditedSdoData data = {};

    ExpeditedSdo() = default;

    ExpeditedSdo(const can_payload& payload) {
        memcpy(this, payload.data(), sizeof(ExpeditedSdo));
    }

    ExpeditedSdo(const uint8_t* data) {
        memcpy(this, data, sizeof(ExpeditedSdo));
    }

    can_payload to_payload() const {
        can_payload payload;
        memcpy(payload.data(), this, sizeof(ExpeditedSdo));
        return payload;
    }
};


struct AbortSdo {
    uint32_t _reserved : 5;
    uint32_t cs : 3;
    uint32_t index : 16;
    uint32_t subindex : 8;
    uint32_t error_code;

    AbortSdo() {
        memset(this, 0, sizeof(AbortSdo));
        cs = sdo_cs_codes::abort;
    }

    AbortSdo(const can_payload& payload) {
        memcpy(this, payload.data(), sizeof(AbortSdo));
    }

    AbortSdo(const uint8_t* data) {
        memcpy(this, data, sizeof(AbortSdo));
    }

    can_payload to_payload() const {
        can_payload payload;
        memcpy(payload.data(), this, sizeof(AbortSdo));
        return payload;
    }
};


enum class SdoAbortCode : uint32_t {
    no_error                = 0,
    invalid_cs              = 0x05040001,
    unsupported_access      = 0x06010000,
    read_access_wo          = 0x06010001,
    write_access_ro         = 0x06010002,
    object_not_found        = 0x06020000,
    hardware_error          = 0x06060000,
    value_range_exceeded    = 0x06090030,
    value_too_high          = 0x06090031,
    value_too_low           = 0x06090032,
    general_error           = 0x08000000,
    data_store_error        = 0x08000020,
    local_control_error     = 0x08000021,
    state_error             = 0x08000022
};


const std::map<SdoAbortCode, std::string_view> sdo_abort_messages = {
    {SdoAbortCode::no_error, "no error"},
    {SdoAbortCode::invalid_cs, "client/server command specifier not valid or unknown"},
    {SdoAbortCode::unsupported_access, "unsupported access to an object"},
    {SdoAbortCode::read_access_wo, "attempt to read a write-only object"},
    {SdoAbortCode::write_access_ro, "attempt to write a read-only object"},
    {SdoAbortCode::object_not_found, "object does not exist in the object dictionary"},
    {SdoAbortCode::hardware_error, "access failed due to a hardware error"},
    {SdoAbortCode::value_range_exceeded, "Value range of parameter exceeded"},
    {SdoAbortCode::value_too_high, "Value of parameter written is too high"},
    {SdoAbortCode::value_too_low, "Value of parameter written is too low"},
    {SdoAbortCode::general_error, "general error"},
    {SdoAbortCode::data_store_error, "data cannot be transferred or stored to the application"},
    {SdoAbortCode::local_control_error, "data cannot be transferred or stored to the application because of local control"},
    {SdoAbortCode::state_error, "data cannot be transferred or stored to the application because of the present device state"}
};


enum class SdoType {
    response_to_read,
    response_to_write,
    response_to_exec,
    abort
};


enum ODObjectAccessPermission {
    OD_ACCESS_RW,
    OD_ACCESS_RO,
    OD_ACCESS_WO,
    OD_ACCESS_CONST
};


struct ODObjectKey {
    unsigned int index;
    unsigned int subindex;
};


inline bool operator<(const ODObjectKey& lhs, const ODObjectKey& rhs) {
    return (lhs.index < rhs.index) || ((lhs.index == rhs.index) && (lhs.subindex < rhs.subindex));
}


struct ODObject {
    std::string_view category;
    std::string_view subcategory;
    std::string_view name;
    std::string_view unit;
    ODObjectType type;
    ODObjectAccessPermission access_permission;

    bool has_read_permission() const { return access_permission != OD_ACCESS_WO; }
    bool has_write_permission() const { return (access_permission == OD_ACCESS_RW) || (access_permission == OD_ACCESS_WO); }
};


struct ODObjectAux {
    std::string_view category;
    std::string_view subcategory;
    std::string_view name;
};


inline bool operator<(const ODObjectAux& lhs, const ODObjectAux& rhs) {
    return (lhs.category < rhs.category) 
            || ((lhs.category == rhs.category) && (lhs.subcategory < rhs.subcategory))
            || ((lhs.category == rhs.category) && (lhs.subcategory == rhs.subcategory) && (lhs.name < rhs.name));
}


struct ObjectDictionaryConfig {
    std::string_view watch_category;
    std::string_view config_category;
};


using ObjectDictionaryEntries = std::map<ODObjectKey, ODObject>;
using ODEntryIter = ObjectDictionaryEntries::const_iterator;
using ObjectDictionaryAux = std::map<ODObjectAux, ODEntryIter>;


struct ObjectDictionary {
    ObjectDictionaryConfig config;
    ObjectDictionaryEntries entries;
};


inline can_frame create_frame(CobType cob_type, NodeId node_id, const can_payload& payload) {
    can_frame frame = {};
    frame.can_id = calculate_cob_id(cob_type, node_id);
    frame.len = cob_sizes[static_cast<int>(cob_type)];
    std::copy(payload.begin(), std::next(payload.begin(), frame.len), frame.data);
    return frame;
}


inline can_frame create_frame(canid_t id, unsigned char len, const can_payload& payload) {
    can_frame frame = {};
    frame.can_id = id;
    frame.len = len;
    std::copy(payload.begin(), std::next(payload.begin(), frame.len), frame.data);
    return frame;	
}

} // namespace ucanopen

