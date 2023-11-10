#pragma once


#include <imgui.h>
#include <ui/view/view.h>
#include <ucanopen_servers/atvvcu/atvvcu_server.h>


namespace ui {
namespace atvvcu {


class MotorDataPanel : public ui::View {
private:
    std::shared_ptr<::atvvcu::Server> _server;
public:
    MotorDataPanel(std::shared_ptr<::atvvcu::Server> server,
                   const std::string& menu_title,
                   const std::string& window_title,
                   bool show_by_default);
    virtual void draw(bool& open) override;
private:
    std::array<std::string, 4> _window_titles = {"Motor LF Data##data",
                                                 "Motor LB Data##data",
                                                 "Motor RF Data##data",
                                                 "Motor RB Data##data"};
};


} // namespace atvvcu
} // namespace ui
