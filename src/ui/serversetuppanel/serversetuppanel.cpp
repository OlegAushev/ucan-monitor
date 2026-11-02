#include "serversetuppanel.h"
#include <ui/util/style.h>


namespace ui {


ServerSetupPanel::ServerSetupPanel(std::shared_ptr<ucanopen::Server> server,
                                   const std::string& menu_title,
                                   const std::string& window_title,
                                   bool open)
        : View(menu_title, window_title, open)
        , _server(server) {}


void ServerSetupPanel::draw() {
    ImGui::Begin(_window_title.c_str(), &_opened);

    _draw_about();

    ImGui::End();
}


void ServerSetupPanel::_draw_about() {
    ImGui::SeparatorText("О сервере");
    if (ImGui::Button(ICON_MDI_REFRESH " Обновить##about")) {
        _server->info_service.refresh();
    }

    auto const info = _server->info_service.info();
    auto const file_fingerprint = _server->description()->fingerprint();
    for (const auto& entry : info.entries) {
        const auto& label = entry.item.label.ru.empty() ? entry.item.path : entry.item.label.ru;
        ImGui::TextDisabled("%s: ", label.c_str());
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
            ImGui::SetTooltip("%s", entry.item.path.c_str());
        }
        ImGui::SameLine();
        if (entry.item.path == "dictionary.fingerprint" && info.fingerprint.has_value()) {
            ImGui::Text("0x%08X", *info.fingerprint);
        } else {
            ImGui::TextUnformatted(entry.value.has_value() ? entry.value->c_str() : "н/д");
        }
    }

    ImGui::TextDisabled("Описание словаря: ");
    ImGui::SameLine();
    ImGui::Text("%s, отпечаток 0x%08X", _server->description()->device().c_str(), file_fingerprint);
    if (_server->info_service.fingerprint_matches() == false) {
        ImGui::PushStyleColor(ImGuiCol_Text, colors::icon_yellow);
        ImGui::TextWrapped("Описание словаря не от этой прошивки: в устройстве 0x%08X, в файле 0x%08X.",
                           info.fingerprint.value_or(0),
                           file_fingerprint);
        ImGui::PopStyleColor();
    }
}


} // namespace ui
