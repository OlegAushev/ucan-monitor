#pragma once

#include <imgui.h>

#include <ucanopen_servers/pdu/pdu_server.hpp>
#include <ui/view/view.h>

#include <memory>

namespace ui {
namespace pdu {

class StatusPanel : public View {
private:
    std::shared_ptr<::pdu::Server> _server;
public:
    StatusPanel(std::shared_ptr<::pdu::Server> server,
                const std::string& menu_title,
                const std::string& window_title,
                bool open);
    virtual void draw() override;
};

} // namespace pdu
} // namespace ui
