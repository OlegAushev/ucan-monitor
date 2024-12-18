#include "statuspanel.h"

#include <ui/util/style.h>

using namespace shm80;

namespace ui {
namespace shm80 {

StatusPanel::StatusPanel(std::shared_ptr<::shm80::Server> server,
                         const std::string& menu_title,
                         const std::string& window_title,
                         bool open)
        : View(menu_title, window_title, open), _server(server) {}

void StatusPanel::draw() {
    ImGui::Begin(_window_title.c_str(), &_opened);

    for (auto domain_idx = 0uz; domain_idx < syslog::domains.size();
         ++domain_idx) {
        if (syslog::errors[domain_idx].empty() &&
            syslog::warnings[domain_idx].empty()) {
            continue;
        }

        if (_server->errors()[domain_idx] != 0) {
            ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_red);
            ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED);
            ImGui::PopStyleColor();
        } else if (_server->warnings()[domain_idx] != 0) {
            ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_yellow);
            ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED);
            ImGui::PopStyleColor();
        } else {
            ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_green);
            ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED);
            ImGui::PopStyleColor();
        }

        ImGui::SameLine();

        if (ImGui::TreeNode(syslog::domains[domain_idx].data())) {
            // draw error table
            if (syslog::errors[domain_idx].size() != 0) {
                ImGui::SeparatorText("Errors");
                static ImGuiTableFlags flags =
                        ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
                if (ImGui::BeginTable("error_table", 1, flags)) {
                    uint32_t errors = _server->errors()[domain_idx];
                    for (auto row = 0uz;
                         row < syslog::errors[domain_idx].size();
                         ++row) {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("%s",
                                    syslog::errors[domain_idx][row].data());
                        if ((errors & (1 << row)) != 0) {
                            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg,
                                                   ui::colors::table_bg_red);
                        }
                    }
                    ImGui::EndTable();
                }
            }

            // draw warning table
            if (syslog::warnings[domain_idx].size() != 0) {
                ImGui::SeparatorText("Warnings");
                static ImGuiTableFlags flags =
                        ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
                if (ImGui::BeginTable("warning_table", 1, flags)) {
                    uint32_t warnings = _server->warnings()[domain_idx];
                    for (auto row = 0uz;
                         row < syslog::warnings[domain_idx].size();
                         ++row) {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("%s",
                                    syslog::warnings[domain_idx][row].data());
                        if ((warnings & (1 << row)) != 0) {
                            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg,
                                                   ui::colors::table_bg_yellow);
                        }
                    }
                    ImGui::EndTable();
                }
            }

            ImGui::TreePop();
        }
    }
    ImGui::End();
}

} // namespace shm80
} // namespace ui
