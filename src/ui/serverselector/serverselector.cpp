#include "serverselector.h"

namespace ui {

void ServerSelector::show(const std::vector<std::string>& server_names,
                          ImVec2 size) {
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking |
                                    ImGuiWindowFlags_NoResize |
                                    ImGuiWindowFlags_NoMove
                                    //| ImGuiWindowFlags_NoTitleBar
                                    | ImGuiWindowFlags_NoCollapse |
                                    ImGuiWindowFlags_NoBringToFrontOnFocus |
                                    ImGuiWindowFlags_NoNavFocus;
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    //ImGui::SetNextWindowPos(viewport->WorkPos);
    //ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowPos({(viewport->WorkSize.x - size.x) / 2,
                             (viewport->WorkSize.y - size.y) / 2});
    ImGui::SetNextWindowSize({size.x, size.y});
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGui::Begin("Выбор Сервера", nullptr, window_flags);

    for (size_t i = 0; i < server_names.size(); ++i) {
        ImGui::RadioButton(server_names[i].c_str(), &_selected_server_idx, i);
    }

    ImGui::NewLine();

    if (ImGui::Button("Ok", ImVec2{100, 0})) {
        _server_is_selected = true;
        _selected_server = server_names[_selected_server_idx];
    }

    ImGui::End();
}

} // namespace ui
