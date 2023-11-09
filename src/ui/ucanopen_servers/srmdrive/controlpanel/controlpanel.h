#pragma once


#include <imgui.h>
#include <ui/view/view.h>
#include <ucanopen_servers/srmdrive/srmdrive_server.h>
#include <memory>


namespace ui {
namespace srmdrive {

class ControlPanel : public View {
private:
    std::shared_ptr<::srmdrive::Server> _server;
    bool _emergency{false};
    bool _power_enabled{false};
    bool _run_enabled{false};
    int _ctlmode{std::to_underlying(::srmdrive::ControlMode::torque)};
    float _torque_percent_ref{0};
    float _speed_ref{0};

    bool _fieldctl_enabled{false};
    float _fieldcurr_ref{0};
    
    int _ctlloop{std::to_underlying(::srmdrive::ControlLoop::closed)};
    float _dcurr_ref{0};

    float _gamma_correction{0};
public:
    ControlPanel(std::shared_ptr<::srmdrive::Server> server,
                 const std::string& menu_title,
                 const std::string& window_title,
                 bool show_by_default);
    virtual void draw(bool& open) override;
private:
    void _draw_popups();
    void _read_keyboard();
};


} // namespace srmdrive
} // namespace ui
