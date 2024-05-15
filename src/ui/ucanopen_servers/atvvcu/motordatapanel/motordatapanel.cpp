#include "motordatapanel.h"
#include "imgui.h"
#include <ui/util/style.h>


namespace ui {
namespace atvvcu {


MotorDataPanel::MotorDataPanel(std::shared_ptr<::atvvcu::Server> server,
                               const std::string& menu_title,
                               const std::string& window_title,
                               bool open)
        : View(menu_title, window_title, open)
        , _server(server)
{}


void MotorDataPanel::draw() {
    auto data = _server->drive.data();


    for (size_t i = 0; i < 4; ++i) {
        ImGui::Begin(_window_titles[i].c_str(), &_opened);

        static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
        if (ImGui::BeginTable("data_table", 2, flags)) {
            ImGui::TableSetupColumn("Parameter");
            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthFixed, 120.0f);
            ImGui::TableHeadersRow();

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted("Mode [bool]");
            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted(data[i].ctlmode.data());

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted("Enabled [bool]");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%d", data[i].enabled);
            if (data[i].enabled) {
                ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ui::colors::table_bg_green);
            } else {
                ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ui::colors::table_bg_red);
            }

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted("Vdc [V]");
            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted(_server->watch_service.string_value("motordrive", _vdc_watchobj[i]).c_str());

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted("Iac [A]");
            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted(_server->watch_service.string_value("motordrive", _iac_watchobj[i]).c_str());

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted("Speed [rpm]");
            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted(_server->watch_service.string_value("motordrive", _speed_watchobj[i]).c_str());

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted("Torque [Nm]");
            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted(_server->watch_service.string_value("motordrive", _torque_watchobj[i]).c_str());

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted("Motor Temp [o]");
            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted(_server->watch_service.string_value("motordrive", _motortemp_watchobj[i]).c_str());

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted("Module Temp [o]");
            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted(_server->watch_service.string_value("motordrive", _moduletemp_watchobj[i]).c_str());

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted("DC Link");
            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted(data[i].discharge.data());

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted("Fault Level");
            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted(data[i].faultlevel.data());

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted("Fault Code [hex]");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("0x%02X", data[i].faultcode);

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted("Errors [hex]");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("0x%08X", data[i].errors);

            ImGui::EndTable();
        }

        ImGui::End();
    }
}


} // namespace atvvcu
} // namespace ui
