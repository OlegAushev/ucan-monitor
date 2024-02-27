#pragma once


#include <atomic>


namespace atvvcu {


class Server;


namespace bms {


class Controller {
    friend class atvvcu::Server;
private:
    std::atomic_bool _debug_mode{false};
    std::atomic_bool _ref_debug_mode{false};

    std::atomic<float> _voltage{0.0f};
    std::atomic<float> _charge_pct{0.0f};
public:
    bool debug_mode() const { return _debug_mode.load(); }
    void toggle_debug_mode(bool dbg_enabled) { _ref_debug_mode.store(dbg_enabled); }
};


} // namespace bms


} // namespace atvvcu
