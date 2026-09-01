#pragma once

#include <imgui.h>

#include <ucanopen_servers/adptetk/adptetk_server.hpp>
#include <ui/view/view.h>

#include <memory>

namespace ui {
namespace adptetk {

class StatusPanel : public View {
private:
    std::shared_ptr<::adptetk::Server> _server;
public:
    StatusPanel(std::shared_ptr<::adptetk::Server> server,
                const std::string& menu_title,
                const std::string& window_title,
                bool open);
    virtual void draw() override;
};

} // namespace adptetk
} // namespace ui
