#pragma once

#include <imgui.h>

#include <ucanopen_servers/psfb/psfb_server.hpp>
#include <ui/view/view.h>

#include <memory>

namespace ui {
namespace psfb {

class StatusPanel : public View {
private:
    std::shared_ptr<::psfb::Server> _server;
public:
    StatusPanel(std::shared_ptr<::psfb::Server> server,
                const std::string& menu_title,
                const std::string& window_title,
                bool open);
    virtual void draw() override;
};

} // namespace psfb
} // namespace ui
