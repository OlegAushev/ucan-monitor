#include "dashboard.h"
#include <ui/util/togglebutton.h>
#include <ui/util/util.h>


namespace ui {
namespace atvvcu {


Dashboard::Dashboard(std::shared_ptr<::atvvcu::Server> server,
                         const std::string& menu_title,
                         const std::string& window_title,
                         bool open)
        : View(menu_title, window_title, open)
        , _server(server)
{}


void Dashboard::draw() {
    ImGui::Begin(_window_title.c_str(), &is_open);

    _read_keyboard();
    _draw_debug_controls();
    _draw_dash();
    _draw_gear();
    _draw_accl_brakes();
    _draw_misc_controls();
    _draw_status();

    ImGui::End();
}


void Dashboard::_draw_misc_controls() {
    // Misc
    ImGui::SeparatorText("");

    ImGui::TextUnformatted(ICON_MDI_CAR_WRENCH);
    ImGui::SameLine();
    if (ImGui::TreeNode("Service Actions")) {
        if (ImGui::Button(ICON_MDI_SHIELD_REFRESH" Clear Errors       ")) {
            _server->exec("ctl", "sys", "clear_errors");
        }

        if (ImGui::Button(ICON_MDI_PUMP" Toggle Pump        ")) {
            _server->exec("ctl", "vcu", "toggle_pump");
        }

        if (ImGui::Button(ICON_MDI_STEERING" Toggle Hydrostation")) {
            _server->exec("ctl", "vcu", "toggle_hydrostation");
        }

        if (ImGui::Button(ICON_MDI_FAN" Toggle Fan         ")) {
            _server->exec("ctl", "vcu", "toggle_fan");
        }

        if (ImGui::Button(ICON_MDI_RESTART" Reset VCU          ")) {
            _server->exec("ctl", "sys", "reset_device");
        }


        ImGui::TreePop();
    }
}


void Dashboard::_draw_debug_controls() {
    if (_server->vcu_debug_mode()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.7f, 0.3f, 0.3f, 0.95f)));
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.3f, 0.3f, 0.3f, 0.95f)));
    }
    ImGui::TextUnformatted(ICON_MDI_BUG); 
    ImGui::PopStyleColor();        

    ImGui::SameLine();

    if (ImGui::TreeNode("Debug Options")) {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
        
        ImGui::Checkbox("PDM", &_pdm_dbg);
        if (_server->pdm.debug_mode()) {
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.7f, 0.3f, 0.3f, 0.95f)));
            ImGui::TextUnformatted(ICON_MDI_BUG); 
            ImGui::PopStyleColor();             
        }

        ImGui::Checkbox("Drive", &_drive_dbg);
        if (_server->drive.debug_mode()) {
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.7f, 0.3f, 0.3f, 0.95f)));
            ImGui::TextUnformatted(ICON_MDI_BUG); 
            ImGui::PopStyleColor();             
        }

        ImGui::Checkbox("Dash", &_dash_dbg);
        if (_server->dash.debug_mode()) {
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.7f, 0.3f, 0.3f, 0.95f)));
            ImGui::TextUnformatted(ICON_MDI_BUG); 
            ImGui::PopStyleColor();             
        }

        ImGui::Checkbox("Gear", &_gear_dbg);
        if (_server->gear_selector.debug_mode()) {
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.7f, 0.3f, 0.3f, 0.95f)));
            ImGui::TextUnformatted(ICON_MDI_BUG); 
            ImGui::PopStyleColor();             
        }

        ImGui::Checkbox("Pedal", &_accl_dbg);
        if (_server->accl_pedal.debug_mode()) {
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.7f, 0.3f, 0.3f, 0.95f)));
            ImGui::TextUnformatted(ICON_MDI_BUG); 
            ImGui::PopStyleColor();             
        }

        ImGui::Checkbox("Brakes", &_brake_dbg);
        if (_server->brakes.debug_mode()) {
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.7f, 0.3f, 0.3f, 0.95f)));
            ImGui::TextUnformatted(ICON_MDI_BUG); 
            ImGui::PopStyleColor();             
        }

        ImGui::Checkbox("ESP", &_esp_dbg);
        if (_server->esp_system.debug_mode()) {
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.7f, 0.3f, 0.3f, 0.95f)));
            ImGui::TextUnformatted(ICON_MDI_BUG); 
            ImGui::PopStyleColor();             
        }

        ImGui::PopStyleVar();
        ImGui::TreePop();
    }

    _server->pdm.toggle_debug_mode(_pdm_dbg);
    _server->drive.toggle_debug_mode(_drive_dbg);
    _server->dash.toggle_debug_mode(_dash_dbg);
    _server->gear_selector.toggle_debug_mode(_gear_dbg);
    _server->accl_pedal.toggle_debug_mode(_accl_dbg);
    _server->brakes.toggle_debug_mode(_brake_dbg);
    _server->esp_system.toggle_debug_mode(_esp_dbg);
}


