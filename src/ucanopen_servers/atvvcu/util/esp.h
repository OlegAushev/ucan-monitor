#pragma once


#include <array>
#include <atomic>
#include <string_view>
#include <utility>


namespace atvvcu {


class Server;


namespace esp {


class EspSystem {
    friend class atvvcu::Server;
private:
    std::atomic_bool _debug_mode{false};
    std::atomic_bool _ref_debug_mode{false};

    std::atomic<bool> _tcs_enabled{false};
    std::atomic<bool> _tcs_triggered{false};
    std::atomic<bool> _ref_tcs_enable{true};
public:
    bool debug_mode() const { return _debug_mode.load(); }
    void toggle_debug_mode(bool dbg_enabled) { _ref_debug_mode.store(dbg_enabled); }

    bool tcs_enabled() const { return _tcs_enabled.load(); }
    bool tcs_triggered() const { return _tcs_triggered.load(); }
    void toggle_tcs(bool enable) { _ref_tcs_enable.store(enable); }
};


} // namespace esp
} // namespace atvvcu
