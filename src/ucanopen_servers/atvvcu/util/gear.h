#pragma once


#include <array>
#include <atomic>
#include <string_view>
#include <utility>


namespace atvvcu {


class Server;


namespace gear {


enum class Gear : unsigned int {
    neutral = 0,
    forward = 1,
    reverse = 2
};  
inline constexpr std::array<std::string_view, 3> gears = {"N", "D", "R"};


class GearSelector {
    friend class atvvcu::Server;
private:
    std::atomic_bool _debug_mode{false};
    std::atomic_bool _ref_debug_mode{false};

    std::atomic<Gear> _ref_gear{Gear::neutral};
    std::atomic<Gear> _gear{Gear::neutral};
public:
    bool debug_mode() const { return _debug_mode.load(); }
    void toggle_debug_mode(bool dbg_enabled) { _ref_debug_mode.store(dbg_enabled); }

    Gear gear() const { return _gear.load(); }
    std::string_view gear_str() const { return gears[std::to_underlying(gear())]; }
    void set_gear(Gear gear) { _ref_gear.store(gear); }
};


} // namespace gear


} // namespace atvvcu
