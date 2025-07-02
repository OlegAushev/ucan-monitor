#pragma once

#include <imgui.h>
#include <ui/view/view.h>
#include <ucanopen_servers/sevpress/sevpress_server.hpp>
#include <memory>

namespace ui {
namespace sevpress {

class DataPanel : public View {
private:
    std::shared_ptr<::sevpress::Server> server_;
public:
    DataPanel(std::shared_ptr<::sevpress::Server> server,
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

} // namespace sevpress
} // namespace ui
