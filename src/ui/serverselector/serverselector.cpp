#include "serverselector.h"


namespace ui {


void ServerSelector::show_server_selector_window(const std::vector<std::string>& server_names) {
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;
    
    ImGui::SetNextWindowPos(ImVec2{0, 0});
    
    ImGui::Begin("Select Server", nullptr, window_flags);

    for (auto i = 0; i < server_names.size(); ++i) {
        ImGui::RadioButton(server_names[i].c_str(), &_server_selected_id, i);
    }

    ImGui::NewLine();

    if (ImGui::Button("Ok", ImVec2{50, 0})) {
        _is_selected = true;
    }
    


    ImGui::End();
}


} // namespace ui
