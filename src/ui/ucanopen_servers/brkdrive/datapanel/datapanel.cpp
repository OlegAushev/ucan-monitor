#include "datapanel.h"
#include <ui/util/checkbox_tristate.h>
#include <ui/util/style.h>
#include <ui/util/util.h>


namespace ui {
namespace brkdrive {


DataPanel::DataPanel(std::shared_ptr<::brkdrive::Server> server,
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
        ImGui::TextUnformatted(_server->drive_state().data());

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Run [bool]");
        ImGui::TableSetColumnIndex(1);
        if (_server->is_running().has_value()) {
            ImGui::Text("%d", _server->is_running().value());
            if (_server->is_running().value()) {
                ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ui::colors::table_bg_green);
            }
        } else {
            ImGui::TextUnformatted("n/a");
        }

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Error [bool]");
        ImGui::TableSetColumnIndex(1);
        if (_server->has_error().has_value()) {
            ImGui::Text("%d", _server->has_error().value());
            if (_server->has_error().value()) {
                ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ui::colors::table_bg_red);
            }
        } else {
            ImGui::TextUnformatted("n/a");
        }

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Warning [bool]");
        ImGui::TableSetColumnIndex(1);
        if (_server->has_warning().has_value()) {
            ImGui::Text("%d", _server->has_warning().value());
            if (_server->has_warning().value()) {
                ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ui::colors::table_bg_yellow);
            }
        } else {
            ImGui::TextUnformatted("n/a");
        }

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Operation Mode");
        ImGui::TableSetColumnIndex(1);
        ImGui::TextUnformatted(_server->opmode().data());

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Control Mode");
        ImGui::TableSetColumnIndex(1);
        ImGui::TextUnformatted(_server->ctlmode().data());

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Control Loop");
        ImGui::TableSetColumnIndex(1);
        ImGui::TextUnformatted(_server->ctlloop().data());

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Torque [%]");
        ImGui::TableSetColumnIndex(1);
        if (_server->torque().has_value()) {
            float torque_pct = _server->torque().value() * 100.0f;
            ImGui::Text("%.2f", torque_pct);
        } else {
            ImGui::TextUnformatted("n/a");
        }

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Speed [rpm]");
        ImGui::TableSetColumnIndex(1);
        if (_server->speed().has_value()) {
            ImGui::Text("%d", _server->speed().value());
        } else {
            ImGui::TextUnformatted("n/a");
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

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Braking Force [%]");
        ImGui::TableSetColumnIndex(1);
        if (_server->torque().has_value()) {
            float braking_pct = _server->braking_force().value() * 100.0f;
            ImGui::Text("%.2f", braking_pct);
        } else {
            ImGui::TextUnformatted("n/a");
        }

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

    // static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
    // if (ImGui::BeginTable("tpdo3_table", 2, flags)) {
    //     ImGui::TableSetupColumn("Parameter");
    //     ImGui::TableSetupColumn("Value");
    //     ImGui::TableHeadersRow();

    //     ImGui::TableNextRow();
    //     ImGui::TableSetColumnIndex(0);
    //     ImGui::TextUnformatted("Connection");
    //     ImGui::TableSetColumnIndex(1);
    //     _server->tpdo_service.good(ucanopen::CobTpdo::tpdo3) ? ImGui::TextUnformatted("ok") : ImGui::TextUnformatted("bad");

    //     ImGui::TableNextRow();
    //     ImGui::TableSetColumnIndex(0);
    //     ImGui::TextUnformatted("Raw Data [hex]");
    //     ImGui::TableSetColumnIndex(1);
    //     auto payload = _server->tpdo_service.data(ucanopen::CobTpdo::tpdo3);
    //     ImGui::Text("%02X %02X %02X %02X %02X %02X %02X %02X", payload[0], payload[1], payload[2], payload[3], payload[4], payload[5], payload[6], payload[7]);

    //     auto tpdo3 = _server->tpdo3();

    //     ImGui::TableNextRow();
    //     ImGui::TableSetColumnIndex(0);
    //     ImGui::TextUnformatted("PWR Temperature [°C]");
    //     ImGui::TableSetColumnIndex(1);
    //     ImGui::Text("%d", tpdo3.pwrmodule_temp);

    //     ImGui::TableNextRow();
    //     ImGui::TableSetColumnIndex(0);
    //     ImGui::TextUnformatted("EXC Temperature [°C]");
    //     ImGui::TableSetColumnIndex(1);
    //     ImGui::Text("%d", tpdo3.excmodule_temp);

    //     ImGui::TableNextRow();
    //     ImGui::TableSetColumnIndex(0);
    //     ImGui::TextUnformatted("PCB Temperature [°C]");
    //     ImGui::TableSetColumnIndex(1);
    //     ImGui::Text("%d", tpdo3.pcb_temp);

    //     ImGui::TableNextRow();
    //     ImGui::TableSetColumnIndex(0);
    //     ImGui::TextUnformatted("AW Temperature [°C]");
    //     ImGui::TableSetColumnIndex(1);
    //     ImGui::Text("%d", tpdo3.aw_temp);

    //     ImGui::TableNextRow();
    //     ImGui::TableSetColumnIndex(0);
    //     ImGui::TextUnformatted("FW Temperature [°C]");
    //     ImGui::TableSetColumnIndex(1);
    //     ImGui::Text("%d", tpdo3.fw_temp);

    //     ImGui::EndTable();
    // }    
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
        if (_server->errors().has_value()) {
            ImGui::Text("%d",_server->errors().value());
        } else {
            ImGui::TextUnformatted("n/a");
        }

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Warnings");
        ImGui::TableSetColumnIndex(1);
        if (_server->warnings().has_value()) {
            ImGui::Text("%d",_server->warnings().value());
        } else {
            ImGui::TextUnformatted("n/a");
        }

        ImGui::EndTable();
    }  
}


} // namespace srmdrive
} // namespace ui
