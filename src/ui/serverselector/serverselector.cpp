#include "serverselector.h"


namespace ui {


void ServerSelector::show(const std::vector<std::string>& server_names) {
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;
    
    ImGui::SetNextWindowPos(ImVec2{0, 0});
    
    ImGui::Begin("Select Server", nullptr, window_flags);

    for (auto i = 0; i < server_names.size(); ++i) {
        ImGui::RadioButton(server_names[i].c_str(), &_selected_server_idx, i);
    }

    ImGui::NewLine();

    if (ImGui::Button("Ok", ImVec2{50, 0})) {
        _server_is_selected = true;
        _selected_server = server_names[_selected_server_idx];
    }
    
    ImGui::End();
}


} // namespace ui
