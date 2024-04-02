#include "watchpanel.h"
#include <ui/util/checkbox_tristate.h>


namespace ui {


WatchPanel::WatchPanel(std::shared_ptr<ucanopen::Server> server,
                       const std::string& menu_title,
                       const std::string& window_title,
                       bool open)
        : View(menu_title, window_title, open)
        , _server(server)
{
    _watch_objects = _server->watch_service.objects();
}


void WatchPanel::draw() {
    ImGui::Begin(_window_title.c_str(), &is_open);

    static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
    if (ImGui::BeginTable("watch_table", 3, flags)) {
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 20.0f);
        ImGui::TableSetupColumn("Parameter");
        ImGui::TableSetupColumn("Value");
        //ImGui::TableHeadersRow();

        static int all_acq_enabled = 1;

        ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
        for (int col = 0; col < 3; ++col)
        {
            ImGui::TableSetColumnIndex(col);
            if (col == 0) {
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
                if (ui::CheckBoxTristate("##select_all", all_acq_enabled)) {
                    for (size_t i = 0; i < _watch_objects.size(); ++i) {
                        _server->watch_service.enable_acq(i, all_acq_enabled);
                    }
                }
                ImGui::PopStyleVar();
                ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
            }
            ImGui::TableHeader(ImGui::TableGetColumnName(col));
        }

        for (size_t row = 0; row < _watch_objects.size(); ++row) {
            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            bool acq_enabled = _server->watch_service.acq_enabled(row);
            ImGui::PushID(row);
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
            if (ImGui::Checkbox("##", &acq_enabled)) {
                _server->watch_service.enable_acq(row, acq_enabled);
                all_acq_enabled = -1;
            }
            ImGui::PopStyleVar();
            ImGui::PopID();

            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%s[%s]", _watch_objects[row]->name.c_str(), _watch_objects[row]->unit.c_str());
            ImGui::TableSetColumnIndex(2);
            ImGui::TextUnformatted(_server->watch_service.string_value(_watch_objects[row]->subcategory, _watch_objects[row]->name).c_str());
        }
        ImGui::EndTable();
    }

    ImGui::End();
}


} // namespace ui
