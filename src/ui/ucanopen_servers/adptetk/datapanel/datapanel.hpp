#pragma once

#include <imgui.h>
#include <ui/view/view.h>
#include <ucanopen_servers/adptetk/adptetk_server.hpp>
#include <memory>

namespace ui {
namespace adptetk {

class DataPanel : public View {
private:
    std::shared_ptr<::adptetk::Server> server_;
public:
    DataPanel(std::shared_ptr<::adptetk::Server> server,
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

} // namespace adptetk
} // namespace ui
