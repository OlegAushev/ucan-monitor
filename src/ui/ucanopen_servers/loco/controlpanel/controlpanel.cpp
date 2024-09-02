#include "controlpanel.h"
#include <ui/util/togglebutton.h>
#include <ui/util/util.h>


using namespace loco;


namespace ui {
namespace loco {


ControlPanel::ControlPanel(std::shared_ptr<::loco::Server> server,
                           const std::string& menu_title,
                           const std::string& window_title,
                           bool open)
        : View(menu_title, window_title, open)
        , _server(server)
{}


void ControlPanel::_reset_refs() {
    _start = false;
    _ref_torque_pct = 0;
    _ref_speed = 0;
    _ref_d_angle_deg = 0;
    _ref_d_current_pct = 0;
    _ref_f_current_pct = 0;
}


void ControlPanel::_update_refs() {
    _server->set_opmode(_opmode);
    _server->toggle_power(_power);
    _server->toggle_start(_start);
    _server->set_ctlmode(_ctlmode);
    _server->set_ctlloop(_ctlloop);
    _server->set_torque(_ref_torque_pct/100.f);
    _server->set_speed(_ref_speed);
    _server->set_d_angle(_ref_d_angle_deg);
    _server->set_d_current(_ref_d_current_pct/100.f);
    _server->toggle_manual_field(_manual_field);
    _server->set_f_current(_ref_f_current_pct/100.f);
}


void ControlPanel::draw() {
    ImGui::Begin(_window_title.c_str(), &_opened);

    _read_keyboard();

    _draw_dash();
    _draw_normal_mode_controls();
    _draw_testing_mode_controls();

    _draw_actions();
    
    _update_refs();

    ImGui::End();
}


void ControlPanel::_read_keyboard() {

}



void ControlPanel::_draw_dash() {
    // Uptime
    ImGui::TextUnformatted(ICON_MDI_TIMER_OUTLINE);
    ImGui::SameLine();
    ImGui::PushItemWidth(120);
    auto uptime = _server->watch_service.string_value("sys", "uptime");
    ImGui::InputText("##uptime", uptime.data(), uptime.size(), ImGuiInputTextFlags_ReadOnly);
    ImGui::PopItemWidth();

    // Heartbeat indicator
    if (_server->heartbeat_service.good()) {
        ui::util::BlinkingText(ICON_MDI_NETWORK, std::chrono::milliseconds{750},
                               ui::colors::icon_green, ui::colors::icon_inactive);
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_red);
        ImGui::TextUnformatted(ICON_MDI_CLOSE_NETWORK);
        ImGui::PopStyleColor();
    }

    // Drive state indicator
    ImGui::SameLine();
    std::string state(drive_state_names.at(_server->drive_state()));
    ImGui::PushItemWidth(120);
    ImGui::InputText("##state", state.data(), state.size(), ImGuiInputTextFlags_ReadOnly);
    ImGui::PopItemWidth();

    // Power On/Off
    ToggleButton(ICON_MDI_POWER_ON" On/Off " ICON_MDI_POWER_OFF, _power, ImVec2{200, 0});
    ImGui::SameLine();
    ImGui::TextDisabled("(F3)");

    // Start/Stop
    ToggleButton(ICON_MDI_PLAY_CIRCLE_OUTLINE" Start/Stop " ICON_MDI_STOP_CIRCLE_OUTLINE, _start, ImVec2{200, 0});
    ImGui::SameLine();
    ImGui::TextDisabled("(F4)");
}


void ControlPanel::_draw_normal_mode_controls() {

}


void ControlPanel::_draw_testing_mode_controls() {

}


void ControlPanel::_draw_actions() {

}


void _draw_popups() {

}




} // namespace loco
} // namespace ui
