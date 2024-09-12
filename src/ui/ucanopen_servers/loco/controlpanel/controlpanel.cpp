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

    _ctlmode = static_cast<ControlMode>(_ctlmode_v);
    _server->set_ctlmode(_ctlmode);

    _ctlloop = static_cast<ControlLoop>(_ctlloop_v);
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

    // Operation mode selection
    ImGui::PushItemWidth(200);
    auto opmode_preview = opmode_names.at(_opmode).data();
    if (ImGui::BeginCombo("Operating Mode", opmode_preview)) {
        for (const auto& mode : opmode_names) {
            bool is_selected = (mode.first == _opmode);
            if (ImGui::Selectable(mode.second.data(), is_selected)) {
                _opmode = mode.first;
                _reset_refs();
            }
        }
        ImGui::EndCombo();
    }
    ImGui::PopItemWidth();

    // Power On/Off
    ToggleButton(ICON_MDI_POWER_ON"   On/Off   " ICON_MDI_POWER_OFF, _power, ImVec2{200, 0});
    ImGui::SameLine();
    ImGui::TextDisabled("(F3)");

    // Start/Stop
    ToggleButton(ICON_MDI_PLAY_CIRCLE_OUTLINE" Start/Stop " ICON_MDI_STOP_CIRCLE_OUTLINE, _start, ImVec2{200, 0});
    ImGui::SameLine();
    ImGui::TextDisabled("(F4)");
}


void ControlPanel::_draw_normal_mode_controls() {
    bool enabled = _server->opmode() == OperatingMode::normal;
    if (enabled) {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_green);
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_red);
    }
    ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED); 
    ImGui::PopStyleColor();

    bool selected = _opmode == OperatingMode::normal;
    ui::util::Switchable run_mode_header(selected, []() {
        ImGui::SameLine();
        ImGui::SeparatorText("Normal Mode");
    });

    if (!selected) {
        return;
    }

    // torque input
    ImGui::RadioButton("##torque_ctlmode", &_ctlmode_v, std::to_underlying(ControlMode::torque));
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNone)) {
        ImGui::SetTooltip("Torque mode");
    }
    ImGui::SameLine();

    ImGui::PushItemWidth(200);
    if (ImGui::InputFloat("Torque [%]", &_ref_torque_pct, 1.0f, 100.0f, "%.2f", ImGuiInputTextFlags_EnterReturnsTrue)) {
        _ref_torque_pct = std::clamp(_ref_torque_pct, -100.0f, 100.0f);
    }
    if (_ctlmode != ControlMode::torque) {
        _ref_torque_pct = 0;
    }
    ImGui::PopItemWidth();

    // speed input
    ImGui::RadioButton("##speed_ctlmode", &_ctlmode_v, std::to_underlying(ControlMode::speed));
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNone)) {
        ImGui::SetTooltip("Speed mode");
    }
    ImGui::SameLine();

    ImGui::PushItemWidth(200);
    if (ImGui::InputScalar("Speed [rpm]", ImGuiDataType_S16,  &_ref_speed, NULL, NULL, NULL, ImGuiInputTextFlags_EnterReturnsTrue)) {
        _ref_speed = std::clamp(_ref_speed, int16_t(-5000), int16_t(5000));
    }
    if (_ctlmode != ControlMode::speed) {
        _ref_speed = 0;
    }
    ImGui::PopItemWidth();

    // field current
    ImGui::Checkbox("##manual_field", &_manual_field);
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNone)) {
        ImGui::SetTooltip("Manual field current control");
    }
    ImGui::SameLine();
    ui::util::Switchable manual_field_controller(_manual_field, [&](){
        ImGui::PushItemWidth(200);
        if (ImGui::InputFloat("Field Current [%]", &_ref_f_current_pct, 1.0f, 100.0f, "%.1f", ImGuiInputTextFlags_EnterReturnsTrue)) {
            _ref_f_current_pct = std::clamp(_ref_f_current_pct, 0.f, 100.f);
        }
        if (!_manual_field) {
            _ref_f_current_pct = 0;
        }
        ImGui::PopItemWidth();
    });

    // control loop
    if (ImGui::TreeNode("Control Loop")) {
        ImGui::RadioButton("Closed Loop", &_ctlloop_v, std::to_underlying(ControlLoop::closed));
        if (ImGui::RadioButton("Open Loop", &_ctlloop_v, std::to_underlying(ControlLoop::open))) {
            _ref_d_current_pct = std::clamp(_ref_d_current_pct, 0.0f, 100.0f);
        }
        ImGui::RadioButton("Semi-Closed Loop", &_ctlloop_v, std::to_underlying(ControlLoop::semiclosed));
        ImGui::PushItemWidth(200);

        switch (_ctlloop) {
        case ControlLoop::closed:
            // DO NOTHING
            break;
        case ControlLoop::open:
            if (ImGui::InputFloat("D-Current [%]", &_ref_d_current_pct, 0.1f, 100.0f, "%.1f", ImGuiInputTextFlags_EnterReturnsTrue)) {
                _ref_d_current_pct = std::clamp(_ref_d_current_pct, 0.0f, 100.0f);
            }

            if (_ref_speed == 0) {
                if (ImGui::InputScalar("Angle [deg]", ImGuiDataType_S16, &_ref_d_angle_deg, NULL, NULL, NULL, ImGuiInputTextFlags_EnterReturnsTrue)) {
                    // _openloop_ref_angle = std::clamp(_openloop_ref_angle, 0, 360);
                }
            }

            break;
        case ControlLoop::semiclosed:
            if (ImGui::InputFloat("D-Current [%]", &_ref_d_current_pct, 0.1f, 100.0f, "%.1f", ImGuiInputTextFlags_EnterReturnsTrue)) {
                _ref_d_current_pct = std::clamp(_ref_d_current_pct, -100.0f, 100.0f);
            }           
            break;
        }

        ImGui::PopItemWidth();
        ImGui::TreePop();
    }
}


void ControlPanel::_draw_testing_mode_controls() {

}


void ControlPanel::_draw_actions() {
    ImGui::SeparatorText("");

    ImGui::TextUnformatted(ICON_MDI_CAR_WRENCH);
    ImGui::SameLine();
    if (ImGui::TreeNode("Service Actions")) {
        if (ImGui::Button(ICON_MDI_SHIELD_REFRESH" Clear Errors             ")) {
            _server->exec("ctl", "sys", "clear_errors");
        }

        if (ImGui::Button(ICON_MDI_RESTART" Reset MCU                ")) {
            _server->exec("ctl", "sys", "reset_device");
        }

        // if (ImGui::Button(ICON_MDI_COMPASS_OUTLINE" Calibrate Angle Sensor   ")) {
        //     _server->exec("ctl", "angsens", "calibrate");
        // }

        // if (ImGui::Button(ICON_MDI_CONTENT_SAVE_OUTLINE" Save Calibration Results ")) {
        //     _server->exec("ctl", "angsens", "save_calibration");
        // }
        
        ImGui::TreePop();
    }
}


void _draw_popups() {

}




} // namespace loco
} // namespace ui
