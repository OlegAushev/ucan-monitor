#include "statuspanel.h"
#include <ui/util/style.h>


namespace ui {
namespace brkdrive {


StatusPanel::StatusPanel(std::shared_ptr<::brkdrive::Server> server,
                         const std::string& menu_title,
                         const std::string& window_title,
                         bool open)
        : View(menu_title, window_title, open)
        , _server(server)
{}


void StatusPanel::draw() {
    ImGui::Begin(_window_title.c_str(), &_opened);
    _draw_error_table();
    _draw_warning_table();
    ImGui::End();
}


void StatusPanel::_draw_error_table() {
    auto errors = _server->errors();
    if (errors != 0) {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_red);
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_green);
    }

    ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED); 
    ImGui::PopStyleColor();
    ImGui::SameLine();

    if (ImGui::TreeNode("Errors")) {
        static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
        if (ImGui::BeginTable("error_table", 1, flags)) {
            const auto& error_list = _server->error_list();

            for (size_t row = 0; row < error_list.size(); ++row) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%s", error_list[row].c_str());
                
                if ((errors & (1 << row)) != 0) {
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ui::colors::table_bg_red);
                }
            }

            ImGui::EndTable();
        }
        ImGui::TreePop();
    }

}


void StatusPanel::_draw_warning_table() {
    auto warnings = _server->warnings();
    if (warnings != 0) {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_yellow);
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_green);
    }

    ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED); 
    ImGui::PopStyleColor();
    ImGui::SameLine();

    if (ImGui::TreeNode("Warnings")) {
        static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
        if (ImGui::BeginTable("warning_table", 1, flags)) {
            const auto& warning_list = _server->warning_list();

            for (size_t row = 0; row < warning_list.size(); ++row) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%s", warning_list[row].c_str());
                
                if ((warnings & (1 << row)) != 0) {
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ui::colors::table_bg_yellow);
                }
            }

            ImGui::EndTable();
        }
        ImGui::TreePop();
    }
}


} // namespace brkdrive
} // namespace ui
