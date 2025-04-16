#pragma once

#include <imgui.h>
#include <memory>
#include <ucanopen_servers/shm80/shm80_server.h>
#include <ui/view/view.h>

namespace ui {
namespace shm80 {

class Dashboard : public ui::View {
private:
    std::shared_ptr<::shm80::Server> server_;
public:
    Dashboard(std::shared_ptr<::shm80::Server> server,
              const std::string& menu_title,
              const std::string& window_title,
              bool open);
    virtual void draw() override;
};

} // namespace shm80
} // namespace ui
