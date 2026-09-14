#pragma once

#include <imgui.h>

#include <ucanopen_servers/adptbike/adptbike_server.hpp>
#include <ui/view/view.h>

#include <memory>

namespace ui {
namespace adptbike {

class StatusPanel : public View {
private:
    std::shared_ptr<::adptbike::Server> _server;
public:
    StatusPanel(std::shared_ptr<::adptbike::Server> server,
                const std::string& menu_title,
                const std::string& window_title,
                bool open);
    virtual void draw() override;
};

} // namespace adptbike
} // namespace ui
