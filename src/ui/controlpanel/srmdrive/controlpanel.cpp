#include "controlpanel.h"
#include "../../components/togglebutton.h"
#include <icons_font_awesome/IconsFontAwesome6.h>


namespace ui {
namespace srmdrive {


void ControlPanel::draw(bool& open) {
    ImGui::Begin("Control", &open);

    // power switch
    ToggleButton(ICON_FA_POWER_OFF " Power On/Off", _power_enabled);
    _server->set_power_enabled(_power_enabled);
    
    // run switch
    ToggleButton(ICON_FA_CIRCLE_PLAY " Run On/Off  ", _run_enabled);
    _server->set_run_enabled(_run_enabled);
    
    // torque slider
    ImGui::SliderFloat("Torque [%]", &_torque_percent_ref, -100.0f, 100.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
    _server->set_torque(_torque_percent_ref / 100.0f);
    
    // speed slider
    ImGui::SliderFloat("Speed [rpm]", &_speed_ref, -8000.0f, 8000.0f, "%.f", ImGuiSliderFlags_AlwaysClamp);
    _server->set_speed(_speed_ref);

    // ctlmode
    if (ImGui::TreeNode("Control Mode")) {
        ImGui::RadioButton("Torque", &_ctlmode, std::to_underlying(::srmdrive::ControlMode::torque));
        ImGui::SameLine();
        ImGui::RadioButton("Speed", &_ctlmode, std::to_underlying(::srmdrive::ControlMode::speed));
        ImGui::TreePop();
    }
    _server->set_ctlmode(::srmdrive::ControlMode(_ctlmode));
   
    // actions
    if (ImGui::TreeNode("Actions")) {
        if (ImGui::Button("Calibrate Position Sensor", ImVec2(300, 0))) {
            ImGui::OpenPopup("Warning: calibration");
        }

        if (ImGui::Button("Invert Default Direction", ImVec2(300, 0))) {
            ImGui::OpenPopup("Warning: direction inverting");
        }

        if (ImGui::Button("Reset Driver Fault", ImVec2(300, 0))) {
            _server->exec("ctl", "drive", "reset_driver_fault");
        }

        if (ImGui::Button("Clear Errors", ImVec2(300, 0))) {
            _server->exec("ctl", "sys", "clear_errors");
        }

        if (ImGui::Button("Reset Device", ImVec2(300, 0))) {
            ImGui::OpenPopup("Warning: device reset");
        }

        _draw_popups();
        ImGui::TreePop();
    }

    // field current
    if (ImGui::TreeNode("Field Current")) {
        ImGui::Checkbox("Enable Manual Control", &_fieldctl_enabled);
        ImGui::SliderFloat("Field Current [A]", &_fieldcurr_ref, 0.0f, 50.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::TreePop();
    }
    _server->set_manual_fieldctl_enabled(_fieldctl_enabled);
    _server->set_field_current(_fieldcurr_ref);

    // control loop
    if (ImGui::TreeNode("Control Loop")) {
        ImGui::RadioButton("Closed Loop", &_ctlloop, std::to_underlying(::srmdrive::ControlLoopType::closed));
        ImGui::SameLine();
        ImGui::RadioButton("Open Loop", &_ctlloop, std::to_underlying(::srmdrive::ControlLoopType::open));
        ImGui::SliderFloat("Stator Current [%]", &_statorcurr_ref, 0.0f, 100.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::TreePop();
    }
    _server->set_ctlloop(::srmdrive::ControlLoopType(_ctlloop));
    _server->set_stator_current(_statorcurr_ref / 100.0f);

    // gamma correction
    if (ImGui::TreeNode("Gamma Correction")) {
        if (ImGui::SliderFloat("Gamma Correction [deg]", &_gamma_correction, -180.0f, 180.0f, "%.f", ImGuiSliderFlags_AlwaysClamp)) {
            _server->write("ctl", "drive", "set_gamma_correction", std::to_string(_gamma_correction));
        }
        ImGui::TreePop();
    }

    ImGui::End();
}


void ControlPanel::_draw_popups() {
    // Always center this window when appearing
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Warning: calibration", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Warning! Calibration procedure is about to begin.");
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

    if (ImGui::BeginPopupModal("Warning: direction inverting", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Warning! Default rotation direction will be changed.");
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

    if (ImGui::BeginPopupModal("Warning: device reset", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Warning! Device will be reset.");
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
