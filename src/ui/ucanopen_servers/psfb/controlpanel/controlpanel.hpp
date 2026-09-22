#pragma once


#include <imgui.h>
#include <ui/view/view.h>
#include <ucanopen_servers/psfb/psfb_server.hpp>

#include <memory>


namespace ui {
namespace psfb {

class ControlPanel : public View {
private:
    std::shared_ptr<::psfb::Server> _server;

    bool _emergency{false};
    bool _power{false};
    bool _start{false};

    float _ref_overlap1_pct{0.0f};
    float _ref_overlap2_pct{0.0f};
public:
    ControlPanel(std::shared_ptr<::psfb::Server> server,
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
};


} // namespace psfb
} // namespace ui
