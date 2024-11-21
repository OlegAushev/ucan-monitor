#pragma once


#include <imgui.h>
#include <ui/view/view.h>
#include <ucanopen_servers/shm80/shm80_server.h>
#include <memory>


namespace ui {
namespace shm80 {

class ControlPanel : public View {
private:
    std::shared_ptr<::shm80::Server> _server;

    bool _emergency{false};
    bool _power{false};
    bool _start{false};

    float _ref_torque_pct{0.0f};
    int16_t _ref_speed{0};

    int _ref_angle{0};
    float _ref_current_pct{0.0f};
    float _ref_voltage_pct{0.0f};
    bool _manual_field{false};
    float _ref_field_pct{0.0f};

    ::shm80::OperatingMode _opmode{::shm80::OperatingMode::normal};

    ::shm80::ControlMode _ctlmode{::shm80::ControlMode::torque};
    int _ctlmode_v{std::to_underlying(::shm80::ControlMode::torque)};

    ::shm80::ControlLoop _ctlloop{::shm80::ControlLoop::closed};
    int _ctlloop_v{std::to_underlying(::shm80::ControlLoop::closed)};
public:
    ControlPanel(std::shared_ptr<::shm80::Server> server,
                 const std::string& menu_title,
                 const std::string& window_title,
                 bool open);
    virtual void draw() override;
private:
    void _draw_dash();
    void _draw_normal_mode_controls();
    void _draw_testing_mode_controls();
    void _draw_actions();

    void _draw_popups();
    void _read_keyboard();

    void _reset_refs();
    void _update_refs();
};


} // namespace shm80
} // namespace ui
