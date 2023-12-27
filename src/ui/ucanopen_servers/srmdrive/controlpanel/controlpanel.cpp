#include "controlpanel.h"
#include "imgui.h"
#include <ui/components/togglebutton.h>


namespace ui {
namespace srmdrive {


ControlPanel::ControlPanel(std::shared_ptr<::srmdrive::Server> server,
                           const std::string& menu_title,
                           const std::string& window_title,
                           bool show_by_default)
        : View(menu_title, window_title, show_by_default)
        , _server(server)
{}


void ControlPanel::draw() {
    ImGui::Begin(_window_title.c_str(), &show);

    _read_keyboard();

    // emergency
    ToggleButton(ICON_MDI_ALERT_OCTAGON_OUTLINE" Emergency   ", _emergency);
    ImGui::SameLine();
    ImGui::TextDisabled("(F2)");
    _server->set_emergency_enabled(_emergency);

    // power switch
    ToggleButton(ICON_MDI_CAR_BATTERY" Power On/Off", _power_enabled);
    ImGui::SameLine();
    ImGui::TextDisabled("(F3)");
    _server->set_power_enabled(_power_enabled);
    
    // run switch
    ToggleButton(ICON_MDI_POWER" Run On/Off  ", _run_enabled);
    ImGui::SameLine();
    ImGui::TextDisabled("(F4)");
    _server->set_run_enabled(_run_enabled);

    ImGui::Separator();
    
    // torque input
    ImGui::RadioButton("##torque_ctlmode", &_ctlmode, std::to_underlying(::srmdrive::ControlMode::torque));
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNone)) {
        ImGui::SetTooltip("Torque Mode");
    }
    ImGui::SameLine();

    ImGui::PushItemWidth(200);
    if (ImGui::InputFloat("Torque [%]", &_torque_percent_ref, 1.0f, 100.0f, "%.2f", ImGuiInputTextFlags_EnterReturnsTrue)) {
        _torque_percent_ref = std::clamp(_torque_percent_ref, -100.0f, 100.0f);
    }
    ImGui::PopItemWidth();
    _server->set_torque(_torque_percent_ref / 100.0f);

    // speed input
    ImGui::RadioButton("##speed_ctlmode", &_ctlmode, std::to_underlying(::srmdrive::ControlMode::speed));
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNone)) {
        ImGui::SetTooltip("Speed Mode");
    }
    ImGui::SameLine();

    ImGui::PushItemWidth(200);
    if (ImGui::InputFloat("Speed [rpm]", &_speed_ref, 1.0f, 100.0f, "%.f", ImGuiInputTextFlags_EnterReturnsTrue)) {
        _speed_ref = std::clamp(_speed_ref, -8000.0f, 8000.0f);
    }
    ImGui::PopItemWidth();
    _server->set_speed(_speed_ref);

    _server->set_ctlmode(::srmdrive::ControlMode(_ctlmode));

    ImGui::Separator();
   
    // actions
    if (ImGui::TreeNode("Actions")) {
        if (ImGui::Button("Calibrate Position Sensor", ImVec2(300, 0))) {
            ImGui::OpenPopup("Warning!##calibration");
        }

        if (ImGui::Button("Invert Default Direction", ImVec2(300, 0))) {
            ImGui::OpenPopup("Warning!##direction");
        }

        if (ImGui::Button("Reset Driver Fault", ImVec2(300, 0))) {
            _server->exec("ctl", "drive", "reset_driver_fault");
        }

        if (ImGui::Button("Clear Errors", ImVec2(300, 0))) {
            _server->exec("ctl", "sys", "clear_errors");
        }

        if (ImGui::Button("Reset Device", ImVec2(300, 0))) {
            ImGui::OpenPopup("Warning!##device_reset");
        }

        _draw_popups();
        ImGui::TreePop();
    }

    // field current
    if (ImGui::TreeNode("Field Current")) {
        ImGui::Checkbox("Enable Manual Control", &_fieldctl_enabled);
        ImGui::PushItemWidth(200);
        if (ImGui::InputFloat("Field Current [A]", &_fieldcurr_ref, 0.1f, 100.0f, "%.1f", ImGuiInputTextFlags_EnterReturnsTrue)) {
            _fieldcurr_ref = std::clamp(_fieldcurr_ref, 0.0f, 50.0f);
        }
        ImGui::PopItemWidth();
        ImGui::TreePop();
    }
    _server->set_manual_fieldctl_enabled(_fieldctl_enabled);
    _server->set_field_current(_fieldcurr_ref);

    // control loop
    if (ImGui::TreeNode("Control Loop")) {
        ImGui::RadioButton("Closed Loop", &_ctlloop, std::to_underlying(::srmdrive::ControlLoop::closed));
        if (ImGui::RadioButton("Open Loop", &_ctlloop, std::to_underlying(::srmdrive::ControlLoop::open))) {
            _dcurr_ref = std::clamp(_dcurr_ref, 0.0f, 100.0f);
        }
        ImGui::RadioButton("Semi-Closed Loop", &_ctlloop, std::to_underlying(::srmdrive::ControlLoop::semiclosed));
        ImGui::PushItemWidth(200);

        switch (_ctlloop) {
        case std::to_underlying(::srmdrive::ControlLoop::closed):
            // DO NOTHING
            break;
        case std::to_underlying(::srmdrive::ControlLoop::open):
            if (ImGui::InputFloat("D-Current [%]", &_dcurr_ref, 0.1f, 100.0f, "%.1f", ImGuiInputTextFlags_EnterReturnsTrue)) {
                _dcurr_ref = std::clamp(_dcurr_ref, 0.0f, 100.0f);
            }
            break;
        case std::to_underlying(::srmdrive::ControlLoop::semiclosed):
            if (ImGui::InputFloat("D-Current [%]", &_dcurr_ref, 0.1f, 100.0f, "%.1f", ImGuiInputTextFlags_EnterReturnsTrue)) {
                _dcurr_ref = std::clamp(_dcurr_ref, -100.0f, 100.0f);
            }           
            break;
        }

        ImGui::PopItemWidth();
        ImGui::TreePop();
    }
    _server->set_ctlloop(::srmdrive::ControlLoop(_ctlloop));
    _server->set_d_current(_dcurr_ref / 100.0f);

    // gamma correction
    if (ImGui::TreeNode("Gamma Correction")) {
        ImGui::PushItemWidth(200);
        if (ImGui::InputFloat("Gamma Correction [deg]", &_gamma_correction, 1.0f, 100.0f, "%.f", ImGuiInputTextFlags_EnterReturnsTrue)) {
            _gamma_correction = std::clamp(_gamma_correction, -180.0f, 180.0f);
            _server->write("ctl", "drive", "set_gamma_correction", std::to_string(_gamma_correction));
        }
        ImGui::PopItemWidth();
        ImGui::TreePop();
    }

    ImGui::End();
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


void ControlPanel::_read_keyboard() {
    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_F2))) {
        _emergency = !_emergency;
    }

    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_F3))) {
        _power_enabled = !_power_enabled;
    }

    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_F4))) {
        _run_enabled = !_run_enabled;
    }
}


} // namespace srmdrive
} // namespace ui
