#include "datapanel.hpp"
#include <ui/util/checkbox_tristate.h>
#include <ui/util/style.h>
#include <ui/util/util.h>

using namespace sevpress;

namespace ui {
namespace sevpress {

DataPanel::DataPanel(std::shared_ptr<::sevpress::Server> server,
                     const std::string& menu_title,
                     const std::string& window_title,
                     bool open)
        : View(menu_title, window_title, open), server_(server) {}

void DataPanel::draw() {
    ImGui::Begin(_window_title.c_str(), &_opened);
    draw_tpdo1_table();
    ImGui::NewLine();
    draw_tpdo2_table();
    ImGui::NewLine();
    draw_tpdo3_table();
    ImGui::NewLine();
    draw_tpdo4_table();
    ImGui::End();
}

void DataPanel::draw_tpdo1_table() {
    if (server_->tpdo_service.good(ucanopen::CobTpdo::tpdo1)) {
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
        ImGui::TableSetupColumn("Параметр");
        ImGui::TableSetupColumn("Значение");
        ImGui::TableHeadersRow();

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Сообщение [hex]");
        ImGui::TableSetColumnIndex(1);
        auto payload = server_->tpdo_service.data(ucanopen::CobTpdo::tpdo1);
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
        ImGui::TextUnformatted("Состояние");
        ImGui::TableSetColumnIndex(1);
        ImGui::TextUnformatted(server_->drive_state_str().data());

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Режим Работы");
        ImGui::TableSetColumnIndex(1);
        ImGui::TextUnformatted(server_->opmode_str().data());

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Режим Управления");
        ImGui::TableSetColumnIndex(1);
        ImGui::TextUnformatted(server_->ctlmode_str().data());

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Режим СУ");
        ImGui::TableSetColumnIndex(1);
        ImGui::TextUnformatted(server_->ctlloop_str().data());

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("ШИМ");
        ImGui::TableSetColumnIndex(1);
        if (server_->is_pwm_on()) {
            ImGui::TextUnformatted("вкл");
            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg,
                                   ui::colors::table_bg_green);
        } else {
            ImGui::TextUnformatted("выкл");
        }

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Ошибки");
        ImGui::TableSetColumnIndex(1);
        if (server_->has_any_error()) {
            ImGui::TextUnformatted("да");
            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg,
                                   ui::colors::table_bg_red);
        } else {
            ImGui::TextUnformatted("нет");
        }

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Предупреждения");
        ImGui::TableSetColumnIndex(1);
        if (server_->has_any_warning()) {
            ImGui::TextUnformatted("да");
            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg,
                                   ui::colors::table_bg_yellow);
        } else {
            ImGui::TextUnformatted("нет");
        }

        ImGui::EndTable();
    }
}

void DataPanel::draw_tpdo2_table() {
    if (server_->tpdo_service.good(ucanopen::CobTpdo::tpdo2)) {
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
        ImGui::TableSetupColumn("Параметр");
        ImGui::TableSetupColumn("Значение");
        ImGui::TableHeadersRow();

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Сообщение [hex]");
        ImGui::TableSetColumnIndex(1);
        auto payload = server_->tpdo_service.data(ucanopen::CobTpdo::tpdo2);
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
        ImGui::TextUnformatted("Напряжение ЗПТ [В]");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%.1f", server_->dc_voltage());

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Ток ОЯ [А]");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%.1f", server_->stator_current());

        ImGui::EndTable();
    }
}

void DataPanel::draw_tpdo3_table() {
    if (server_->tpdo_service.good(ucanopen::CobTpdo::tpdo3)) {
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
        ImGui::TableSetupColumn("Параметр");
        ImGui::TableSetupColumn("Значения");
        ImGui::TableHeadersRow();

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Сообщение [hex]");
        ImGui::TableSetColumnIndex(1);
        auto payload = server_->tpdo_service.data(ucanopen::CobTpdo::tpdo3);
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
        ImGui::TextUnformatted("Скорость [об/мин]");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%d", server_->speed());

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Угол [°]");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%d", server_->angle());

        ImGui::EndTable();
    }
}

void DataPanel::draw_tpdo4_table() {
    if (server_->tpdo_service.good(ucanopen::CobTpdo::tpdo4)) {
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
        ImGui::TableSetupColumn("Параметр");
        ImGui::TableSetupColumn("Значение");
        ImGui::TableHeadersRow();

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Сообщение [hex]");
        ImGui::TableSetColumnIndex(1);
        auto payload = server_->tpdo_service.data(ucanopen::CobTpdo::tpdo4);
        ImGui::Text("%02X %02X %02X %02X %02X %02X %02X %02X",
                    payload[0],
                    payload[1],
                    payload[2],
                    payload[3],
                    payload[4],
                    payload[5],
                    payload[6],
                    payload[7]);

        // ImGui::TableNextRow();
        // ImGui::TableSetColumnIndex(0);
        // ImGui::TextUnformatted("Errors");
        // ImGui::TableSetColumnIndex(1);
        // ImGui::Text("%d",server_->errors());

        // ImGui::TableNextRow();
        // ImGui::TableSetColumnIndex(0);
        // ImGui::TextUnformatted("Warnings");
        // ImGui::TableSetColumnIndex(1);
        // ImGui::Text("%d",server_->warnings());

        ImGui::EndTable();
    }
}

} // namespace sevpress
} // namespace ui
