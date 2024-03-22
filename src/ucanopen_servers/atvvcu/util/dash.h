#pragma once


#include <atomic>


namespace atvvcu {


class Server;


namespace dash {


class Dashboard {
    friend class atvvcu::Server;
private:
    std::atomic_bool _debug_mode{false};
    std::atomic_bool _ref_debug_mode{false};

    std::atomic<bool> _remote_control{false};
    std::atomic<bool> _emergency{false};
    std::atomic<bool> _fault_reset{false};
    std::atomic<bool> _power_enabled{false};
    std::atomic<bool> _run_enabled{false};

    std::atomic<bool> _ref_emergency{false};
    std::atomic<bool> _ref_fault_reset{false};
    std::atomic<bool> _ref_power_enabled{false};
    std::atomic<bool> _ref_run_enabled{false};
public:
    bool debug_mode() const { return _debug_mode.load(); }
    void toggle_debug_mode(bool dbg_enabled) { _ref_debug_mode.store(dbg_enabled); }

    bool remote_control_enabled() const { return _remote_control.load(); }
    bool emergency() const { return _emergency.load(); }
    bool fault_reset_enabled() const { return _fault_reset.load(); }
    bool power_enabled() const { return _power_enabled.load(); }
    bool run_enabled() const { return _run_enabled.load(); }

    void toggle_fault_reset(bool is_enabled) { _ref_fault_reset.store(is_enabled); }
    void toggle_emergency(bool is_enabled) { _ref_emergency.store(is_enabled); }
    void toggle_power(bool is_enabled) { _ref_power_enabled.store(is_enabled); }
    void toggle_run(bool is_enabled) { _ref_run_enabled.store(is_enabled); }
};


} // namespace dash


} // namespace atvvcu