void Dashboard::_draw_status() {
    ImGui::SeparatorText(ICON_MDI_INFORMATION_SLAB_CIRCLE" Status");

    for (auto domain_idx = 0uz; domain_idx < ::atvvcu::error_domain_count; ++domain_idx) {
        if (_server->errors[domain_idx] != 0) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.7f, 0.3f, 0.3f, 0.95f)));
            ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED); 
            ImGui::PopStyleColor();
        } else if (_server->warnings[domain_idx] != 0) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.7f, 0.7f, 0.3f, 0.95f)));
            ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED); 
            ImGui::PopStyleColor();
        } else {
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.3f, 0.7f, 0.3f, 0.95f)));
            ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED); 
            ImGui::PopStyleColor();         
        }

        ImGui::SameLine();
        
        if (ImGui::TreeNode(::atvvcu::error_domains[domain_idx].data())) {
            // draw error table
            if (::atvvcu::error_list[domain_idx].size() != 0) {
                ImGui::SeparatorText("Errors");
                static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
                if (ImGui::BeginTable("error_table", 1, flags)) {
                    uint32_t errors = _server->errors[domain_idx];

                    for (auto row = 0uz; row < ::atvvcu::error_list[domain_idx].size(); ++row) {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("%s", ::atvvcu::error_list[domain_idx][row].data());
                        
                        if ((errors & (1 << row)) != 0) {
                            ImU32 cell_bg_color = ImGui::GetColorU32(ImVec4(0.7f, 0.3f, 0.3f, 0.65f));
                            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, cell_bg_color);
                        }
                    }

                    ImGui::EndTable();
                }
            }

            // draw warning table
            if (::atvvcu::warning_list[domain_idx].size() != 0) {
                ImGui::SeparatorText("Warnings");
                static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
                if (ImGui::BeginTable("warning_table", 1, flags)) {
                    uint32_t warnings = _server->warnings[domain_idx];

                    for (auto row = 0uz; row < ::atvvcu::warning_list[domain_idx].size(); ++row) {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("%s", ::atvvcu::warning_list[domain_idx][row].data());
                        
                        if ((warnings & (1 << row)) != 0) {
                            ImU32 cell_bg_color = ImGui::GetColorU32(ImVec4(0.7f, 0.7f, 0.3f, 0.65f));
                            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, cell_bg_color);
                        }
                    }

                    ImGui::EndTable();
                }
            }

            ImGui::TreePop();
        }
    }
}


