#pragma once


#include <atomic>


namespace atvvcu {


class Server;


namespace accl {


class AcceleratorPedal {
    friend class atvvcu::Server;
private:
    std::atomic_bool _debug_mode{false};
    std::atomic_bool _ref_debug_mode{false};

    std::atomic<float> _pressure{0.0f};
    std::atomic<float> _ref_pressure{0.0f};
public:
    bool debug_mode() const { return _debug_mode.load(); }
    void toggle_debug_mode(bool dbg_enabled) { _ref_debug_mode.store(dbg_enabled); }

    float pressure() const { return _pressure.load(); }
    void set_pressure(float value) { _ref_pressure.store(value); }
};


} // namespace accl


} // namespace atvvcu
