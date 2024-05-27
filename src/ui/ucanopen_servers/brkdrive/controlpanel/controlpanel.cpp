#include "controlpanel.h"
#include "imgui.h"
#include <ui/util/togglebutton.h>
#include <ui/util/util.h>


namespace ui {
namespace brkdrive {


ControlPanel::ControlPanel(std::shared_ptr<::brkdrive::Server> server,
                           const std::string& menu_title,
                           const std::string& window_title,
                           bool open)
        : View(menu_title, window_title, open)
        , _server(server)
{}


void ControlPanel::draw() {
    ImGui::Begin(_window_title.c_str(), &_opened);

    _read_keyboard();

    _draw_dash();    
    _draw_normal_mode_controls();
    _draw_run_mode_controls();
    _draw_track_mode_controls();
    _draw_hwtest_mode_controls();

    _draw_actions();

    ImGui::End();
}


void ControlPanel::_draw_dash() {
    // Uptime
    ImGui::TextUnformatted(ICON_MDI_TIMER_OUTLINE" Uptime[s]:");
    ImGui::SameLine();
    ImGui::TextUnformatted(_server->watch_service.string_value("sys", "uptime").c_str());

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
    std::string state(_server->drive_state());
    ImGui::PushItemWidth(90);
    ImGui::InputText("##state", state.data(), state.size(), ImGuiInputTextFlags_ReadOnly);
    ImGui::PopItemWidth();

    // Operation mode selection
    ImGui::PushItemWidth(132);
    auto opmode_preview = ::brkdrive::opmode_map.at(_opmode).data();
    if (ImGui::BeginCombo("Operation Mode", opmode_preview)) {
        for (const auto& mode : ::brkdrive::opmode_map) {
            bool is_selected = (mode.first == _opmode);
            if (ImGui::Selectable(mode.second.data(), is_selected)) {
                _opmode = mode.first;
                _brake_ref_pu = 0;
                _torque_ref_pct = 0;
                _speed_ref = 0;
                _dcurr_ref_pu = 0;
                _openloop_angle_ref = 0;
                _angle_ref = 0;
                _track_speed = 0;
                _run = false;
            }
        }
        ImGui::EndCombo();
    }
    ImGui::PopItemWidth();
    _server->set_opmode(static_cast<::brkdrive::OperationMode>(_opmode));

    // wakeup
    ToggleButton(ICON_MDI_POWER" On/Off    ", _wakeup);
    ImGui::SameLine();
    ImGui::TextDisabled("(F3)");
    _server->toggle_wakeup(_wakeup);

    // start/stop
    ui::util::Switchable run_button(_opmode != std::to_underlying(::brkdrive::OperationMode::normal), [this](){
        ToggleButton(ICON_MDI_POWER" Start/Stop", _run);
        ImGui::SameLine();
        ImGui::TextDisabled("(F4)");
    });
    _server->toggle_run(_run);
}


void ControlPanel::_read_keyboard() {
    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_F3))) {
        _wakeup = !_wakeup;
    }

    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_F4))) {
        _run = !_run;
    }
}


void ControlPanel::_draw_normal_mode_controls() {
    bool enabled = _opmode == std::to_underlying(::brkdrive::OperationMode::normal);
    if (enabled) {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_green);
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_red);
    }
    ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED); 
    ImGui::PopStyleColor();
    ImGui::SameLine();
    ImGui::SeparatorText("Normal Mode");

    ui::util::Switchable normal_mode_controls(enabled, [this](){
        ImGui::PushItemWidth(200);
        ImGui::SliderFloat("Brake Pressure [%]", &this->_brake_ref_pu, 0.0f, 100.0f, "%.2f");
        ImGui::PopItemWidth();
        _server->set_brake_ref(_brake_ref_pu/100.0f);
    });
}


