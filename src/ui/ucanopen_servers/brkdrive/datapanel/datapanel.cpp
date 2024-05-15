#include "datapanel.h"
#include <ui/util/checkbox_tristate.h>
#include <ui/util/style.h>


namespace ui {
namespace brkdrive {


DataPanel::DataPanel(std::shared_ptr<::srmdrive::Server> server,
                     const std::string& menu_title,
                     const std::string& window_title,
                     bool open)
        : View(menu_title, window_title, open)
        , _server(server)
{}


void DataPanel::draw() {
    ImGui::Begin(_window_title.c_str(), &_opened);
    _draw_tpdo1_table();
    ImGui::NewLine();
    _draw_tpdo2_table();
    ImGui::NewLine();
    _draw_tpdo3_table();
    ImGui::NewLine();
    _draw_tpdo4_table();
    ImGui::End();
}


void DataPanel::_draw_tpdo1_table() {
    static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
    if (ImGui::BeginTable("tpdo1_table", 2, flags)) {
        ImGui::TableSetupColumn("Parameter");
        ImGui::TableSetupColumn("Value");
        ImGui::TableHeadersRow();

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Connection");
        ImGui::TableSetColumnIndex(1);
        _server->tpdo_service.good(ucanopen::CobTpdo::tpdo1) ? ImGui::TextUnformatted("ok") : ImGui::TextUnformatted("bad");

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Raw Data [hex]");
        ImGui::TableSetColumnIndex(1);
        auto payload = _server->tpdo_service.data(ucanopen::CobTpdo::tpdo1);
        ImGui::Text("%02X %02X %02X %02X %02X %02X %02X %02X", payload[0], payload[1], payload[2], payload[3], payload[4], payload[5], payload[6], payload[7]);

        auto tpdo1 = _server->tpdo1();

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Drive State");
        ImGui::TableSetColumnIndex(1);
        ImGui::TextUnformatted(tpdo1.drive_state.data());

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Run [bool]");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%d", tpdo1.run);
        if (tpdo1.run != 0) {
            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ui::colors::table_bg_green);
        }

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Error [bool]");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%d", tpdo1.error);
        if (tpdo1.error) {
            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ui::colors::table_bg_red);
        }

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Warning [bool]");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%d", tpdo1.warning);
        if (tpdo1.warning) {
            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ui::colors::table_bg_yellow);
        }

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Overheat [bool]");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%d", tpdo1.overheat);
        if (tpdo1.overheat) {
            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ui::colors::table_bg_yellow);
        }

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Control Mode");
        ImGui::TableSetColumnIndex(1);
        ImGui::TextUnformatted(tpdo1.ctlmode.data());

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Control Loop");
        ImGui::TableSetColumnIndex(1);
        ImGui::TextUnformatted(tpdo1.ctlloop.data());

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Torque [Nm]");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%d", tpdo1.torque);

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Speed [rpm]");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%d", tpdo1.speed);

        ImGui::EndTable();
    }
}


void DataPanel::_draw_tpdo2_table() {
    static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
    if (ImGui::BeginTable("tpdo2_table", 2, flags)) {
        ImGui::TableSetupColumn("Parameter");
        ImGui::TableSetupColumn("Value");
        ImGui::TableHeadersRow();

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Connection");
        ImGui::TableSetColumnIndex(1);
        _server->tpdo_service.good(ucanopen::CobTpdo::tpdo2) ? ImGui::TextUnformatted("ok") : ImGui::TextUnformatted("bad");

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Raw Data [hex]");
        ImGui::TableSetColumnIndex(1);
        auto payload = _server->tpdo_service.data(ucanopen::CobTpdo::tpdo2);
        ImGui::Text("%02X %02X %02X %02X %02X %02X %02X %02X", payload[0], payload[1], payload[2], payload[3], payload[4], payload[5], payload[6], payload[7]);

        auto tpdo2 = _server->tpdo2();

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("DC Voltage [V]");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%d", tpdo2.dc_voltage);

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Stator Current [A]");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%d", tpdo2.stator_current);

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Field Current [A]");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%.0f", tpdo2.field_current);

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Mech Power [kW]");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%d", tpdo2.mech_power);

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Manual Field Current [bool]");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%d", tpdo2.manual_field_current);

        ImGui::EndTable();
    }
}


void DataPanel::_draw_tpdo3_table() {
    static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
    if (ImGui::BeginTable("tpdo3_table", 2, flags)) {
        ImGui::TableSetupColumn("Parameter");
        ImGui::TableSetupColumn("Value");
        ImGui::TableHeadersRow();

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Connection");
        ImGui::TableSetColumnIndex(1);
        _server->tpdo_service.good(ucanopen::CobTpdo::tpdo3) ? ImGui::TextUnformatted("ok") : ImGui::TextUnformatted("bad");

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Raw Data [hex]");
        ImGui::TableSetColumnIndex(1);
        auto payload = _server->tpdo_service.data(ucanopen::CobTpdo::tpdo3);
        ImGui::Text("%02X %02X %02X %02X %02X %02X %02X %02X", payload[0], payload[1], payload[2], payload[3], payload[4], payload[5], payload[6], payload[7]);

        auto tpdo3 = _server->tpdo3();

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("PWR Temperature [°C]");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%d", tpdo3.pwrmodule_temp);

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("EXC Temperature [°C]");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%d", tpdo3.excmodule_temp);

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("PCB Temperature [°C]");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%d", tpdo3.pcb_temp);

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("AW Temperature [°C]");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%d", tpdo3.aw_temp);

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("FW Temperature [°C]");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%d", tpdo3.fw_temp);

        ImGui::EndTable();
    }    
}


void DataPanel::_draw_tpdo4_table() {
    static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
    if (ImGui::BeginTable("watch_table", 2, flags)) {
        ImGui::TableSetupColumn("Parameter");
        ImGui::TableSetupColumn("Value");
        ImGui::TableHeadersRow();

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Connection");
        ImGui::TableSetColumnIndex(1);
        _server->tpdo_service.good(ucanopen::CobTpdo::tpdo4) ? ImGui::TextUnformatted("ok") : ImGui::TextUnformatted("bad");

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Raw Data [hex]");
        ImGui::TableSetColumnIndex(1);
        auto payload = _server->tpdo_service.data(ucanopen::CobTpdo::tpdo4);
        ImGui::Text("%02X %02X %02X %02X %02X %02X %02X %02X", payload[0], payload[1], payload[2], payload[3], payload[4], payload[5], payload[6], payload[7]);

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Errors");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%d",_server->errors());

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Warnings");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%d",_server->warnings());

        ImGui::EndTable();
    }  
}


} // namespace srmdrive
} // namespace ui
