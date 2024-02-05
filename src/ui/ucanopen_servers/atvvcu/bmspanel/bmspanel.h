#pragma once


#include <imgui.h>
#include <string_view>
#include <ui/view/view.h>
#include <ucanopen_servers/atvvcu/atvvcu_server.h>
#include <memory>


namespace ui {
namespace atvvcu {


class BmsPanel : public View {
private:
    std::shared_ptr<::atvvcu::Server> _server;
public:
    BmsPanel(std::shared_ptr<::atvvcu::Server> server,
             const std::string& menu_title,
             const std::string& window_title,
             bool open);
    virtual void draw() override;
};


} // namespace atvvcu
} // namespace ui
