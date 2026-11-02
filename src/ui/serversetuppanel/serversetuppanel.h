#pragma once


#include <imgui.h>
#include <ui/view/view.h>
#include <ucanopen/server/server.h>

#include <memory>
#include <string>


namespace ui {


// The setup of a server: what the device says of itself. Its parameters come
// back with stage 5 of docs/canopen-rework-plan.ru.md.
class ServerSetupPanel : public View {
private:
    std::shared_ptr<ucanopen::Server> _server;
public:
    ServerSetupPanel(std::shared_ptr<ucanopen::Server> server,
                     const std::string& menu_title,
                     const std::string& window_title,
                     bool open);
    virtual void draw() override;
private:
    void _draw_about();
};


} // namespace ui