void ControlPanel::_draw_run_mode_controls() {
    bool enabled = _opmode == std::to_underlying(::brkdrive::OperationMode::run);
    if (enabled) {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_green);
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_red);
    }
    ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED); 
    ImGui::PopStyleColor();
    ImGui::SameLine();
    ImGui::SeparatorText("Run Mode");

    ui::util::Switchable run_mode_controls(_opmode == std::to_underlying(::brkdrive::OperationMode::run), [this](){
        // torque input
        ImGui::RadioButton("##torque_ctlmode", &_ctlmode, std::to_underlying(::brkdrive::ControlMode::torque));
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNone)) {
            ImGui::SetTooltip("Torque Mode");
        }
        ImGui::SameLine();

        ImGui::PushItemWidth(200);
        if (ImGui::InputFloat("Torque [%]", &_torque_ref_pct, 1.0f, 100.0f, "%.2f", ImGuiInputTextFlags_EnterReturnsTrue)) {
            _torque_ref_pct = std::clamp(_torque_ref_pct, -100.0f, 100.0f);
        }
        if (_ctlmode != std::to_underlying(::brkdrive::ControlMode::torque)) {
            _torque_ref_pct = 0;
        }
        ImGui::PopItemWidth();

        // speed input
        ImGui::RadioButton("##speed_ctlmode", &_ctlmode, std::to_underlying(::brkdrive::ControlMode::speed));
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNone)) {
            ImGui::SetTooltip("Speed Mode");
        }
        ImGui::SameLine();

        ImGui::PushItemWidth(200);
        if (ImGui::InputFloat("Speed [rpm]", &_speed_ref, 1.0f, 100.0f, "%.f", ImGuiInputTextFlags_EnterReturnsTrue)) {
            _speed_ref = std::clamp(_speed_ref, -5000.0f, 5000.0f);
        }
        if (_ctlmode != std::to_underlying(::brkdrive::ControlMode::speed)) {
            _speed_ref = 0;
        }
        ImGui::PopItemWidth();

        // control loop
        if (ImGui::TreeNode("Control Loop")) {
            ImGui::RadioButton("Closed Loop", &_ctlloop, std::to_underlying(::brkdrive::ControlLoop::closed));
            if (ImGui::RadioButton("Open Loop", &_ctlloop, std::to_underlying(::brkdrive::ControlLoop::open))) {
                _dcurr_ref_pu = std::clamp(_dcurr_ref_pu, 0.0f, 100.0f);
            }
            ImGui::RadioButton("Semi-Closed Loop", &_ctlloop, std::to_underlying(::brkdrive::ControlLoop::semiclosed));
            ImGui::PushItemWidth(200);

            switch (_ctlloop) {
            case std::to_underlying(::brkdrive::ControlLoop::closed):
                // DO NOTHING
                break;
            case std::to_underlying(::brkdrive::ControlLoop::open):
                if (ImGui::InputFloat("D-Current [%]", &_dcurr_ref_pu, 0.1f, 100.0f, "%.1f", ImGuiInputTextFlags_EnterReturnsTrue)) {
                    _dcurr_ref_pu = std::clamp(_dcurr_ref_pu, 0.0f, 100.0f);
                }

                if (_speed_ref == 0) {
                    ImGui::PushItemWidth(200);
                    if (ImGui::InputInt("Angle [deg]", &_openloop_angle_ref, 1, 100, ImGuiInputTextFlags_EnterReturnsTrue)) {
                        _openloop_angle_ref = std::clamp(_openloop_angle_ref, 0, 360);
                    }
                    ImGui::PopItemWidth();
                }

                break;
            case std::to_underlying(::brkdrive::ControlLoop::semiclosed):
                if (ImGui::InputFloat("D-Current [%]", &_dcurr_ref_pu, 0.1f, 100.0f, "%.1f", ImGuiInputTextFlags_EnterReturnsTrue)) {
                    _dcurr_ref_pu = std::clamp(_dcurr_ref_pu, -100.0f, 100.0f);
                }           
                break;
            }

            ImGui::PopItemWidth();
            ImGui::TreePop();
        }

        _server->set_torque_ref(_torque_ref_pct / 100.0f);
        _server->set_speed_ref(_speed_ref);
        _server->set_ctlmode(::brkdrive::ControlMode(_ctlmode));
        _server->set_ctlloop(::brkdrive::ControlLoop(_ctlloop));
        _server->set_dcurr_ref(_dcurr_ref_pu / 100.0f);
        _server->set_openloop_angle_ref(_openloop_angle_ref);
    });
}


void ControlPanel::_draw_track_mode_controls() {
    bool enabled = _opmode == std::to_underlying(::brkdrive::OperationMode::track);
    if (enabled) {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_green);
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_red);
    }
    ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED); 
    ImGui::PopStyleColor();
    ImGui::SameLine();
    ImGui::SeparatorText("Track Mode");

    ui::util::Switchable track_mode_controls(_opmode == std::to_underlying(::brkdrive::OperationMode::track), [this](){
        ImGui::PushItemWidth(200);
        if (ImGui::InputInt("Angle [deg]", &_angle_ref, 1, 100, ImGuiInputTextFlags_EnterReturnsTrue)) {
            _openloop_angle_ref = std::clamp(_angle_ref, 0, 10000);
        }
        ImGui::PopItemWidth();

        ImGui::PushItemWidth(200);
        if (ImGui::InputFloat("Track Speed [rpm]", &_track_speed, 1.0f, 100.0f, "%.f", ImGuiInputTextFlags_EnterReturnsTrue)) {
            _track_speed = std::clamp(_track_speed, 0.0f, 5000.0f);
        }
        ImGui::PopItemWidth();
        
        _server->set_angle_ref(_openloop_angle_ref);
        _server->set_track_speed(_track_speed);
    });
}


void ControlPanel::_draw_hwtest_mode_controls() {
    bool enabled = _opmode == std::to_underlying(::brkdrive::OperationMode::hwtest);
    if (enabled) {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_green);
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_red);
    }
    ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED); 
    ImGui::PopStyleColor();
    ImGui::SameLine();
    ImGui::SeparatorText("Hardware Test Mode");

    ui::util::Switchable hwtest_mode_controls(_opmode == std::to_underlying(::brkdrive::OperationMode::hwtest), [this](){
    });
}


void ControlPanel::_draw_actions() {
    ImGui::SeparatorText("");

    ImGui::TextUnformatted(ICON_MDI_CAR_WRENCH);
    ImGui::SameLine();
    if (ImGui::TreeNode("Service Actions")) {
        if (ImGui::Button(ICON_MDI_SHIELD_REFRESH" Clear Errors       ")) {
            _server->exec("ctl", "sys", "clear_errors");
        }

        if (ImGui::Button(ICON_MDI_RESTART" Reset VCU          ")) {
            _server->exec("ctl", "sys", "reset_device");
        }


        ImGui::TreePop();
    }
}


















void ControlPanel::_draw_popups() {
    // Always center this window when appearing
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Warning!##calibration", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Calibration procedure is about to begin.");
        ImGui::Separator();

        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Ok", ImVec2(120, 0))) {
            _server->exec("ctl", "drive", "calibrate");
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal("Warning!##direction", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Default rotation direction will be changed.");
        ImGui::Separator();

        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Ok", ImVec2(120, 0))) {
            _server->exec("ctl", "drive", "invert_rotdir");
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal("Warning!##device_reset", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Device will be reset.");
        ImGui::Separator();

        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Ok", ImVec2(120, 0))) {
            _server->exec("ctl", "sys", "reset_device");
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}


} // namespace srmdrive
} // namespace ui
