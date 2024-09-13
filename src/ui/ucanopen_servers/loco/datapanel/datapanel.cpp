#include "datapanel.h"
#include <ui/util/style.h>
#include <ui/util/util.h>


using namespace loco;


namespace ui {
namespace loco {


DataPanel::DataPanel(std::shared_ptr<::loco::Server> server,
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
    if (_server->tpdo_service.good(ucanopen::CobTpdo::tpdo1)) {
        ui::util::BlinkingText(ICON_MDI_NETWORK, std::chrono::milliseconds{750},
                               ui::colors::icon_green, ui::colors::icon_inactive);
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_red);
        ImGui::TextUnformatted(ICON_MDI_CLOSE_NETWORK);
        ImGui::PopStyleColor();
    }
    
    ImGui::SameLine();
    ImGui::SeparatorText("TPDO1");

    static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
    if (ImGui::BeginTable("tpdo1_table", 2, flags)) {
        ImGui::TableSetupColumn("Parameter");
        ImGui::TableSetupColumn("Value");
        ImGui::TableHeadersRow();

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Raw Data [hex]");
        ImGui::TableSetColumnIndex(1);
        auto payload = _server->tpdo_service.data(ucanopen::CobTpdo::tpdo1);
        ImGui::Text("%02X %02X %02X %02X %02X %02X %02X %02X", payload[0], payload[1], payload[2], payload[3], payload[4], payload[5], payload[6], payload[7]);

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Drive State");
        ImGui::TableSetColumnIndex(1);
        ImGui::TextUnformatted(drive_state_names.at(_server->drive_state()).data());

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("PWM [bool]");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%d", _server->pwm_on());
        if (_server->pwm_on()) {
            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ui::colors::table_bg_green);
        }

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Error [bool]");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%d", _server->has_error());
        if (_server->has_error()) {
            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ui::colors::table_bg_red);
        }

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Warning [bool]");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%d", _server->has_warning());
        if (_server->has_warning()) {
            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ui::colors::table_bg_yellow);
        }

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Operating Mode");
        ImGui::TableSetColumnIndex(1);
        ImGui::TextUnformatted(opmode_names.at(_server->opmode()).data());

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Control Mode");
        ImGui::TableSetColumnIndex(1);
        ImGui::TextUnformatted(ctlmode_names.at(_server->ctlmode()).data());

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Control Loop");
        ImGui::TableSetColumnIndex(1);
        ImGui::TextUnformatted(ctlloop_names.at(_server->ctlloop()).data());

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Torque [%]");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%.2f", _server->torque());

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Speed [rpm]");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%d", _server->speed());
        
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Manual Field [bool]");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%d", _server->manual_field());
        if (_server->manual_field()) {
            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ui::colors::table_bg_yellow);
        }

        ImGui::EndTable();
    }
}


void DataPanel::_draw_tpdo2_table() {
    if (_server->tpdo_service.good(ucanopen::CobTpdo::tpdo2)) {
        ui::util::BlinkingText(ICON_MDI_NETWORK, std::chrono::milliseconds{750},
                               ui::colors::icon_green, ui::colors::icon_inactive);
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_red);
        ImGui::TextUnformatted(ICON_MDI_CLOSE_NETWORK);
        ImGui::PopStyleColor();
    }
    
    ImGui::SameLine();
    ImGui::SeparatorText("TPDO2");

    static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
    if (ImGui::BeginTable("tpdo2_table", 2, flags)) {
        ImGui::TableSetupColumn("Parameter");
        ImGui::TableSetupColumn("Value");
        ImGui::TableHeadersRow();

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Raw Data [hex]");
        ImGui::TableSetColumnIndex(1);
        auto payload = _server->tpdo_service.data(ucanopen::CobTpdo::tpdo2);
        ImGui::Text("%02X %02X %02X %02X %02X %02X %02X %02X", payload[0], payload[1], payload[2], payload[3], payload[4], payload[5], payload[6], payload[7]);

        ImGui::EndTable();
    }
}


void DataPanel::_draw_tpdo3_table() {
    if (_server->tpdo_service.good(ucanopen::CobTpdo::tpdo3)) {
        ui::util::BlinkingText(ICON_MDI_NETWORK, std::chrono::milliseconds{750},
                               ui::colors::icon_green, ui::colors::icon_inactive);
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_red);
        ImGui::TextUnformatted(ICON_MDI_CLOSE_NETWORK);
        ImGui::PopStyleColor();
    }
    
    ImGui::SameLine();
    ImGui::SeparatorText("TPDO3");

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

        ImGui::EndTable();
    }    
}


void DataPanel::_draw_tpdo4_table() {
    if (_server->tpdo_service.good(ucanopen::CobTpdo::tpdo4)) {
        ui::util::BlinkingText(ICON_MDI_NETWORK, std::chrono::milliseconds{750},
                               ui::colors::icon_green, ui::colors::icon_inactive);
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_red);
        ImGui::TextUnformatted(ICON_MDI_CLOSE_NETWORK);
        ImGui::PopStyleColor();
    }
    
    ImGui::SameLine();
    ImGui::SeparatorText("TPDO4");

    static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
    if (ImGui::BeginTable("tpdo4_table", 2, flags)) {
        ImGui::TableSetupColumn("Parameter");
        ImGui::TableSetupColumn("Value");
        ImGui::TableHeadersRow();

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


} // namespace loco
} // namespace ui
