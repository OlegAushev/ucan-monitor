#pragma once


#include <imgui.h>
#include <ui/view/view.h>
#include <ucanopen_servers/chss/chss_server.hpp>

#include <memory>


namespace ui {
namespace chss {

class ControlPanel : public View {
private:
    std::shared_ptr<::chss::Server> _server;

    ::chss::Mode _mode{::chss::Mode::storage};
    int _mode_v{std::to_underlying(::chss::Mode::storage)};

    // Ручные уровни клапанов: СХКВ слушает их только в продувке.
    bool _inlet_open{false};
    bool _outlet_open{false};
public:
    ControlPanel(std::shared_ptr<::chss::Server> server,
                 const std::string& menu_title,
                 const std::string& window_title,
                 bool open);
    virtual void draw() override;
private:
    void _draw_dash();
    void _draw_controls();
    void _draw_actions();

    void _draw_popups();
    void _read_keyboard();

    void _reset_refs();
    void _update_refs();

    void _draw_fact(const char* title, bool active);
    void _draw_valve(const char* title,
                     ::chss::ValvePosition position);
};


} // namespace chss
} // namespace ui
