#pragma once


#include <atomic>


namespace atvvcu {


class Server;


namespace gear {


class GearSelector {
    friend class atvvcu::Server;
private:
    std::atomic_bool _debug_mode{false};
    std::atomic_bool _ref_debug_mode{false};
public:
    bool debug_mode() const { return _debug_mode; }
    void toggle_debug_mode(bool dbg_enabled) { _ref_debug_mode = dbg_enabled; }
};


} // namespace gear


} // namespace atvvcu
