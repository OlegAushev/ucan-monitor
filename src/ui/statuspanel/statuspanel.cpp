#include "statuspanel.h"
#include <ui/util/style.h>

#include <optional>


namespace ui {


StatusPanel::StatusPanel(std::shared_ptr<ucanopen::Server> server,
                         const std::string& menu_title,
                         const std::string& window_title,
                         bool open)
        : View(menu_title, window_title, open)
        , _server(server) {}


void StatusPanel::draw() {
    ImGui::Begin(_window_title.c_str(), &_opened);

    const auto& service = _server->status_service;
    const auto& description = _server->description();
    if (!description || !service.available()) {
        ImGui::TextDisabled("Описание словаря не называет статусов.");
        ImGui::End();
        return;
    }

    // Without frames the severities are the last ones that came, maybe old.
    bool const good = service.good();
    if (!good) {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_red);
        ImGui::TextUnformatted(ICON_MDI_CLOSE_NETWORK " Нет кадров статусов");
        ImGui::PopStyleColor();
    }

    static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
    if (ImGui::BeginTable("status_table", 3, flags)) {
        ImGui::TableSetupColumn("Статус");
        ImGui::TableSetupColumn("Степень");
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, ImGui::GetTextLineHeight());
        ImGui::TableHeadersRow();

        for (const auto& status : service.statuses()) {
            const auto& field = *status.field;
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted(field.label.ru.empty() ? field.name.c_str() : field.label.ru.c_str());
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
                ImGui::SetTooltip("%s", field.name.c_str());
            }

            ImGui::TableSetColumnIndex(1);
            if (!status.severity.has_value()) {
                ImGui::TextDisabled("н/д");
                continue;
            }
            const auto* table = description->value_table(field.value_table);
            const auto* severity = (table == nullptr) ? nullptr : table->find(*status.severity);
            if (severity == nullptr) {
                ImGui::Text("%u?", unsigned(*status.severity));
                continue;
            }
            if (*status.severity == 0) {
                continue; // no level, nothing to show
            }

            bool const alert = (severity->name == "critical" || severity->name == "emergency");
            auto const bg = [&]() {
                if (alert || severity->name == "error") {
                    return ui::colors::table_bg_red;
                }
                if (severity->name == "warning") {
                    return ui::colors::table_bg_yellow;
                }
                return ui::colors::table_bg_blue;
            }();
            if (good) {
                ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, bg);
                ImGui::TextUnformatted(severity->label.ru.empty() ? severity->name.c_str()
                                                                  : severity->label.ru.c_str());
            } else {
                ImGui::TextDisabled("%s", severity->label.ru.empty() ? severity->name.c_str()
                                                                    : severity->label.ru.c_str());
            }

            ImGui::TableSetColumnIndex(2);
            if (alert) {
                ImGui::TextUnformatted(ICON_MDI_ALERT);
                if (good) {
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, bg);
                }
            }
        }
        ImGui::EndTable();
    }

    ImGui::End();
}


} // namespace ui
