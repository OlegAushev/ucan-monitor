#pragma once


#include <ui/view/view.h>
#include <ucanopen_servers/loco/loco_server.h>


namespace ui {
namespace loco {


class ControlPanel : public View {
private:
    std::shared_ptr<::loco::Server> _server;

    bool _power{false};
    bool _run{false};

    float _ref_torque_pct{0.f};
    int16_t _ref_speed{0};

    float _ref_d_angle_deg{0.f};
    float _ref_d_current_pu{0.f};

    ::loco::OperatingMode _opmode{::loco::OperatingMode::normal};

    ::loco::ControlMode _ctlmode{::loco::ControlMode::torque};
    int _ctlmode_v{std::to_underlying(::loco::ControlMode::torque)};

    ::loco::ControlLoop _ctlloop{::loco::ControlLoop::closed};
    int _ctlloop_v{std::to_underlying(::loco::ControlLoop::closed)};

public:
    ControlPanel(std::shared_ptr<::loco::Server> server,
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


} // namespace loco
} // namespace ui
