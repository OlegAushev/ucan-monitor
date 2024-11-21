#pragma once

#include <imgui.h>

#include <ucanopen_servers/shm80/shm80_server.h>
#include <ui/view/view.h>

#include <memory>

namespace ui {
namespace shm80 {

class StatusPanel : public View {
private:
    std::shared_ptr<::shm80::Server> _server;
public:
    StatusPanel(std::shared_ptr<::shm80::Server> server,
                const std::string& menu_title,
                const std::string& window_title,
                bool open);
    virtual void draw() override;
};

} // namespace shm80
} // namespace ui
