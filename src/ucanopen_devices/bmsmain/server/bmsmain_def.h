#pragma once


#include <cstdint>


namespace bmsmain {

struct CobTpdo1 {
    uint32_t discrete_inputs : 8;
    uint32_t current : 16;
    uint32_t temp_min : 8;
    uint32_t temp_max : 8;
    uint32_t charge_percentage : 8;
    uint32_t voltage : 16;
    CobTpdo1() = default;
};

}

