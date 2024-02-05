#include "bmspanel.h"
#include <ui/components/togglebutton.h>


namespace ui {
namespace atvvcu {


BmsPanel::BmsPanel(std::shared_ptr<::atvvcu::Server> server,
                   const std::string& menu_title,
                   const std::string& window_title,
                   bool open)
        : View(menu_title, window_title, open)
        , _server(server)
{}


void BmsPanel::draw() {
    ImGui::Begin(_window_title.c_str(), &is_open);

    static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
    if (ImGui::BeginTable("contactor_table", 2, flags)) {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Charge[%]");
        ImGui::TableSetColumnIndex(1);
        ImGui::TextUnformatted(_server->watch_service.string_value("bms", "charge_pct").c_str());

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Voltage[V]");
        ImGui::TableSetColumnIndex(1);
        ImGui::TextUnformatted(_server->watch_service.string_value("bms", "v_batt").c_str());

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Current[A]");
        ImGui::TableSetColumnIndex(1);
        ImGui::TextUnformatted(_server->watch_service.string_value("bms", "i_batt").c_str());

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Tcellmin[o]");
        ImGui::TableSetColumnIndex(1);
        ImGui::TextUnformatted(_server->watch_service.string_value("bms", "t_cellmin").c_str());

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Tcellmax[o]");
        ImGui::TableSetColumnIndex(1);
        ImGui::TextUnformatted(_server->watch_service.string_value("bms", "t_cellmax").c_str());

        ImGui::EndTable();
    }

    ImGui::End();
}


} // namespace atvvcu
} // namespace ui
