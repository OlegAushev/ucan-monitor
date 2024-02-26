#pragma once


#include <algorithm>
#include <array>
#include <atomic>
#include <string_view>


namespace atvvcu {


class Server;


namespace drive {


class Controller {
    friend class atvvcu::Server;
public:
    struct Data {
        uint32_t errors;
        std::string_view ctlmode{"..."};
        bool enabled;
        std::string_view discharge{"..."};
        std::string_view faultlevel{"..."};
        uint32_t faultcode;
    };

private:
    std::array<std::atomic<Data>, 4> _data{};

    std::array<std::atomic_bool, 4> _ref_ctlmode{};
    std::array<std::atomic_bool, 4> _ref_enable{};
    std::array<std::atomic_bool, 4> _ref_discharge{};
    std::array<std::atomic_bool, 4> _ref_mainrelay{};
    std::array<std::atomic_bool, 4> _ref_footbrake{};
    std::array<std::atomic_bool, 4> _ref_handbrake{};
    std::array<std::atomic_bool, 4> _ref_faultreset{};
    std::array<std::atomic_uint8_t, 4> _ref_gear{};
    std::array<std::atomic_int16_t, 4> _ref_speed{};
    std::array<std::atomic_int16_t, 4> _ref_torque{};

public:
    std::array<Data, 4> data() const {
        std::array<Data, 4> ret;
        std::copy(_data.begin(), _data.end(), ret.begin());
        return ret;
    }

    void set_ref_ctlmode(const std::array<int, 4>& ref) {
        std::copy(ref.begin(), ref.end(), _ref_ctlmode.begin());
    }

    void set_ref_enable(const std::array<bool, 4>& ref) {
        std::copy(ref.begin(), ref.end(), _ref_enable.begin());
    }

    void set_ref_discharge(const std::array<bool, 4>& ref) {
        std::copy(ref.begin(), ref.end(), _ref_discharge.begin());
    }

    void set_ref_mainrelay(const std::array<bool, 4>& ref) {
        std::copy(ref.begin(), ref.end(), _ref_mainrelay.begin());
    }

    void set_ref_footbrake(const std::array<bool, 4>& ref) {
        std::copy(ref.begin(), ref.end(), _ref_footbrake.begin());
    }

    void set_ref_handbrake(const std::array<bool, 4>& ref) {
        std::copy(ref.begin(), ref.end(), _ref_handbrake.begin());
    }

    void set_ref_faultreset(const std::array<bool, 4>& ref) {
        std::copy(ref.begin(), ref.end(), _ref_faultreset.begin());
    }

    void set_ref_gear(const std::array<int, 4>& ref) {
        std::copy(ref.begin(), ref.end(), _ref_gear.begin());
    }

    void set_ref_speed(const std::array<float, 4>& ref) {
        for (auto i = 0uz; i < ref.size(); ++i) {
            _ref_speed[i] = std::clamp(ref[i], -10000.0f, 10000.0f);
        }
    }

    void set_ref_torque(const std::array<float, 4>& ref) {
        for (auto i = 0uz; i < ref.size(); ++i) {
            _ref_torque[i] = std::clamp(ref[i] * 10.0f, -4000.0f, 4000.0f);
        }
    }
};


} // namespace drive


} // namespace atvvcu
