#pragma once


#include <imgui.h>
#include <ui/view/view.h>
#include <ucanopen_servers/atvvcu/atvvcu_server.h>
#include <memory>


namespace ui {
namespace atvvcu {


class MotorControlPanel : public View {
private:
    std::shared_ptr<::atvvcu::Server> _server;
public:
    MotorControlPanel(std::shared_ptr<::atvvcu::Server> server,
                      const std::string& menu_title,
                      const std::string& window_title,
                      bool show_by_default);
    virtual void draw(bool& open) override;
};


} // namespace atvvcu
} // namespace ui
