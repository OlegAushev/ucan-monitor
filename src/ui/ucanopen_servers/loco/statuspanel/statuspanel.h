#pragma once


#include <ui/view/view.h>
#include <ucanopen_servers/loco/loco_server.h>


namespace ui {
namespace loco {

class StatusPanel : public View {
private:
    std::shared_ptr<::loco::Server> _server;
public:
    StatusPanel(std::shared_ptr<::loco::Server> server,
                const std::string& menu_title,
                const std::string& window_title,
                bool open);
    virtual void draw() override;
private:
    void _draw_error_table();
    void _draw_warning_table();
};


} // namespace loco
} // namespace ui
