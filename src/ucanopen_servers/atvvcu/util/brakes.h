#pragma once


#include <array>
#include <atomic>
#include <string_view>
#include <utility>


namespace atvvcu {


class Server;


namespace brk {


class Brakes {
    friend class atvvcu::Server;
private:
    std::atomic_bool _debug_mode{false};
    std::atomic_bool _ref_debug_mode{false};

    std::atomic<bool> _ref_left_pressed{false};
    std::atomic<bool> _left_pressed{false};
    std::atomic<bool> _ref_right_pressed{false};
    std::atomic<bool> _right_pressed{false};
public:
    bool debug_mode() const { return _debug_mode.load(); }
    void toggle_debug_mode(bool dbg_enabled) { _ref_debug_mode.store(dbg_enabled); }

    bool left_pressed() const { return _left_pressed.load(); }
    bool right_pressed() const { return _right_pressed.load(); }
    void set_brakes(bool left, bool right) {
        _ref_left_pressed.store(left);
        _ref_right_pressed.store(right);
    }
};


} // namespace brk
} // namespace atvvcu
