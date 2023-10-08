#include "statuspanel.h"


namespace ui {
namespace srmdrive {


void StatusPanel::draw(bool& open) {
    ImGui::Begin("Status", &open);
    _draw_error_table();
    ImGui::NewLine();
    _draw_warning_table();
    ImGui::End();
}


void StatusPanel::_draw_error_table() {
    static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
    if (ImGui::BeginTable("error_table", 2, flags)) {
        const auto& error_list = _server->error_list();
        auto errors = _server->errors();

        for (int row = 0; row < error_list.size(); ++row) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%s", error_list[row].c_str());
            
            if ((errors & (1 << row)) != 0) {
                ImGui::TableSetColumnIndex(1);
                ImGui::TextUnformatted("X");
            }
        }

        ImGui::EndTable();
    }
}


void StatusPanel::_draw_warning_table() {
    static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
    if (ImGui::BeginTable("warning_table", 2, flags)) {
        const auto& warning_list = _server->warning_list();
        auto warnings = _server->warnings();

        for (int row = 0; row < warning_list.size(); ++row) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%s", warning_list[row].c_str());
            
            if ((warnings & (1 << row)) != 0) {
                ImGui::TableSetColumnIndex(1);
                ImGui::TextUnformatted("X");
            }
        }

        ImGui::EndTable();
    }
}


} // namespace srmdrive
} // namespace ui
