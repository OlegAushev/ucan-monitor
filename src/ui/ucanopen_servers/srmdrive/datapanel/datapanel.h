#pragma once


#include <imgui.h>
#include <ui/view/view.h>
#include <ucanopen_servers/srmdrive/srmdrive_server.h>
#include <memory>


namespace ui {
namespace srmdrive {

class DataPanel : public View {
private:
    std::shared_ptr<::srmdrive::Server> _server;
public:
    DataPanel(std::shared_ptr<::srmdrive::Server> server,
              const std::string& menu_title,
              const std::string& window_title,
              bool open);
    virtual void draw() override;
private:
    void _draw_tpdo1_table();
    void _draw_tpdo2_table();
    void _draw_tpdo3_table();
    void _draw_tpdo4_table();
};


} // namespace srmdrive
} // namespace ui
