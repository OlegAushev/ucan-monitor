#pragma once


#include <imgui.h>
#include <ui/view/view.h>
#include <ucanopen_servers/brkdrive/brkdrive_server.h>
#include <memory>


namespace ui {
namespace brkdrive {

class StatusPanel : public View {
private:
    std::shared_ptr<::brkdrive::Server> _server;
public:
    StatusPanel(std::shared_ptr<::brkdrive::Server> server,
                const std::string& menu_title,
                const std::string& window_title,
                bool open);
    virtual void draw() override;
private:
    void _draw_error_table();
    void _draw_warning_table();
};


} // namespace brkdrive
} // namespace ui
