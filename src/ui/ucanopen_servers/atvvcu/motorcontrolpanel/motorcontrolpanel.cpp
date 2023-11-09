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
{

}


void MotorControlPanel::draw(bool& open) {
    for (size_t i = 0; i < 4; ++i) {
        ImGui::Begin(_window_titles[i].c_str(), &open);

        ImGui::SeparatorText(ICON_MDI_CAMERA_CONTROL" Control Mode");
        ImGui::RadioButton("Speed", &_refmode[i], 0);
        ImGui::SameLine();
        ImGui::RadioButton("Torque", &_refmode[i], 1);

        ImGui::SeparatorText(ICON_MDI_CAR_SHIFT_PATTERN" Gear");
        ImGui::RadioButton("Neutral", &_refgear[i], 0);
        ImGui::SameLine();
        ImGui::RadioButton("Forward", &_refgear[i], 1);
        ImGui::SameLine();
        ImGui::RadioButton("Reverse", &_refgear[i], 2);

        ImGui::SeparatorText("");
        ui::ToggleButton(ICON_MDI_ELECTRIC_SWITCH" Relay ", _refrelay[i]);
        ImGui::SameLine();
        ui::ToggleButton(ICON_MDI_POWER" Enable", _refstatus[i]);     
        ui::ToggleButton(ICON_MDI_FLASH_OFF_OUTLINE" Discharge  ", _refdischarge[i]);

        ui::ToggleButton(ICON_MDI_CAR_BRAKE_ALERT" FBrake", _reffootbrake[i]);
        ImGui::SameLine();
        ui::ToggleButton(ICON_MDI_CAR_BRAKE_HOLD" HBrake", _refhandbrake[i]);

        ui::ToggleButton(ICON_MDI_ALERT_CIRCLE_OUTLINE" Fault Reset", _reffaultreset[i]);

        ImGui::PushItemWidth(150);
        if (ImGui::InputInt("Speed [rpm]", &_refspeed[i], 1, 100, ImGuiInputTextFlags_EnterReturnsTrue)) {
            _refspeed[i] = std::clamp(_refspeed[i], -10000, 10000);
        }

        if (ImGui::InputInt("Torque [Nm]", &_reftorque[i], 1, 100, ImGuiInputTextFlags_EnterReturnsTrue)) {
            _reftorque[i] = std::clamp(_reftorque[i], -400, 400);
        }
        ImGui::PopItemWidth();

        if (ImGui::TreeNode("Torque Limits")) {
            ImGui::PushItemWidth(150);
            if (ImGui::InputInt("El Torque [Nm]", &_refeltorque_max, 1, 100, ImGuiInputTextFlags_EnterReturnsTrue)) {
                _refeltorque_max = std::clamp(_refeltorque_max, 0, 400);
                _server->write("ctl", "motordrive", "eltorque_max", std::to_string(_refeltorque_max));
            }

            if (ImGui::InputInt("Brk Torque [Nm]", &_refbraketorque_max, 1, 100, ImGuiInputTextFlags_EnterReturnsTrue)) {
                _refbraketorque_max = std::clamp(_refbraketorque_max, -400, 400);
                _server->write("ctl", "motordrive", "braketorque_max", std::to_string(_refbraketorque_max));
            }
            ImGui::PopItemWidth();

            ImGui::TreePop();
        }

        ImGui::End();
    }

    std::copy(_refmode.begin(), _refmode.end(), _server->motordrive_refmode.begin());
    std::copy(_refstatus.begin(), _refstatus.end(), _server->motordrive_refstatus.begin());
    std::copy(_refdischarge.begin(), _refdischarge.end(), _server->motordrive_refdischarge.begin());
    std::copy(_refrelay.begin(), _refrelay.end(), _server->motordrive_refrelay.begin());
    std::copy(_reffootbrake.begin(), _reffootbrake.end(), _server->motordrive_reffootbrake.begin());
    std::copy(_refhandbrake.begin(), _refhandbrake.end(), _server->motordrive_refhandbrake.begin());
    std::copy(_reffaultreset.begin(), _reffaultreset.end(), _server->motordrive_reffaultreset.begin());
    std::copy(_refgear.begin(), _refgear.end(), _server->motordrive_refgear.begin());
    std::copy(_refspeed.begin(), _refspeed.end(), _server->motordrive_refspeed.begin());
    std::copy(_reftorque.begin(), _reftorque.end(), _server->motordrive_reftorque.begin());
}


} // namespace atvvcu
} // namespace ui
