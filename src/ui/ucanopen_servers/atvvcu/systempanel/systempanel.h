#pragma once


#include <imgui.h>
#include <ui/view/view.h>
#include <ucanopen_servers/atvvcu/atvvcu_server.h>
#include <memory>


namespace ui {
namespace atvvcu {


class SystemPanel : public View {
private:
    std::shared_ptr<::atvvcu::Server> _server;

    bool _debug_enabled{false};
    bool _power_enabled{false};
    bool _run_enabled{false};
public:
    SystemPanel(std::shared_ptr<::atvvcu::Server> server,
                const std::string& menu_title,
                const std::string& window_title,
                bool show_by_default);
    virtual void draw(bool& open) override;
private:
    void _read_keyboard();
};


} // namespace atvvcu
} // namespace ui
