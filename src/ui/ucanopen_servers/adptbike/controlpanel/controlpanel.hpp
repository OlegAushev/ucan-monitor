#pragma once


#include <imgui.h>
#include <ui/view/view.h>
#include <ucanopen_servers/adptbike/adptbike_server.hpp>

#include <memory>


namespace ui {
namespace adptbike {

class ControlPanel : public View {
private:
    std::shared_ptr<::adptbike::Server> _server;

    bool _emergency{false};
    bool _power{false};
    bool _start{false};

    float _ref_torque_pct{0.0f};
    int16_t _ref_speed{0};
    int _ref_angle{0};
    float _ref_current_pct{0.0f};
    float _ref_voltage_pct{0.0f};
    float _ref_dc_a{0.0f};
    float _ref_dc_b{0.0f};
    float _ref_dc_c{0.0f};

    ::adptbike::ControlMode _control_mode{::adptbike::ControlMode::torque};
    int _control_mode_v{std::to_underlying(::adptbike::ControlMode::torque)};

    ::adptbike::ModelMode _model_mode{::adptbike::ModelMode::closed};
    int _model_mode_v{std::to_underlying(::adptbike::ModelMode::closed)};
public:
    ControlPanel(std::shared_ptr<::adptbike::Server> server,
                 const std::string& menu_title,
                 const std::string& window_title,
                 bool open);
    virtual void draw() override;
private:
    void _draw_dash();
    void _draw_controls();
    void _draw_actions();

    void _draw_popups();
    void _read_keyboard();

    void _reset_refs();
    void _update_refs();
};


} // namespace adptbike
} // namespace ui