void Dashboard::_read_keyboard() {
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


void Dashboard::_draw_dash() {
    ImGui::SeparatorText(ICON_MDI_GAUGE_LOW" Dash");

    ImGui::TextUnformatted(ICON_MDI_TIMER_OUTLINE" Uptime[s]:");
    ImGui::SameLine();
    ImGui::TextUnformatted(_server->watch_service.string_value("sys", "uptime").c_str());

    if (!_server->heartbeat_service.good() || !_server->tpdo_service.good(ucanopen::CobTpdo::tpdo1)) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.7f, 0.3f, 0.3f, 0.95f)));
        ImGui::TextUnformatted(ICON_MDI_CLOSE_NETWORK_OUTLINE);
        ImGui::PopStyleColor();
    } else {
        static bool icon_active = true;
        static auto timestamp = std::chrono::steady_clock::now();

        if (icon_active) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.3f, 0.7f, 0.3f, 0.95f)));
        } else {
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.3f, 0.3f, 0.3f, 0.95f)));
        }
        ImGui::TextUnformatted(ICON_MDI_CHECK_NETWORK_OUTLINE);
        ImGui::PopStyleColor();        

        if (std::chrono::steady_clock::now() > timestamp + std::chrono::milliseconds(750)) {
            icon_active = !icon_active;
            timestamp = std::chrono::steady_clock::now();
        }
    }

    ImGui::SameLine();
    std::string state(_server->vcu_state());
    ImGui::PushItemWidth(90);
    ImGui::InputText("##state", state.data(), state.size(), ImGuiInputTextFlags_ReadOnly);
    ImGui::PopItemWidth();

    ImGui::SameLine();
    if (_server->dash.network_charging_enabled()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.3f, 0.9f, 0.3f, 0.95f)));
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.3f, 0.3f, 0.3f, 0.95f)));
    }
    ImGui::TextUnformatted(ICON_MDI_EV_STATION); 
    ImGui::PopStyleColor();
    
    ImGui::SameLine();
    if (_server->dash.genset_charging_enabled()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.3f, 0.9f, 0.3f, 0.95f)));
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.3f, 0.3f, 0.3f, 0.95f)));
    }
    ImGui::TextUnformatted(ICON_MDI_GENERATOR_PORTABLE); 
    ImGui::PopStyleColor();


    if (_server->dash.remote_control_enabled()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.9f, 0.9f, 0.3f, 0.95f)));
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.3f, 0.3f, 0.3f, 0.95f)));
    }
    ImGui::TextUnformatted(ICON_MDI_CONTROLLER); 
    ImGui::PopStyleColor();        
    
    ImGui::SameLine();
    if (!_server->esp_system.tcs_enabled()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.9f, 0.3f, 0.3f, 0.95f)));
    } else if (!_server->esp_system.tcs_triggered()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.3f, 0.3f, 0.3f, 0.95f)));
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.9f, 0.9f, 0.3f, 0.95f)));
    }
    ImGui::TextUnformatted(ICON_MDI_CAR_TRACTION_CONTROL); 
    ImGui::PopStyleColor();

    ImGui::SameLine();
    if (_server->brakes.left_pressed()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.9f, 0.3f, 0.3f, 0.95f)));
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.3f, 0.3f, 0.3f, 0.95f)));
    }
    ImGui::TextUnformatted(ICON_MDI_ALPHA_L "" ICON_MDI_CAR_BRAKE_ALERT); 
    ImGui::PopStyleColor();

    ImGui::SameLine();
    if (_server->brakes.right_pressed()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.9f, 0.3f, 0.3f, 0.95f)));
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.3f, 0.3f, 0.3f, 0.95f)));
    }
    ImGui::TextUnformatted(ICON_MDI_CAR_BRAKE_ALERT "" ICON_MDI_ALPHA_R); 
    ImGui::PopStyleColor();

    ImGui::SameLine();
    if (_server->dash.leftturn_enabled()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.9f, 0.9f, 0.3f, 0.95f)));
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.3f, 0.3f, 0.3f, 0.95f)));
    }
    ImGui::TextUnformatted(ICON_MDI_ARROW_U_DOWN_LEFT_BOLD);
    ImGui::PopStyleColor();

    ImGui::SameLine();
    if (_server->dash.rightturn_enabled()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.9f, 0.9f, 0.3f, 0.95f)));
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.3f, 0.3f, 0.3f, 0.95f)));
    }
    ImGui::TextUnformatted(ICON_MDI_ARROW_U_DOWN_RIGHT_BOLD);
    ImGui::PopStyleColor();

    ImGui::SameLine();
    if (_server->aux_systems.pump_enabled()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.3f, 0.3f, 0.3f, 0.95f)));
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.9f, 0.3f, 0.3f, 0.95f)));
    }
    ImGui::TextUnformatted(ICON_MDI_PUMP); 
    ImGui::PopStyleColor();

    ImGui::SameLine();
    if (_server->aux_systems.hydrostation_enabled()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.3f, 0.3f, 0.3f, 0.95f)));
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.9f, 0.9f, 0.3f, 0.95f)));
    }
    ImGui::TextUnformatted(ICON_MDI_STEERING); 
    ImGui::PopStyleColor();

    ImGui::SameLine();
    if (_server->aux_systems.fan_enabled()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.9f, 0.9f, 0.3f, 0.95f)));
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.3f, 0.3f, 0.3f, 0.95f)));
    }
    ImGui::TextUnformatted(ICON_MDI_FAN); 
    ImGui::PopStyleColor();


    // emergency switch
    if (_server->dash.emergency()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.3f, 0.7f, 0.3f, 0.95f)));
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.7f, 0.3f, 0.3f, 0.95f)));
    }
    ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED); 
    ImGui::PopStyleColor();
    ImGui::SameLine();
    {
        ui::util::DisableGuard dg(!_server->dash.debug_mode());
        ToggleButton(ICON_MDI_ALERT_OCTAGON_OUTLINE" Emergency   ", _emergency);
        ImGui::SameLine();
        ImGui::TextDisabled("(F2)");
        _server->dash.toggle_emergency(_emergency);
    }

    // power switch
    if (_server->dash.power_enabled()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.3f, 0.7f, 0.3f, 0.95f)));
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.7f, 0.3f, 0.3f, 0.95f)));
    }
    ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED); 
    ImGui::PopStyleColor();
    ImGui::SameLine();
    {   
        ui::util::DisableGuard dg(!_server->dash.debug_mode());
        ToggleButton(ICON_MDI_CAR_BATTERY" Power On/Off", _power_enabled);
        ImGui::SameLine();
        ImGui::TextDisabled("(F3)");
        _server->dash.toggle_power(_power_enabled);
    }
    
    // run switch
    if (_server->dash.run_enabled()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.3f, 0.7f, 0.3f, 0.95f)));
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.7f, 0.3f, 0.3f, 0.95f)));
    }
    ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED); 
    ImGui::PopStyleColor();
    ImGui::SameLine();
    {
        ui::util::DisableGuard dg(!_server->dash.debug_mode());
        ToggleButton(ICON_MDI_POWER" Run On/Off  ", _run_enabled);
        ImGui::SameLine();
        ImGui::TextDisabled("(F4)");
        _server->dash.toggle_run(_run_enabled);
    }

    // fault reset switch
    if (_server->dash.faultreset_enabled()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.3f, 0.7f, 0.3f, 0.95f)));
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.7f, 0.3f, 0.3f, 0.95f)));
    }
    ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED);
    ImGui::PopStyleColor();
    ImGui::SameLine();
    {
        ui::util::DisableGuard dg(!_server->dash.debug_mode());
        ToggleButton(ICON_MDI_SHIELD_REFRESH" Fault Reset ", _fault_reset);
        _server->dash.toggle_faultreset(_fault_reset);

        ImGui::TextUnformatted("Charging: ");
        ImGui::SameLine();
        ImGui::RadioButton(ICON_MDI_POWER_OFF, &_charging_mode, 0);
        ImGui::SameLine();
        ImGui::RadioButton(ICON_MDI_EV_STATION, &_charging_mode, 1);
        ImGui::SameLine();
        ImGui::RadioButton(ICON_MDI_GENERATOR_PORTABLE, &_charging_mode, 2);
        _server->dash.set_charging_mode(_charging_mode);
    }
}


