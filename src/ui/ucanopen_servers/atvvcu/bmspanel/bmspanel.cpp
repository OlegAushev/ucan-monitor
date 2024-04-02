#include "bmspanel.h"
#include <ui/util/togglebutton.h>


namespace ui {
namespace atvvcu {


BmsPanel::BmsPanel(std::shared_ptr<::atvvcu::Server> server,
                   const std::string& menu_title,
                   const std::string& window_title,
                   bool open)
        : View(menu_title, window_title, open)
        , _server(server)
{}


void BmsPanel::draw() {
    ImGui::Begin(_window_title.c_str(), &is_open);

    static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
    if (ImGui::BeginTable("contactor_table", 2, flags)) {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Charge[%]");
        ImGui::TableSetColumnIndex(1);
        ImGui::TextUnformatted(_server->watch_service.string_value("bms", "charge_pct").c_str());

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Voltage[V]");
        ImGui::TableSetColumnIndex(1);
        ImGui::TextUnformatted(_server->watch_service.string_value("bms", "v_batt").c_str());

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Current[A]");
        ImGui::TableSetColumnIndex(1);
        ImGui::TextUnformatted(_server->watch_service.string_value("bms", "i_batt").c_str());

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Tcellmin[o]");
        ImGui::TableSetColumnIndex(1);
        ImGui::TextUnformatted(_server->watch_service.string_value("bms", "t_cellmin").c_str());

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Tcellmax[o]");
        ImGui::TableSetColumnIndex(1);
        ImGui::TextUnformatted(_server->watch_service.string_value("bms", "t_cellmax").c_str());

        ImGui::EndTable();
    }

    // internal state
    if (_server->watch_service.value("bms", "internal_state").u32() != 0) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.3f, 0.3f, 0.7f, 0.95f)));
        ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED); 
        ImGui::PopStyleColor();
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.3f, 0.3f, 0.3f, 0.95f)));
        ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED); 
        ImGui::PopStyleColor();         
    }

    ImGui::SameLine();

    if (ImGui::TreeNode("Internal State")) {
        static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
        if (ImGui::BeginTable("internal_state_table", 1, flags)) {
            uint32_t state = _server->watch_service.value("bms", "internal_state").u32();

            for (auto row = 0uz; row < ::atvvcu::bms_internal_states.size(); ++row) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%s", ::atvvcu::bms_internal_states[row].data());
                
                if ((state & (1 << row)) != 0) {
                    ImU32 cell_bg_color = ImGui::GetColorU32(ImVec4(0.3f, 0.3f, 0.7f, 0.65f));
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, cell_bg_color);
                }
            }

            ImGui::EndTable();
        }
        ImGui::TreePop();
    }

    // error register 1
    if (_server->watch_service.value("bms", "error_register_1").u32() != 0) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.7f, 0.3f, 0.3f, 0.95f)));
        ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED); 
        ImGui::PopStyleColor();
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.3f, 0.3f, 0.3f, 0.95f)));
        ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED); 
        ImGui::PopStyleColor();         
    }

    ImGui::SameLine();

    if (ImGui::TreeNode("Error Register 1")) {
        static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
        if (ImGui::BeginTable("error_register_1_table", 1, flags)) {
            uint32_t error = _server->watch_service.value("bms", "error_register_1").u32();

            for (auto row = 0uz; row < ::atvvcu::bms_error_register_1.size(); ++row) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%s", ::atvvcu::bms_error_register_1[row].data());
                
                if ((error & (1 << row)) != 0) {
                    ImU32 cell_bg_color = ImGui::GetColorU32(ImVec4(0.7f, 0.3f, 0.3f, 0.65f));
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, cell_bg_color);
                }
            }

            ImGui::EndTable();
        }
        ImGui::TreePop();
    }

    // error register 2
    if (_server->watch_service.value("bms", "error_register_2").u32() != 0) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.7f, 0.3f, 0.3f, 0.95f)));
        ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED); 
        ImGui::PopStyleColor();
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.3f, 0.3f, 0.3f, 0.95f)));
        ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED); 
        ImGui::PopStyleColor();         
    }

    ImGui::SameLine();

    if (ImGui::TreeNode("Error Register 2")) {
        static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
        if (ImGui::BeginTable("error_register_2_table", 1, flags)) {
            uint32_t error = _server->watch_service.value("bms", "error_register_2").u32();

            for (auto row = 0uz; row < ::atvvcu::bms_error_register_2.size(); ++row) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%s", ::atvvcu::bms_error_register_2[row].data());
                
                if ((error & (1 << row)) != 0) {
                    ImU32 cell_bg_color = ImGui::GetColorU32(ImVec4(0.7f, 0.3f, 0.3f, 0.65f));
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, cell_bg_color);
                }
            }

            ImGui::EndTable();
        }
        ImGui::TreePop();
    }

    // discrete inputs 1
    if (_server->watch_service.value("bms", "discrete_inputs_1").u32() != 0) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.3f, 0.3f, 0.7f, 0.95f)));
        ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED); 
        ImGui::PopStyleColor();
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.3f, 0.3f, 0.3f, 0.95f)));
        ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED); 
        ImGui::PopStyleColor();         
    }

    ImGui::SameLine();

    if (ImGui::TreeNode("Discrete Inputs 1")) {
        static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
        if (ImGui::BeginTable("discrete_inputs_1_table", 1, flags)) {
            uint32_t inputs = _server->watch_service.value("bms", "discrete_inputs_1").u32();

            for (auto row = 0uz; row < ::atvvcu::bms_discrete_inputs_1.size(); ++row) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%s", ::atvvcu::bms_discrete_inputs_1[row].data());
                
                if ((inputs & (1 << row)) != 0) {
                    ImU32 cell_bg_color = ImGui::GetColorU32(ImVec4(0.3f, 0.3f, 0.7f, 0.65f));
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, cell_bg_color);
                }
            }

            ImGui::EndTable();
        }
        ImGui::TreePop();
    }

    // discrete inputs 2
    if (_server->watch_service.value("bms", "discrete_inputs_2").u32() != 0) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.3f, 0.3f, 0.7f, 0.95f)));
        ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED); 
        ImGui::PopStyleColor();
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.3f, 0.3f, 0.3f, 0.95f)));
        ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED); 
        ImGui::PopStyleColor();         
    }

    ImGui::SameLine();

    if (ImGui::TreeNode("Discrete Inputs 2")) {
        static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
        if (ImGui::BeginTable("discrete_inputs_2_table", 1, flags)) {
            uint32_t inputs = _server->watch_service.value("bms", "discrete_inputs_2").u32();

            for (auto row = 0uz; row < ::atvvcu::bms_discrete_inputs_2.size(); ++row) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%s", ::atvvcu::bms_discrete_inputs_2[row].data());
                
                if ((inputs & (1 << row)) != 0) {
                    ImU32 cell_bg_color = ImGui::GetColorU32(ImVec4(0.3f, 0.3f, 0.7f, 0.65f));
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, cell_bg_color);
                }
            }

            ImGui::EndTable();
        }
        ImGui::TreePop();
    }

    ImGui::End();
}


} // namespace atvvcu
} // namespace ui
