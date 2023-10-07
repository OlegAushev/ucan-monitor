#include "datapanel.h"


namespace ui {
namespace srmdrive {


void DataPanel::draw(bool& open) {
    if (ImGui::Begin("Data", &open)) {
        _draw_watch_table();
        ImGui::End();
    }
}


void DataPanel::_draw_watch_table() {
    static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
    if (ImGui::BeginTable("watch_table", 2, flags)) {
        for (int row = 0; row < _server->watch_service.objects().size(); row++)
        {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted(std::string(_server->watch_service.objects()[row].second).c_str());
            // for (int column = 0; column < 3; column++)
            // {
            //     ImGui::TableSetColumnIndex(column);
            //     // char buf[32];
            //     // sprintf(buf, "Hello %d,%d", column, row);
            //     // ImGui::Button(buf, ImVec2(-FLT_MIN, 0.0f));
            // }
        }
        ImGui::EndTable();
    }
}


} // namespace srmdrive
} // namespace ui
