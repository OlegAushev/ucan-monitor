#include "motorcontrolpanel.h"
#include <ui/util/togglebutton.h>


namespace ui {
namespace atvvcu {


MotorControlPanel::MotorControlPanel(std::shared_ptr<::atvvcu::Server> server,
                                     const std::string& menu_title,
                                     const std::string& window_title,
                                     bool open)
        : View(menu_title, window_title, open)
        , _server(server)
{}


void MotorControlPanel::draw() {
    for (size_t i = 0; i < 4; ++i) {
        ImGui::Begin(_window_titles[i].c_str());

        if (!_server->drive.debug_mode()) { ImGui::BeginDisabled(); }

        ImGui::SeparatorText(ICON_MDI_CAMERA_CONTROL" Control Mode");
        ImGui::RadioButton("Speed", &_ref_ctlmode[i], 0);
        ImGui::SameLine();
        ImGui::RadioButton("Torque", &_ref_ctlmode[i], 1);

        ImGui::SeparatorText(ICON_MDI_CAR_SHIFT_PATTERN" Gear");

        //if (_server->gear_selector.debug_mode()) { ImGui::BeginDisabled(); }
        ImGui::RadioButton("R", &_ref_gear[i], 2);
        ImGui::RadioButton("N", &_ref_gear[i], 0);
        ImGui::RadioButton("D", &_ref_gear[i], 1);
        //if (_server->gear_selector.debug_mode()) { ImGui::EndDisabled(); }

        ImGui::SeparatorText("");
        ui::ToggleButton(ICON_MDI_ELECTRIC_SWITCH" Relay ", _ref_mainrelay[i]);
        ImGui::SameLine();
        ui::ToggleButton(ICON_MDI_POWER" Enable", _ref_enable[i]);     
        ui::ToggleButton(ICON_MDI_FLASH_OFF_OUTLINE" Discharge  ", _ref_discharge[i]);

        ui::ToggleButton(ICON_MDI_CAR_BRAKE_ALERT" FBrake", _ref_footbrake[i]);
        ImGui::SameLine();
        ui::ToggleButton(ICON_MDI_CAR_BRAKE_HOLD" HBrake", _ref_handbrake[i]);

        ui::ToggleButton(ICON_MDI_ALERT_CIRCLE_OUTLINE" Fault Reset", _ref_faultreset[i]);

        ImGui::PushItemWidth(150);
        if (ImGui::InputFloat("Speed [rpm]", &_ref_speed[i], 1.0f, 100.0f, "%.0f", ImGuiInputTextFlags_EnterReturnsTrue)) {
            _ref_speed[i] = std::clamp(_ref_speed[i], -10000.0f, 10000.0f);
        }

        if (ImGui::InputFloat("Torque [Nm]", &_ref_torque[i], 1.0f, 100.0f, "%.1f", ImGuiInputTextFlags_EnterReturnsTrue)) {
            _ref_torque[i] = std::clamp(_ref_torque[i], -400.0f, 400.0f);
        }
        ImGui::PopItemWidth();

        if (ImGui::TreeNode("Torque Limits")) {
            ImGui::PushItemWidth(150);
            if (ImGui::InputInt("El Torque [Nm]", &_ref_eltorque_max, 1, 100, ImGuiInputTextFlags_EnterReturnsTrue)) {
                _ref_eltorque_max = std::clamp(_ref_eltorque_max, 0, 400);
                _server->write("ctl", "motordrive", "eltorque_max", std::to_string(_ref_eltorque_max));
            }

            if (ImGui::InputInt("Brk Torque [Nm]", &_ref_braketorque_max, 1, 100, ImGuiInputTextFlags_EnterReturnsTrue)) {
                _ref_braketorque_max = std::clamp(_ref_braketorque_max, -400, 400);
                _server->write("ctl", "motordrive", "braketorque_max", std::to_string(_ref_braketorque_max));
            }
            ImGui::PopItemWidth();

            ImGui::TreePop();
        }

        if (!_server->drive.debug_mode()) { ImGui::EndDisabled(); }

        ImGui::End();
    }

    _server->drive.set_ref_ctlmode(_ref_ctlmode);
    _server->drive.set_ref_enable(_ref_enable);
    _server->drive.set_ref_discharge(_ref_discharge);
    _server->drive.set_ref_mainrelay(_ref_mainrelay);
    _server->drive.set_ref_footbrake(_ref_footbrake);
    _server->drive.set_ref_handbrake(_ref_handbrake);
    _server->drive.set_ref_faultreset(_ref_faultreset);
    _server->drive.set_ref_gear(_ref_gear);
    _server->drive.set_ref_speed(_ref_speed);
    _server->drive.set_ref_torque(_ref_torque);
}


} // namespace atvvcu
} // namespace ui
