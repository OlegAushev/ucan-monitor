#include "datapanel.h"
#include <ui/util/checkbox_tristate.h>
#include <ui/util/style.h>
#include <ui/util/util.h>

using namespace brkdrive;

namespace ui {
namespace brkdrive {

DataPanel::DataPanel(std::shared_ptr<::brkdrive::Server> server,
                     const std::string& menu_title,
                     const std::string& window_title,
                     bool open)
        : View(menu_title, window_title, open), _server(server) {}

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
        ui::util::BlinkingText(ICON_MDI_NETWORK,
                               std::chrono::milliseconds{750},
                               ui::colors::icon_green,
                               ui::colors::icon_inactive);
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_red);
        ImGui::TextUnformatted(ICON_MDI_CLOSE_NETWORK);
        ImGui::PopStyleColor();
    }

    ImGui::SameLine();
    ImGui::SeparatorText("TPDO1");

    static ImGuiTableFlags flags =
            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
    if (ImGui::BeginTable("tpdo1_table", 2, flags)) {
        ImGui::TableSetupColumn("Parameter");
        ImGui::TableSetupColumn("Value");
        ImGui::TableHeadersRow();

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Raw Data [hex]");
        ImGui::TableSetColumnIndex(1);
        auto payload = _server->tpdo_service.data(ucanopen::CobTpdo::tpdo1);
        ImGui::Text("%02X %02X %02X %02X %02X %02X %02X %02X",
                    payload[0],
                    payload[1],
                    payload[2],
                    payload[3],
                    payload[4],
                    payload[5],
                    payload[6],
                    payload[7]);

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Operating Status");
        ImGui::TableSetColumnIndex(1);
        ImGui::TextUnformatted(
                ::brkdrive::opstatus_names.at(_server->opstatus()).data());
        switch (_server->opstatus()) {
        case ::brkdrive::OperatingStatus::inoperable:
            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg,
                                   ui::colors::table_bg_red);
            break;
        case ::brkdrive::OperatingStatus::calibrating:
            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg,
                                   ui::colors::table_bg_yellow);
            break;
        case ::brkdrive::OperatingStatus::operable:
            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg,
                                   ui::colors::table_bg_green);
            break;
        }

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Drive State");
        ImGui::TableSetColumnIndex(1);
        ImGui::TextUnformatted(
                ::brkdrive::drive_state_names.at(_server->drive_state())
                        .data());

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Calibrated [bool]");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%d", _server->is_calibrated());
        if (_server->is_calibrated()) {
            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg,
                                   ui::colors::table_bg_green);
        } else {
            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg,
                                   ui::colors::table_bg_red);
        }

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Calibration State");
        ImGui::TableSetColumnIndex(1);
        ImGui::TextUnformatted(
                calibration_state_names.at(_server->calstatus()).data());
        switch (_server->calstatus()) {
        case CalibrationState::standby:
            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg,
                                   ui::colors::table_bg_red);
            break;
        case CalibrationState::completed:
            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg,
                                   ui::colors::table_bg_green);
            break;
        default:
            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg,
                                   ui::colors::table_bg_yellow);
            break;
        }

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("PWM [bool]");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%d", _server->is_pwm_on());
        if (_server->is_pwm_on()) {
            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg,
                                   ui::colors::table_bg_green);
        }

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Error [bool]");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%d", _server->has_error());
        if (_server->has_error()) {
            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg,
                                   ui::colors::table_bg_red);
        }

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Warning [bool]");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%d", _server->has_warning());
        if (_server->has_warning()) {
            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg,
                                   ui::colors::table_bg_yellow);
        }

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Operating Mode");
        ImGui::TableSetColumnIndex(1);
        ImGui::TextUnformatted(
                ::brkdrive::opmode_names.at(_server->opmode()).data());

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Control Mode");
        ImGui::TableSetColumnIndex(1);
        ImGui::TextUnformatted(
                ::brkdrive::ctlmode_names.at(_server->ctlmode()).data());

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Control Loop");
        ImGui::TableSetColumnIndex(1);
        ImGui::TextUnformatted(
                ::brkdrive::ctlloop_names.at(_server->ctlloop()).data());

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Angle [rad]");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%.2f", _server->angle());

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Trk Completed [bool]");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%d", _server->is_trk_completed());
        if (_server->is_trk_completed()) {
            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg,
                                   ui::colors::table_bg_green);
        } else {
            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg,
                                   ui::colors::table_bg_red);
        }

        ImGui::EndTable();
    }
}

