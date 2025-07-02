#pragma once


#include <imgui.h>
#include <ui/view/view.h>
#include <ucanopen_servers/sevpress/sevpress_server.hpp>

#include <memory>


namespace ui {
namespace sevpress {

class ControlPanel : public View {
private:
    std::shared_ptr<::sevpress::Server> _server;

    bool _emergency{false};
    bool _power{false};
    bool _start{false};

    float _ref_torque_pct{0.0f};
    int16_t _ref_speed{0};

    int _ref_angle{0};
    float _ref_current_pct{0.0f};
    float _ref_voltage_pct{0.0f};

    ::sevpress::OperatingMode _opmode{::sevpress::OperatingMode::normal};

    ::sevpress::ControlMode _ctlmode{::sevpress::ControlMode::torque};
    int _ctlmode_v{std::to_underlying(::sevpress::ControlMode::torque)};

    ::sevpress::ControlLoop _ctlloop{::sevpress::ControlLoop::closed};
    int _ctlloop_v{std::to_underlying(::sevpress::ControlLoop::closed)};
public:
    ControlPanel(std::shared_ptr<::sevpress::Server> server,
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
