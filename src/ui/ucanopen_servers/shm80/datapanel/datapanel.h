#pragma once

#include <imgui.h>
#include <ui/view/view.h>
#include <ucanopen_servers/shm80/shm80_server.h>
#include <memory>

namespace ui {
namespace shm80 {

class DataPanel : public View {
private:
    std::shared_ptr<::shm80::Server> server_;
public:
    DataPanel(std::shared_ptr<::shm80::Server> server,
              const std::string& menu_title,
              const std::string& window_title,
              bool open);
    virtual void draw() override;
private:
    void draw_tpdo1_table();
    void draw_tpdo2_table();
    void draw_tpdo3_table();
    void draw_tpdo4_table();
};

} // namespace shm80
} // namespace ui
