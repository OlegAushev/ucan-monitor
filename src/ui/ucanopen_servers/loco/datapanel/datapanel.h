#pragma once


#include <ui/view/view.h>
#include <ucanopen_servers/loco/loco_server.h>


namespace ui {
namespace loco{


class DataPanel : public View {
private:
    std::shared_ptr<::loco::Server> _server;
public:
    DataPanel(std::shared_ptr<::loco::Server> server,
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


} // namespace loco
} // namespace ui
