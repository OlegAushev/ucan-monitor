#include "systempanel.h"
#include <ui/components/togglebutton.h>


namespace ui {
namespace atvvcu {


SystemPanel::SystemPanel(std::shared_ptr<::atvvcu::Server> server,
                         const std::string& menu_title,
                         const std::string& window_title,
                         bool show_by_default)
        : View(menu_title, window_title, show_by_default)
        , _server(server)
{}


void SystemPanel::draw(bool& open) {
    ImGui::Begin(_window_title.c_str(), &open);

    _read_keyboard();

    ImGui::RadioButton("Normal", &_vcu_opmode, std::to_underlying(::atvvcu::VcuOperationMode::normal));
    ImGui::SameLine();
    ImGui::RadioButton("Ctlemu", &_vcu_opmode, std::to_underlying(::atvvcu::VcuOperationMode::ctlemu));
    ImGui::SameLine();
    ImGui::RadioButton("Debug ", &_vcu_opmode, std::to_underlying(::atvvcu::VcuOperationMode::debug));
   
    _server->vcu_opmode = ::atvvcu::VcuOperationMode(_vcu_opmode);

    ImGui::TextUnformatted("Uptime[s]:");
    ImGui::SameLine();
    ImGui::TextUnformatted(_server->watch_service.string_value("sys", "uptime").c_str());

    ::atvvcu::Server::SystemData systemdata_tpdo = _server->system_data.load();

    ImGui::TextUnformatted("VCU State:");
    ImGui::SameLine();
    ImGui::TextUnformatted(systemdata_tpdo.vcu_state.data());

    ImGui::TextUnformatted("VCU Mode:");
    ImGui::SameLine();
    ImGui::TextUnformatted(systemdata_tpdo.vcu_opmode.data());

    // power switch
    ToggleButton(ICON_MDI_CAR_BATTERY" Power On/Off", _power_enabled);
    ImGui::SameLine();
    ImGui::TextDisabled("(F3)");
    _server->power_enabled = _power_enabled;
    
    // run switch
    ToggleButton(ICON_MDI_POWER" Run On/Off  ", _run_enabled);
    ImGui::SameLine();
    ImGui::TextDisabled("(F4)");
    _server->run_enabled = _run_enabled;

    ImGui::End();
}


void SystemPanel::_read_keyboard() {
    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_F2))) {
        //_emergency = !_emergency;
    }

    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_F3))) {
        _power_enabled = !_power_enabled;
    }

    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_F4))) {
        _run_enabled = !_run_enabled;
    }
}


} // namespace atvvcu
} // namespace ui
