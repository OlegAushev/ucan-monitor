#include "statuspanel.h"
#include <ui/util/style.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <string_view>


namespace ui {


StatusPanel::StatusPanel(std::shared_ptr<ucanopen::Server> server,
                         const std::string& menu_title,
                         const std::string& window_title,
                         bool open)
        : View(menu_title, window_title, open)
        , _server(server) {}


namespace {

struct SeverityLevel {
    std::string_view name;
    const char* label;
};

constexpr std::array<SeverityLevel, 4> severity_levels{{
    {"warning", "Предупреждение"},
    {"error", "Ошибка"},
    {"critical", "Критическая ошибка"},
    {"emergency", "Авария"},
}};

void draw_severity_scale(float width, size_t filled, bool good) {
    const float height = ImGui::GetTextLineHeight();
    ImGui::Dummy({width, height});
    if (!ImGui::IsItemVisible()) {
        return;
    }

    const auto origin = ImGui::GetItemRectMin();
    const float step = width / severity_levels.size();
    const float side = std::floor(step * 0.8f);
    const float top = std::floor(origin.y + (height - side) * 0.5f);
    const auto active = good ? (filled == 1 ? colors::icon_yellow : colors::icon_red)
                             : colors::icon_inactive;
    auto* draw = ImGui::GetWindowDrawList();
    for (size_t i = 0; i < severity_levels.size(); ++i) {
        const float left = std::floor(origin.x + i * step + (step - side) * 0.5f);
        const ImVec2 min{left, top};
        const ImVec2 max{left + side, top + side};
        if (i < filled) {
            draw->AddRectFilled(min, max, active);
        } else {
            draw->AddRect(min, max, colors::icon_inactive);
        }
    }
}

void draw_severity_header(float width) {
    draw_severity_scale(width, 0, true);
    if (ImGui::BeginItemTooltip()) {
        ImGui::TextUnformatted("Степень");
        for (size_t i = 0; i < severity_levels.size(); ++i) {
            draw_severity_scale(width, i + 1, true);
            ImGui::SameLine();
            ImGui::TextUnformatted(severity_levels[i].label);
        }
        ImGui::EndTooltip();
    }
}

void draw_severity(const ucanopen::ServerStatusService::Status& status,
                   const ucanopen::od::Description& description, float width, bool good) {
    if (!status.severity.has_value()) {
        ImGui::TextDisabled("н/д");
        ImGui::SetItemTooltip("Статус ещё не получен");
        return;
    }

    const auto* table = description.value_table(status.field->value_table);
    const auto* value = table ? table->find(*status.severity) : nullptr;
    if (value && *status.severity == 0) {
        ImGui::Dummy({width, ImGui::GetTextLineHeight()});
    } else {
        const auto level = value ? std::ranges::find(severity_levels, value->name, &SeverityLevel::name)
                                 : severity_levels.end();
        if (level != severity_levels.end()) {
            draw_severity_scale(width, level - severity_levels.begin() + 1, good);
        } else {
            const auto color = ImGui::GetStyleColorVec4(good ? ImGuiCol_Text : ImGuiCol_TextDisabled);
            ImGui::TextColored(color, "%u?", unsigned(*status.severity));
        }
    }
    if (value) {
        ImGui::SetItemTooltip("%s", value->label.ru.empty() ? value->name.c_str() : value->label.ru.c_str());
    } else {
        ImGui::SetItemTooltip("Неизвестная степень (%u)", unsigned(*status.severity));
    }
}

} // namespace


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
    if (ImGui::BeginTable("status_table", 2, flags)) {
        const float width = ImGui::CalcTextSize("255?").x;
        ImGui::TableSetupColumn("Статус", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Степень", ImGuiTableColumnFlags_WidthFixed, width);
        ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
        ImGui::TableSetColumnIndex(0);
        ImGui::TableHeader("Статус");
        if (ImGui::TableSetColumnIndex(1)) {
            draw_severity_header(width);
        }

        for (const auto& status : service.statuses()) {
            const auto& field = *status.field;
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::PushTextWrapPos(0.0f);
            ImGui::TextUnformatted(field.label.ru.empty() ? field.name.c_str() : field.label.ru.c_str());
            ImGui::PopTextWrapPos();
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
                ImGui::SetTooltip("%s", field.name.c_str());
            }

            if (ImGui::TableSetColumnIndex(1)) {
                draw_severity(status, *description, width, good);
            }
        }
        ImGui::EndTable();
    }

    ImGui::End();
}


} // namespace ui
