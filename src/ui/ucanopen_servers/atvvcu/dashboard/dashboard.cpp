#include "dashboard.h"
#include <ui/components/togglebutton.h>


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
    _draw_controls();
    _draw_status();

    ImGui::End();
}


void Dashboard::_draw_controls() {
    // Misc
    ImGui::SeparatorText("");

    ImGui::TextUnformatted(ICON_MDI_WRENCH);
    ImGui::SameLine();
    if (ImGui::TreeNode("Misc Actions")) {
        if (ImGui::Button("Clear Errors", ImVec2(300, 0))) {
            _server->exec("ctl", "sys", "clear_errors");
        }
        ImGui::TreePop();
    }
}


void Dashboard::_draw_debug_controls() {
    if (_server->vcu_debug_mode()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.7f, 0.3f, 0.3f, 0.95f)));
        ImGui::TextUnformatted(ICON_MDI_BUG); 
        ImGui::PopStyleColor();        
    } else {
        ImGui::TextUnformatted(ICON_MDI_BUG);           
    }

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

        ImGui::PopStyleVar();
        ImGui::TreePop();
    }

    _server->pdm.toggle_debug_mode(_pdm_dbg);
    _server->drive.toggle_debug_mode(_drive_dbg);
    _server->dash.toggle_debug_mode(_dash_dbg);
    _server->gear_selector.toggle_debug_mode(_gear_dbg);
    _server->accl_pedal.toggle_debug_mode(_accl_dbg);
    _server->brakes.toggle_debug_mode(_brake_dbg);
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
ImGui::SeparatorText(ICON_MDI_LIGHT_SWITCH_OFF" Dash");

    ImGui::TextUnformatted(ICON_MDI_TIMER_OUTLINE" Uptime[s]:");
    ImGui::SameLine();
    ImGui::TextUnformatted(_server->watch_service.string_value("sys", "uptime").c_str());

    if (_server->dash.remote_control_enabled()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.3f, 0.7f, 0.3f, 0.95f)));
        ImGui::TextUnformatted(ICON_MDI_CONTROLLER); 
        ImGui::PopStyleColor();
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.7f, 0.3f, 0.3f, 0.95f)));
        ImGui::TextUnformatted(ICON_MDI_CONTROLLER_OFF); 
        ImGui::PopStyleColor();        
    }
    ImGui::SameLine();
    ImGui::TextUnformatted(" VCU State:");
    ImGui::SameLine();
    ImGui::TextUnformatted(_server->vcu_state().data());

    // emergency switch
    if (_server->dash.emergency()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.3f, 0.7f, 0.3f, 0.95f)));
        ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED); 
        ImGui::PopStyleColor();
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.7f, 0.3f, 0.3f, 0.95f)));
        ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED); 
        ImGui::PopStyleColor();
    }
    ImGui::SameLine();
    if (!_server->dash.debug_mode()) { ImGui::BeginDisabled(); }
    ToggleButton(ICON_MDI_ALERT_OCTAGON_OUTLINE" Emergency   ", _emergency);
    ImGui::SameLine();
    ImGui::TextDisabled("(F2)");
    _server->dash.toggle_emergency(_emergency);
    if (!_server->dash.debug_mode()) { ImGui::EndDisabled(); }

    // power switch
    if (_server->dash.power_enabled()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.3f, 0.7f, 0.3f, 0.95f)));
        ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED); 
        ImGui::PopStyleColor();
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.7f, 0.3f, 0.3f, 0.95f)));
        ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED); 
        ImGui::PopStyleColor();
    }
    ImGui::SameLine();
    if (!_server->dash.debug_mode()) { ImGui::BeginDisabled(); }
    ToggleButton(ICON_MDI_CAR_BATTERY" Power On/Off", _power_enabled);
    ImGui::SameLine();
    ImGui::TextDisabled("(F3)");
    _server->dash.toggle_power(_power_enabled);
    if (!_server->dash.debug_mode()) { ImGui::EndDisabled(); }
    
    // run switch
    if (_server->dash.run_enabled()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.3f, 0.7f, 0.3f, 0.95f)));
        ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED); 
        ImGui::PopStyleColor();
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.7f, 0.3f, 0.3f, 0.95f)));
        ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED); 
        ImGui::PopStyleColor();
    }
    ImGui::SameLine();
    if (!_server->dash.debug_mode()) { ImGui::BeginDisabled(); }
    ToggleButton(ICON_MDI_POWER" Run On/Off  ", _run_enabled);
    ImGui::SameLine();
    ImGui::TextDisabled("(F4)");
    _server->dash.toggle_run(_run_enabled);
    if (!_server->dash.debug_mode()) { ImGui::EndDisabled(); }

    // fault reset switch
    if (_server->dash.faultreset_enabled()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.3f, 0.7f, 0.3f, 0.95f)));
        ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED); 
        ImGui::PopStyleColor();
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.7f, 0.3f, 0.3f, 0.95f)));
        ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED); 
        ImGui::PopStyleColor();
    }
    ImGui::SameLine();
    if (!_server->dash.debug_mode()) { ImGui::BeginDisabled(); }
    ToggleButton(ICON_MDI_CLOSE_CIRCLE_OUTLINE" Fault Reset ", _fault_reset);
    _server->dash.toggle_faultreset(_fault_reset);
    if (!_server->dash.debug_mode()) { ImGui::EndDisabled(); }
}


