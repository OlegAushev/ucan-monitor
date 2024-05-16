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

    ImGui::PushItemWidth(200);
    auto opmode_preview = ::brkdrive::opmode_map.at(_opmode).data();
    if (ImGui::BeginCombo("Operation Mode", opmode_preview)) {
        for (const auto& mode : ::brkdrive::opmode_map) {
            bool is_selected = (mode.first == _opmode);
            if (ImGui::Selectable(mode.second.data(), is_selected)) {
                _opmode = mode.first;
                _run = false;
            }
        }
        ImGui::EndCombo();
    }
    ImGui::PopItemWidth();

    // wakeup
    ToggleButton(ICON_MDI_POWER" On/Off    ", _wakeup);
    ImGui::SameLine();
    ImGui::TextDisabled("(F3)");
    _server->toggle_wakeup(_wakeup);

    ui::util::Switchable normal_mode_controls(_opmode == std::to_underlying(::brkdrive::OperationMode::normal), [this](){
        ImGui::SeparatorText("Normal Mode");
        
        ImGui::PushItemWidth(200);
        ImGui::SliderFloat("Brake Pressure", &this->_brake_ref_pu, 0.0f, 1.0f);
        ImGui::PopItemWidth();
        _server->set_brake_ref(_brake_ref_pu);
    });
    
    ui::util::Switchable run_mode_controls(_opmode == std::to_underlying(::brkdrive::OperationMode::run), [this](){
        ImGui::SeparatorText("Run Mode");
        
        if (_opmode == std::to_underlying(::brkdrive::OperationMode::run)) {
            ToggleButton(ICON_MDI_POWER" Start/Stop", _run);
            ImGui::SameLine();
            ImGui::TextDisabled("(F4)");
            _server->toggle_run(_run);
        }

        // torque input
        ImGui::RadioButton("##torque_ctlmode", &_ctlmode, std::to_underlying(::brkdrive::ControlMode::torque));
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNone)) {
            ImGui::SetTooltip("Torque Mode");
        }
        ImGui::SameLine();

        ImGui::PushItemWidth(200);
        if (ImGui::InputFloat("Torque [%]", &_torque_percent_ref, 1.0f, 100.0f, "%.2f", ImGuiInputTextFlags_EnterReturnsTrue)) {
            _torque_percent_ref = std::clamp(_torque_percent_ref, -100.0f, 100.0f);
        }
        if (_ctlmode != std::to_underlying(::brkdrive::ControlMode::torque)) {
            _torque_percent_ref = 0;
        }
        ImGui::PopItemWidth();

        _server->set_torque_ref(_torque_percent_ref / 100.0f);

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

        _server->set_speed_ref(_speed_ref);
        _server->set_ctlmode(::brkdrive::ControlMode(_ctlmode));

        // control loop
        if (ImGui::TreeNode("Control Loop")) {
            ImGui::RadioButton("Closed Loop", &_ctlloop, std::to_underlying(::brkdrive::ControlLoop::closed));
            if (ImGui::RadioButton("Open Loop", &_ctlloop, std::to_underlying(::brkdrive::ControlLoop::open))) {
                _dcurr_ref = std::clamp(_dcurr_ref, 0.0f, 100.0f);
            }
            ImGui::RadioButton("Semi-Closed Loop", &_ctlloop, std::to_underlying(::brkdrive::ControlLoop::semiclosed));
            ImGui::PushItemWidth(200);

            switch (_ctlloop) {
            case std::to_underlying(::brkdrive::ControlLoop::closed):
                // DO NOTHING
                break;
            case std::to_underlying(::brkdrive::ControlLoop::open):
                if (ImGui::InputFloat("D-Current [%]", &_dcurr_ref, 0.1f, 100.0f, "%.1f", ImGuiInputTextFlags_EnterReturnsTrue)) {
                    _dcurr_ref = std::clamp(_dcurr_ref, 0.0f, 100.0f);
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
                if (ImGui::InputFloat("D-Current [%]", &_dcurr_ref, 0.1f, 100.0f, "%.1f", ImGuiInputTextFlags_EnterReturnsTrue)) {
                    _dcurr_ref = std::clamp(_dcurr_ref, -100.0f, 100.0f);
                }           
                break;
            }

            ImGui::PopItemWidth();
            ImGui::TreePop();
        }

        _server->set_ctlloop(::brkdrive::ControlLoop(_ctlloop));
        _server->set_dcurr_ref(_dcurr_ref / 100.0f);
        _server->set_openloop_angle_ref(_openloop_angle_ref);
    });
    
    
    ui::util::Switchable track_mode_controls(_opmode == std::to_underlying(::brkdrive::OperationMode::track), [this](){
        ImGui::SeparatorText("Track Mode");
        
        if (_opmode == std::to_underlying(::brkdrive::OperationMode::track)) {
            ToggleButton(ICON_MDI_POWER" Start/Stop", _run);
            ImGui::SameLine();
            ImGui::TextDisabled("(F4)");
            _server->toggle_run(_run);
        }
    });
    

    ui::util::Switchable hwtest_mode_controls(_opmode == std::to_underlying(::brkdrive::OperationMode::hwtest), [this](){
        ImGui::SeparatorText("HW-test Mode");

        if (_opmode == std::to_underlying(::brkdrive::OperationMode::hwtest)) {
            ToggleButton(ICON_MDI_POWER" Start/Stop", _run);
            ImGui::SameLine();
            ImGui::TextDisabled("(F4)");
            _server->toggle_run(_run);
        }
    });





   
    // // actions
    // if (ImGui::TreeNode("Actions")) {
    //     if (ImGui::Button("Calibrate Position Sensor", ImVec2(300, 0))) {
    //         ImGui::OpenPopup("Warning!##calibration");
    //     }

    //     if (ImGui::Button("Invert Default Direction", ImVec2(300, 0))) {
    //         ImGui::OpenPopup("Warning!##direction");
    //     }

    //     if (ImGui::Button("Reset Driver Fault", ImVec2(300, 0))) {
    //         _server->exec("ctl", "drive", "reset_driver_fault");
    //     }

    //     if (ImGui::Button("Clear Errors", ImVec2(300, 0))) {
    //         _server->exec("ctl", "sys", "clear_errors");
    //     }

    //     if (ImGui::Button("Reset Device", ImVec2(300, 0))) {
    //         ImGui::OpenPopup("Warning!##device_reset");
    //     }

    //     _draw_popups();
    //     ImGui::TreePop();
    // }

    // // field current
    // if (ImGui::TreeNode("Field Current")) {
    //     ImGui::Checkbox("Enable Manual Control", &_fieldctl_enabled);
    //     ImGui::PushItemWidth(200);
    //     if (ImGui::InputFloat("Field Current [A]", &_fieldcurr_ref, 0.1f, 100.0f, "%.1f", ImGuiInputTextFlags_EnterReturnsTrue)) {
    //         _fieldcurr_ref = std::clamp(_fieldcurr_ref, 0.0f, 50.0f);
    //     }
    //     if (!_fieldctl_enabled) {
    //         _fieldcurr_ref = 0;
    //     }

    //     ImGui::PopItemWidth();
    //     ImGui::TreePop();
    // }

    // _server->set_manual_fieldctl_enabled(_fieldctl_enabled);
    // _server->set_field_current(_fieldcurr_ref);


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
    // if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_F2))) {
    //     _emergency = !_emergency;
    // }

    // if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_F3))) {
    //     _power_enabled = !_power_enabled;
    // }

    // if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_F4))) {
    //     _run_enabled = !_run_enabled;
    // }
}


} // namespace srmdrive
} // namespace ui
