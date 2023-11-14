#include "motorcontrolpanel.h"
#include "imgui.h"
#include <algorithm>
#include <string>
#include <ui/components/togglebutton.h>


namespace ui {
namespace atvvcu {


MotorControlPanel::MotorControlPanel(std::shared_ptr<::atvvcu::Server> server,
                                     const std::string& menu_title,
                                     const std::string& window_title,
                                     bool show_by_default)
        : View(menu_title, window_title, show_by_default)
        , _server(server)
{}


void MotorControlPanel::draw([[maybe_unused]] bool& open) {
    for (size_t i = 0; i < 4; ++i) {
        ImGui::Begin(_window_titles[i].c_str());

        ImGui::SeparatorText(ICON_MDI_CAMERA_CONTROL" Control Mode");
        ImGui::RadioButton("Speed", &_ref_ctlmode[i], 0);
        ImGui::SameLine();
        ImGui::RadioButton("Torque", &_ref_ctlmode[i], 1);

        ImGui::SeparatorText(ICON_MDI_CAR_SHIFT_PATTERN" Gear");
        ImGui::RadioButton("Neutral", &_ref_gear[i], 0);
        ImGui::SameLine();
        ImGui::RadioButton("Forward", &_ref_gear[i], 1);
        ImGui::SameLine();
        ImGui::RadioButton("Reverse", &_ref_gear[i], 2);

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
        if (ImGui::InputInt("Speed [rpm]", &_ref_speed[i], 1, 100, ImGuiInputTextFlags_EnterReturnsTrue)) {
            _ref_speed[i] = std::clamp(_ref_speed[i], -10000, 10000);
        }

        if (ImGui::InputInt("Torque [Nm]", &_ref_torque[i], 1, 100, ImGuiInputTextFlags_EnterReturnsTrue)) {
            _ref_torque[i] = std::clamp(_ref_torque[i], -400, 400);
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

        ImGui::End();
    }

    std::copy(_ref_ctlmode.begin(), _ref_ctlmode.end(), _server->motordrive_ref_ctlmode.begin());
    std::copy(_ref_enable.begin(), _ref_enable.end(), _server->motordrive_ref_enable.begin());
    std::copy(_ref_discharge.begin(), _ref_discharge.end(), _server->motordrive_ref_discharge.begin());
    std::copy(_ref_mainrelay.begin(), _ref_mainrelay.end(), _server->motordrive_ref_mainrelay.begin());
    std::copy(_ref_footbrake.begin(), _ref_footbrake.end(), _server->motordrive_ref_footbrake.begin());
    std::copy(_ref_handbrake.begin(), _ref_handbrake.end(), _server->motordrive_ref_handbrake.begin());
    std::copy(_ref_faultreset.begin(), _ref_faultreset.end(), _server->motordrive_ref_faultreset.begin());
    std::copy(_ref_gear.begin(), _ref_gear.end(), _server->motordrive_ref_gear.begin());
    std::copy(_ref_speed.begin(), _ref_speed.end(), _server->motordrive_ref_speed.begin());
    std::copy(_ref_torque.begin(), _ref_torque.end(), _server->motordrive_ref_torque.begin());
}


} // namespace atvvcu
} // namespace ui
