#include "motordatapanel.h"
#include "imgui.h"


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
    for (size_t i = 0; i < 4; ++i) {
        ImGui::Begin(_window_titles[i].c_str(), &is_open);

        static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
        if (ImGui::BeginTable("data_table", 2, flags)) {
            ImGui::TableSetupColumn("Parameter");
            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthFixed, 120.0f);
            ImGui::TableHeadersRow();

            auto data = _server->motordrive_data[i].load();

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted("Mode [bool]");
            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted(data.ctlmode.data());

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted("Enabled [bool]");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%d", data.enabled);
            if (data.enabled) {
                ImU32 cell_bg_color = ImGui::GetColorU32(ImVec4(0.3f, 0.7f, 0.3f, 0.65f));
                ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, cell_bg_color);
            } else {
                ImU32 cell_bg_color = ImGui::GetColorU32(ImVec4(0.7f, 0.3f, 0.3f, 0.65f));
                ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, cell_bg_color);
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
            ImGui::TextUnformatted(data.discharge.data());

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted("Fault Level");
            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted(data.faultlevel.data());

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted("Fault Code [hex]");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("0x%02X", data.faultcode);

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted("Errors [hex]");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("0x%08X", data.errors);

            ImGui::EndTable();
        }

        ImGui::End();
    }
}


} // namespace atvvcu
} // namespace ui
