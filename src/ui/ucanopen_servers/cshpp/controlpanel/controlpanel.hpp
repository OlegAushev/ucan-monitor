#pragma once


#include <imgui.h>
#include <ui/view/view.h>
#include <ucanopen_servers/cshpp/cshpp_server.hpp>

#include <array>
#include <memory>


namespace ui {
namespace cshpp {

class ControlPanel : public View {
private:
    std::shared_ptr<::cshpp::Server> _server;

    ::cshpp::Mode _mode{::cshpp::Mode::idle};
    int _mode_v{std::to_underlying(::cshpp::Mode::idle)};

    // Ручные уровни продувки: БКСГЭУ исполняет их только в продувке.
    std::array<bool, ::cshpp::valve_count> _valves_open{};
    std::array<bool, ::cshpp::contactor_count> _contactors_closed{};
public:
    ControlPanel(std::shared_ptr<::cshpp::Server> server,
                 const std::string& menu_title,
                 const std::string& window_title,
                 bool open);
    virtual void draw() override;
private:
    void _draw_dash();
    void _draw_controls();
    void _draw_substitutes();
    void _draw_actions();

    void _draw_popups();
    void _read_keyboard();

    void _reset_refs();
    void _update_refs();

    void _draw_valve(const char* title, ::cshpp::ValvePosition position);
};


} // namespace cshpp
} // namespace ui
