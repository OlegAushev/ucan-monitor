
#pragma once


#include <imgui.h>
#include <ui/view/view.h>
#include <ucanopen_servers/moyka/moyka_server.h>
#include <memory>


namespace ui {
namespace moyka {

class Panel : public View {
private:
    std::shared_ptr<::moyka::Server> _server;
public:
    Panel(std::shared_ptr<::moyka::Server> server,
          const std::string& menu_title,
          const std::string& window_title,
          bool open);
    virtual void draw() override;
};


} // namespace moyka
} // namespace ui
