#include "log.h"


namespace ui {


void Log::draw(bool& open) {
    ImGui::Begin("Log", &open);

    if (ImGui::BeginChild("Log Scrolling Region", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar)) {
        //ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing
        std::string newline;
        if (std::getline(*_stream, newline)) {
            _lines.push_back(newline);
        } else {
            _stream->clear();
        }

        for (const auto& line : _lines) {
            ImGui::TextUnformatted(line.c_str());
        }

        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
            ImGui::SetScrollHereY(1.0f);
        }
    }
    ImGui::EndChild();

    ImGui::End();
}


} // namespace ui
