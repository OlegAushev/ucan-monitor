#pragma once

#include <imgui.h>

#include <ucanopen_servers/sevpress/sevpress_server.hpp>
#include <ui/view/view.h>

#include <memory>

namespace ui {
namespace sevpress {

class StatusPanel : public View {
private:
    std::shared_ptr<::sevpress::Server> _server;
public:
    StatusPanel(std::shared_ptr<::sevpress::Server> server,
                const std::string& menu_title,
                const std::string& window_title,
                bool open);
    virtual void draw() override;
};

} // namespace sevpress
} // namespace ui
