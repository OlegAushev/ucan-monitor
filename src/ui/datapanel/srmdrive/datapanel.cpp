#include "datapanel.h"


namespace ui {
namespace srmdrive {


void DataPanel::draw(bool& open) {
    if (ImGui::Begin("Data", &open)) {
        _draw_watch_table();
        ImGui::NewLine();
        _draw_tpdo1_table();
        ImGui::End();
    }
}


void DataPanel::_draw_watch_table() {
    static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
    if (ImGui::BeginTable("watch_table", 3, flags)) {
        ImGui::TableSetupColumn("Parameter");
        ImGui::TableSetupColumn("Value");
        ImGui::TableSetupColumn("Unit");
        ImGui::TableHeadersRow();

        const auto& watch_objects = _server->watch_service.objects();

        for (int row = 0; row < watch_objects.size(); ++row) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted(std::string(watch_objects[row]->name).c_str());
            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted(_server->watch_service.value_str(watch_objects[row]->subcategory, watch_objects[row]->name).c_str());
            ImGui::TableSetColumnIndex(2);
            ImGui::TextUnformatted(std::string(watch_objects[row]->unit).c_str());
        }
        ImGui::EndTable();
    }
}


void DataPanel::_draw_tpdo1_table() {
    static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
    if (ImGui::BeginTable("watch_table", 3, flags)) {
        ImGui::TableSetupColumn("Parameter");
        ImGui::TableSetupColumn("Value");
        ImGui::TableSetupColumn("Unit");
        ImGui::TableHeadersRow();

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Connection");
        ImGui::TableSetColumnIndex(1);
        _server->tpdo_service.is_ok(ucanopen::CobTpdo::tpdo1) ? ImGui::TextUnformatted("ok") : ImGui::TextUnformatted("bad");


        ImGui::EndTable();
    }
}


} // namespace srmdrive
} // namespace ui
