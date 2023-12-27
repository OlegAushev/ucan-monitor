#pragma once


#include <imgui.h>
#include <ui/view/view.h>
#include <ucanopen/server/server.h>


namespace ui {


class WatchPanel : public View {
private:
    std::shared_ptr<ucanopen::Server> _server;
    std::vector<const ucanopen::ODObject*> _watch_objects;
public:
    WatchPanel(std::shared_ptr<ucanopen::Server> server,
               const std::string& menu_title,
               const std::string& window_title,
               bool open);
    virtual void draw() override;
};


} // namespace ui
