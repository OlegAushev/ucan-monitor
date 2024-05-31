#pragma once


#include <imgui.h>
#include <ui/view/view.h>
#include <ucanopen_servers/brkdrive/brkdrive_server.h>
#include <memory>


namespace ui {
namespace brkdrive {

class ControlPanel : public View {
private:
    std::shared_ptr<::brkdrive::Server> _server;

    bool _wakeup{false};
    float _brake_ref_pu{false};

    float _torque_ref_pct{0.0f};
    float _speed_ref{0.0f};
    float _dcurr_ref_pu{0.0f};
    int _opmode_idx{std::to_underlying(::brkdrive::OperationMode::normal)};
    ::brkdrive::OperationMode _opmode{::brkdrive::OperationMode::normal};
    int _ctlmode{std::to_underlying(::brkdrive::ControlMode::torque)};
    int _ctlloop{std::to_underlying(::brkdrive::ControlLoop::closed)};
    bool _run{false};

    int _openloop_angle_ref{0};
    int _angle_ref{0};
    float _track_speed{0.0f};

public:
    ControlPanel(std::shared_ptr<::brkdrive::Server> server,
                 const std::string& menu_title,
                 const std::string& window_title,
                 bool open);
    virtual void draw() override;
private:
    void _draw_dash();
    void _draw_normal_mode_controls();
    void _draw_run_mode_controls();
    void _draw_track_mode_controls();
    void _draw_hwtest_mode_controls();
    void _draw_actions();

    void _draw_popups();
    void _read_keyboard();

    void _reset_refs();
};


} // namespace srmdrive
} // namespace ui
