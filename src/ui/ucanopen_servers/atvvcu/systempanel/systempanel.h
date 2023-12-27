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

    int _vcu_opmode{std::to_underlying(::atvvcu::VcuOperationMode::normal)};
    bool _power_enabled{false};
    bool _run_enabled{false};
public:
    SystemPanel(std::shared_ptr<::atvvcu::Server> server,
                const std::string& menu_title,
                const std::string& window_title,
                bool open);
    virtual void draw() override;
private:
    void _draw_controls();
    void _draw_status();
    void _read_keyboard();
};


} // namespace atvvcu
} // namespace ui
