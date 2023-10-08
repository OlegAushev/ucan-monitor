#pragma once


#include <imgui.h>
#include "../interface.h"
#include <ucanopen_servers/srmdrive/srmdrive_server.h>
#include <memory>


namespace ui {
namespace srmdrive {

class StatusPanel : public StatusPanelInterface {
private:
    std::shared_ptr<::srmdrive::Server> _server;
public:
    StatusPanel(std::shared_ptr<::srmdrive::Server> server) : _server(server) {}
    virtual void draw(bool& open) override;
private:
    void _draw_error_table();
    void _draw_warning_table();
};


} // namespace srmdrive
} // namespace ui
