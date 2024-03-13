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
    _draw_controls();
    _draw_status();

    ImGui::End();
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

        ImGui::PopStyleVar();
        ImGui::TreePop();
    }

    _server->pdm.toggle_debug_mode(_pdm_dbg);
    _server->drive.toggle_debug_mode(_drive_dbg);
    _server->dash.toggle_debug_mode(_dash_dbg);
    _server->gear_selector.toggle_debug_mode(_gear_dbg);
    _server->accl_pedal.toggle_debug_mode(_accl_dbg);
}


void Dashboard::_draw_controls() {
    ImGui::TextUnformatted("Uptime[s]:");
    ImGui::SameLine();
    ImGui::TextUnformatted(_server->watch_service.string_value("sys", "uptime").c_str());

    ImGui::TextUnformatted("VCU State:");
    ImGui::SameLine();
    ImGui::TextUnformatted(_server->vcu_state().data());

    ImGui::SeparatorText(ICON_MDI_LIGHT_SWITCH_OFF" Dash");
    if (!_server->dash.debug_mode()) { ImGui::BeginDisabled(); }
    // power switch
    ToggleButton(ICON_MDI_CAR_BATTERY" Power On/Off", _power_enabled);
    ImGui::SameLine();
    ImGui::TextDisabled("(F3)");
    _server->dash.toggle_power(_power_enabled);
    
    // run switch
    ToggleButton(ICON_MDI_POWER" Run On/Off  ", _run_enabled);
    ImGui::SameLine();
    ImGui::TextDisabled("(F4)");
    _server->dash.toggle_run(_run_enabled);

    if (!_server->dash.debug_mode()) { ImGui::EndDisabled(); }

    // Gear
    ImGui::SeparatorText(ICON_MDI_CAR_SHIFT_PATTERN" Gear");
    if (!_server->gear_selector.debug_mode()) { ImGui::BeginDisabled(); _ref_gear = std::to_underlying(_server->gear_selector.gear()); }

    ImGui::RadioButton("R", &_ref_gear, 2);
    if (_server->gear_selector.gear() == ::atvvcu::gear::Gear::reverse) { ImGui::SameLine(); ImGui::TextUnformatted(ICON_MDI_CHEVRON_LEFT); }

    ImGui::RadioButton("N", &_ref_gear, 0);
    if (_server->gear_selector.gear() == ::atvvcu::gear::Gear::neutral) { ImGui::SameLine(); ImGui::TextUnformatted(ICON_MDI_CHEVRON_LEFT); }

    ImGui::RadioButton("D", &_ref_gear, 1);
    if (_server->gear_selector.gear() == ::atvvcu::gear::Gear::forward) { ImGui::SameLine(); ImGui::TextUnformatted(ICON_MDI_CHEVRON_LEFT); }

    if (!_server->gear_selector.debug_mode()) { ImGui::EndDisabled(); }
    _server->gear_selector.set_gear(::atvvcu::gear::Gear(_ref_gear));

    // Accelerator pedal
    ImGui::SeparatorText(ICON_MDI_SPEEDOMETER" Pedal");
    ImGui::ProgressBar(_server->accl_pedal.pressure());
    if (!_server->accl_pedal.debug_mode()) { ImGui::BeginDisabled(); }
    ImGui::SliderFloat("##", &_accl, 0.0f, 1.0f);
    if (!_server->accl_pedal.debug_mode()) { ImGui::EndDisabled(); }
    _server->accl_pedal.set_pressure(_accl);

    // Misc
    ImGui::SeparatorText("");
    _draw_debug_controls();

    ImGui::TextUnformatted(ICON_MDI_WRENCH);
    ImGui::SameLine();
    if (ImGui::TreeNode("Misc Actions")) {
        if (ImGui::Button("Clear Errors", ImVec2(300, 0))) {
            _server->exec("ctl", "sys", "clear_errors");
        }
        ImGui::TreePop();
    }
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
