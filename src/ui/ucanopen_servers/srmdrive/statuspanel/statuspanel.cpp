#include "statuspanel.h"


namespace ui {
namespace srmdrive {


StatusPanel::StatusPanel(std::shared_ptr<::srmdrive::Server> server,
                         const std::string& menu_title,
                         const std::string& window_title,
                         bool show_by_default)
        : View(menu_title, window_title, show_by_default)
        , _server(server)
{}


void StatusPanel::draw() {
    ImGui::Begin(_window_title.c_str(), &show);
    _draw_error_table();
    ImGui::NewLine();
    _draw_warning_table();
    ImGui::End();
}


void StatusPanel::_draw_error_table() {
    static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
    if (ImGui::BeginTable("error_table", 1, flags)) {
        const auto& error_list = _server->error_list();
        auto errors = _server->errors();

        for (size_t row = 0; row < error_list.size(); ++row) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%s", error_list[row].c_str());
            
            if ((errors & (1 << row)) != 0) {
                ImU32 cell_bg_color = ImGui::GetColorU32(ImVec4(0.7f, 0.3f, 0.3f, 0.65f));
                ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, cell_bg_color);
            }
        }

        ImGui::EndTable();
    }
}


void StatusPanel::_draw_warning_table() {
    static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
    if (ImGui::BeginTable("warning_table", 1, flags)) {
        const auto& warning_list = _server->warning_list();
        auto warnings = _server->warnings();

        for (size_t row = 0; row < warning_list.size(); ++row) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%s", warning_list[row].c_str());
            
            if ((warnings & (1 << row)) != 0) {
                ImU32 cell_bg_color = ImGui::GetColorU32(ImVec4(0.7f, 0.7f, 0.3f, 0.65f));
                ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, cell_bg_color);
            }
        }

        ImGui::EndTable();
    }
}


} // namespace srmdrive
} // namespace ui
