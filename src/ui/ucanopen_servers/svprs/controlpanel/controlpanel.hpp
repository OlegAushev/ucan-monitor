#pragma once


#include <imgui.h>
#include <ui/view/view.h>
#include <ucanopen_servers/svprs/svprs_server.hpp>

#include <memory>


namespace ui {
namespace svprs {

class ControlPanel : public View {
private:
    std::shared_ptr<::svprs::Server> _server;

    bool _emergency{false};
    bool _emergency_sent{false}; // system.emergency, once a press
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

    ::svprs::ControlMode _control_mode{::svprs::ControlMode::torque};
    int _control_mode_v{std::to_underlying(::svprs::ControlMode::torque)};

    ::svprs::ModelMode _model_mode{::svprs::ModelMode::closed_loop};
    int _model_mode_v{std::to_underlying(::svprs::ModelMode::closed_loop)};
public:
    ControlPanel(std::shared_ptr<::svprs::Server> server,
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

    void _update_refs();
};


} // namespace svprs
} // namespace ui