void Dashboard::_draw_gear() {
    ImGui::SeparatorText(ICON_MDI_CAR_SHIFT_PATTERN" Gear");

    if (_server->gear_selector.gear() == ::atvvcu::gear::Gear::reverse){
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.3f, 0.7f, 0.3f, 0.95f)));
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.7f, 0.3f, 0.3f, 0.95f)));
    }
    ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED); 
    ImGui::PopStyleColor();
    ImGui::SameLine();
    {
        ui::util::DisableGuard dg(!_server->gear_selector.debug_mode());
        ImGui::RadioButton("R", &_ref_gear, 2);
    }

    if (_server->gear_selector.gear() == ::atvvcu::gear::Gear::neutral){
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.3f, 0.7f, 0.3f, 0.95f)));
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.7f, 0.3f, 0.3f, 0.95f)));
    }
    ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED); 
    ImGui::PopStyleColor();
    ImGui::SameLine();
    {
        ui::util::DisableGuard dg(!_server->gear_selector.debug_mode());
        ImGui::RadioButton("N", &_ref_gear, 0);
    }

    if (_server->gear_selector.gear() == ::atvvcu::gear::Gear::forward){
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.3f, 0.7f, 0.3f, 0.95f)));
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.7f, 0.3f, 0.3f, 0.95f)));
    }
    ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED); 
    ImGui::PopStyleColor();
    ImGui::SameLine();
    {
        ui::util::DisableGuard dg(!_server->gear_selector.debug_mode());
        ImGui::RadioButton("D", &_ref_gear, 1);
        _server->gear_selector.set_gear(::atvvcu::gear::Gear(_ref_gear));
    }

}


