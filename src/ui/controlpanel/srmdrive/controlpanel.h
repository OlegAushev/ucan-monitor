#pragma once


#include <imgui.h>
#include "../interface.h"
#include <ucanopen_servers/srmdrive/srmdrive_server.h>
#include <memory>


namespace ui {
namespace srmdrive {

class ControlPanel : public ControlPanelInterface {
private:
    std::shared_ptr<::srmdrive::Server> _server;
    bool _power_enabled = false;
    bool _run_enabled = false;
    int _ctlmode = std::to_underlying(::srmdrive::ControlMode::torque);
    float _torque_percent_ref = 0;
    float _speed_ref = 0;
public:
    ControlPanel(std::shared_ptr<::srmdrive::Server> server) : _server(server) {}
    virtual void draw(bool& open) override;
private:
    void _draw_watch_table();
    void _draw_tpdo1_table();
    void _draw_popups();
};


} // namespace srmdrive
} // namespace ui