void Dashboard::_draw_gear() {
    ImGui::SeparatorText(ICON_MDI_CAR_SHIFT_PATTERN" Gear");

    if (_server->gear_selector.gear() == ::atvvcu::gear::Gear::reverse){
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.3f, 0.7f, 0.3f, 0.95f)));
        ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED); 
        ImGui::PopStyleColor();
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.7f, 0.3f, 0.3f, 0.95f)));
        ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED); 
        ImGui::PopStyleColor();
    }
    ImGui::SameLine();
    if (!_server->gear_selector.debug_mode()) { ImGui::BeginDisabled(); }
    ImGui::RadioButton("R", &_ref_gear, 2);
    if (!_server->gear_selector.debug_mode()) { ImGui::EndDisabled(); }

    if (_server->gear_selector.gear() == ::atvvcu::gear::Gear::neutral){
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.3f, 0.7f, 0.3f, 0.95f)));
        ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED); 
        ImGui::PopStyleColor();
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.7f, 0.3f, 0.3f, 0.95f)));
        ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED); 
        ImGui::PopStyleColor();
    }
    ImGui::SameLine();
    if (!_server->gear_selector.debug_mode()) { ImGui::BeginDisabled(); }
    ImGui::RadioButton("N", &_ref_gear, 0);
    if (!_server->gear_selector.debug_mode()) { ImGui::EndDisabled(); }

    if (_server->gear_selector.gear() == ::atvvcu::gear::Gear::forward){
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.3f, 0.7f, 0.3f, 0.95f)));
        ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED); 
        ImGui::PopStyleColor();
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.7f, 0.3f, 0.3f, 0.95f)));
        ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED); 
        ImGui::PopStyleColor();
    }
    ImGui::SameLine();
    if (!_server->gear_selector.debug_mode()) { ImGui::BeginDisabled(); }
    ImGui::RadioButton("D", &_ref_gear, 1);
    if (!_server->gear_selector.debug_mode()) { ImGui::EndDisabled(); }

    _server->gear_selector.set_gear(::atvvcu::gear::Gear(_ref_gear));
}


void Dashboard::_draw_accl_brakes() {
    ImGui::SeparatorText(ICON_MDI_SPEEDOMETER" Pedal & Brakes");
    ImGui::ProgressBar(_server->accl_pedal.pressure());
    if (!_server->accl_pedal.debug_mode()) { ImGui::BeginDisabled(); }
    ImGui::SliderFloat("##", &_accl, 0.0f, 1.0f);
    if (!_server->accl_pedal.debug_mode()) { ImGui::EndDisabled(); }
    _server->accl_pedal.set_pressure(_accl);

    if (!_server->brakes.debug_mode()) { ImGui::BeginDisabled(); }
    ToggleButton(ICON_MDI_CAR_BRAKE_ALERT" L", _brake_left);
    if (!_server->brakes.debug_mode()) { ImGui::EndDisabled(); }
    ImGui::SameLine();
    if (_server->brakes.left_pressed()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.3f, 0.7f, 0.3f, 0.95f)));
        ImGui::TextUnformatted(ICON_MDI_CAR_BRAKE_ALERT); 
        ImGui::PopStyleColor();        
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.7f, 0.3f, 0.3f, 0.95f)));
        ImGui::TextUnformatted(ICON_MDI_CAR_BRAKE_ALERT); 
        ImGui::PopStyleColor();          
    }
    ImGui::SameLine();
    if (_server->brakes.right_pressed()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.3f, 0.7f, 0.3f, 0.95f)));
        ImGui::TextUnformatted(ICON_MDI_CAR_BRAKE_ALERT); 
        ImGui::PopStyleColor();        
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.7f, 0.3f, 0.3f, 0.95f)));
        ImGui::TextUnformatted(ICON_MDI_CAR_BRAKE_ALERT); 
        ImGui::PopStyleColor();          
    }
    ImGui::SameLine();
    if (!_server->brakes.debug_mode()) { ImGui::BeginDisabled(); }
    ToggleButton(ICON_MDI_CAR_BRAKE_ALERT" R", _brake_right);
    if (!_server->brakes.debug_mode()) { ImGui::EndDisabled(); }
}


} // namespace atvvcu
} // namespace ui
