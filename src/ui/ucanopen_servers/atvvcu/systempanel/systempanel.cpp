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
    _draw_controls();
    _draw_status();

    ImGui::End();
}


void SystemPanel::_draw_controls() {
    ImGui::SeparatorText("Control");

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

    // misc actions
    if (ImGui::TreeNode("Misc Actions")) {
        if (ImGui::Button("Clear Errors", ImVec2(300, 0))) {
            _server->exec("ctl", "sys", "clear_errors");
        }
        ImGui::TreePop();
    }
}


void SystemPanel::_draw_status() {
    ImGui::SeparatorText("Status");

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