void Dashboard::_draw_accl_brakes() {
    ImGui::SeparatorText(ICON_MDI_SPEEDOMETER" Pedal & Brakes & ESP");
    ImGui::ProgressBar(_server->accl_pedal.pressure());
    {
        ui::util::DisableGuard dg(!_server->accl_pedal.debug_mode());
        ImGui::SliderFloat("##accl_slider", &_accl, 0.0f, 1.0f);
        _server->accl_pedal.set_pressure(_accl);
    }

    {
        ui::util::DisableGuard dg(!_server->brakes.debug_mode());
        ToggleButton(" L " ICON_MDI_CAR_BRAKE_ALERT, _brake_left);
        ImGui::SameLine();
        ToggleButton(ICON_MDI_CAR_BRAKE_ALERT" R ", _brake_right);
        _server->brakes.set_brakes(_brake_left, _brake_right);
    }

    ImGui::SameLine();
    {
        ui::util::DisableGuard dg(!_server->esp_system.debug_mode());
        ToggleButton(ICON_MDI_CAR_TRACTION_CONTROL "TCS", _tcs_enable);
        _server->esp_system.toggle_tcs(_tcs_enable);
    }

    ImGui::SameLine();
    {
        ui::util::DisableGuard dg(!_server->dash.debug_mode());
        ToggleButton(ICON_MDI_ARROW_U_DOWN_LEFT_BOLD "", _turn_left);
        if (_turn_left) { _turn_right = false; }
        ImGui::SameLine();
        ToggleButton(ICON_MDI_ARROW_U_DOWN_RIGHT_BOLD "", _turn_right);
        if (_turn_right) { _turn_left = false; }
        _server->dash.set_turn(_turn_left, _turn_right);
    }

    {
        ui::util::DisableGuard dg(!_server->esp_system.debug_mode());
        auto drive_enabled = _server->esp_system.drive_enabled();
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
        // LF
        ImGui::TextUnformatted(ICON_MDI_POWER "LF");
        ImGui::SameLine();
        ImGui::Checkbox("##LF", &_drive_enable[0]);
        ImGui::SameLine();
        if (drive_enabled[0]) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.3f, 0.7f, 0.3f, 0.95f)));
        } else {
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.7f, 0.3f, 0.3f, 0.95f)));
        }
        ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED); 
        ImGui::PopStyleColor();
    

        // RF
        ImGui::SameLine();
        if (drive_enabled[2]) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.3f, 0.7f, 0.3f, 0.95f)));
        } else {
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.7f, 0.3f, 0.3f, 0.95f)));
        }
        ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED); 
        ImGui::PopStyleColor();   
        ImGui::SameLine();
        ImGui::Checkbox("##RF", &_drive_enable[2]);
        ImGui::SameLine();
        ImGui::TextUnformatted(ICON_MDI_POWER "RF");

        // LB
        ImGui::TextUnformatted(ICON_MDI_POWER "LB");
        ImGui::SameLine();
        ImGui::Checkbox("##LB", &_drive_enable[1]);
        ImGui::SameLine();
        if (drive_enabled[1]) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.3f, 0.7f, 0.3f, 0.95f)));
        } else {
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.7f, 0.3f, 0.3f, 0.95f)));
        }
        ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED); 
        ImGui::PopStyleColor();
        
        // RB
        ImGui::SameLine();
        if (drive_enabled[3]) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.3f, 0.7f, 0.3f, 0.95f)));
        } else {
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.7f, 0.3f, 0.3f, 0.95f)));
        }
        ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED); 
        ImGui::PopStyleColor(); 
        ImGui::SameLine();
        ImGui::Checkbox("##RB", &_drive_enable[3]);
        ImGui::SameLine();
        ImGui::TextUnformatted(ICON_MDI_POWER "RB");
        ImGui::PopStyleVar();
        _server->esp_system.set_ref_drive_enable(_drive_enable);
    }
}


} // namespace atvvcu
} // namespace ui