void DataPanel::_draw_tpdo2_table() {
    if (_server->tpdo_service.good(ucanopen::CobTpdo::tpdo2)) {
        ui::util::BlinkingText(ICON_MDI_NETWORK,
                               std::chrono::milliseconds{750},
                               ui::colors::icon_green,
                               ui::colors::icon_inactive);
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_red);
        ImGui::TextUnformatted(ICON_MDI_CLOSE_NETWORK);
        ImGui::PopStyleColor();
    }

    ImGui::SameLine();
    ImGui::SeparatorText("TPDO2");

    static ImGuiTableFlags flags =
            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
    if (ImGui::BeginTable("tpdo2_table", 2, flags)) {
        ImGui::TableSetupColumn("Parameter");
        ImGui::TableSetupColumn("Value");
        ImGui::TableHeadersRow();

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Raw Data [hex]");
        ImGui::TableSetColumnIndex(1);
        auto payload = _server->tpdo_service.data(ucanopen::CobTpdo::tpdo2);
        ImGui::Text("%02X %02X %02X %02X %02X %02X %02X %02X",
                    payload[0],
                    payload[1],
                    payload[2],
                    payload[3],
                    payload[4],
                    payload[5],
                    payload[6],
                    payload[7]);

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Ref Angle [rad]");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%.2f", _server->ref_angle());

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Braking Force [%]");
        ImGui::TableSetColumnIndex(1);
        int braking_pct = _server->ref_brake() * 100.0f;
        ImGui::Text("%d", braking_pct);

        ImGui::EndTable();
    }
}

void DataPanel::_draw_tpdo3_table() {
    if (_server->tpdo_service.good(ucanopen::CobTpdo::tpdo3)) {
        ui::util::BlinkingText(ICON_MDI_NETWORK,
                               std::chrono::milliseconds{750},
                               ui::colors::icon_green,
                               ui::colors::icon_inactive);
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_red);
        ImGui::TextUnformatted(ICON_MDI_CLOSE_NETWORK);
        ImGui::PopStyleColor();
    }

    ImGui::SameLine();
    ImGui::SeparatorText("TPDO3");

    static ImGuiTableFlags flags =
            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
    if (ImGui::BeginTable("tpdo3_table", 2, flags)) {
        ImGui::TableSetupColumn("Parameter");
        ImGui::TableSetupColumn("Value");
        ImGui::TableHeadersRow();

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Connection");
        ImGui::TableSetColumnIndex(1);
        _server->tpdo_service.good(ucanopen::CobTpdo::tpdo3) ?
                ImGui::TextUnformatted("ok") :
                ImGui::TextUnformatted("bad");

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Raw Data [hex]");
        ImGui::TableSetColumnIndex(1);
        auto payload = _server->tpdo_service.data(ucanopen::CobTpdo::tpdo3);
        ImGui::Text("%02X %02X %02X %02X %02X %02X %02X %02X",
                    payload[0],
                    payload[1],
                    payload[2],
                    payload[3],
                    payload[4],
                    payload[5],
                    payload[6],
                    payload[7]);

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Speed [rpm]");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%d", _server->speed());

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Temp. PCB-A [°C]");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%d", _server->pcba_temp());

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Temp. PCB-B [°C]");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%d", _server->pcbb_temp());

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Temp. PCB-C [°C]");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%d", _server->pcbc_temp());

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Temp. Mot-A [°C]");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%d", _server->mota_temp());

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Temp. Mot-B [°C]");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%d", _server->motb_temp());

        ImGui::EndTable();
    }
}

void DataPanel::_draw_tpdo4_table() {
    if (_server->tpdo_service.good(ucanopen::CobTpdo::tpdo4)) {
        ui::util::BlinkingText(ICON_MDI_NETWORK,
                               std::chrono::milliseconds{750},
                               ui::colors::icon_green,
                               ui::colors::icon_inactive);
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_red);
        ImGui::TextUnformatted(ICON_MDI_CLOSE_NETWORK);
        ImGui::PopStyleColor();
    }

    ImGui::SameLine();
    ImGui::SeparatorText("TPDO4");

    static ImGuiTableFlags flags =
            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
    if (ImGui::BeginTable("tpdo4_table", 2, flags)) {
        ImGui::TableSetupColumn("Parameter");
        ImGui::TableSetupColumn("Value");
        ImGui::TableHeadersRow();

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Raw Data [hex]");
        ImGui::TableSetColumnIndex(1);
        auto payload = _server->tpdo_service.data(ucanopen::CobTpdo::tpdo4);
        ImGui::Text("%02X %02X %02X %02X %02X %02X %02X %02X",
                    payload[0],
                    payload[1],
                    payload[2],
                    payload[3],
                    payload[4],
                    payload[5],
                    payload[6],
                    payload[7]);

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Errors");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%d", _server->errors());

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Warnings");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%d", _server->warnings());

        ImGui::EndTable();
    }
}

} // namespace brkdrive
} // namespace ui
