#pragma once


#include <imgui.h>
#include <ui/view/view.h>
#include <ucanopen/server/server.h>

#include <memory>
#include <string>


namespace ui {


// The statuses of a server, which its device sends itself in an MPDO, each by
// its label and its severity.
class StatusPanel : public View {
private:
    std::shared_ptr<ucanopen::Server> _server;
public:
    StatusPanel(std::shared_ptr<ucanopen::Server> server,
                const std::string& menu_title,
                const std::string& window_title,
                bool open);
    virtual void draw() override;
};


} // namespace ui
