#pragma once


#include <array>
#include <atomic>


namespace atvvcu {


class Server;


namespace pdm {


inline constexpr int contactor_count = 7;
enum class Contactor : unsigned int {
    battery_p,
    battery_n,
    front_bypass,
    back_bypass,
    aux_bypass,
    charge_allow,
    charge_mode,
};


struct Controller {
    friend class atvvcu::Server;
private:
    std::atomic_bool _debug_mode{false};
    std::atomic_bool _ref_debug_mode{false};
    std::array<std::atomic_bool, contactor_count> _contactor_feedback_state{};
    std::array<std::atomic_bool, contactor_count> _contactor_ref_state{};
public:
    bool debug_mode() const { return _debug_mode.load(); }
    void toggle_debug_mode(bool dbg_enabled) { _ref_debug_mode.store(dbg_enabled); }

    std::array<bool, contactor_count> contactor_feedback_state() const {
        std::array<bool, contactor_count> ret;
        std::copy(_contactor_feedback_state.begin(), _contactor_feedback_state.end(), ret.begin());
        return ret;
    }

    void set_contactor_ref_state(const std::array<bool, contactor_count> ref) {
        std::copy(ref.begin(), ref.end(), _contactor_ref_state.begin());
    }
};


} // namespace pdm


} // namespace atvvcu
