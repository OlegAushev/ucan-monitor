#include "datapanel.hpp"
#include <ui/util/style.h>
#include <ui/util/util.h>

#include <array>
#include <utility>

namespace ui {
namespace pdu {

DataPanel::DataPanel(std::shared_ptr<::pdu::Server> server,
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

void DataPanel::draw_pdo_header(ucanopen::CobTpdo tpdo, const char* title) {
    if (server_->tpdo_service.good(tpdo)) {
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
    ImGui::SeparatorText(title);
}

void DataPanel::draw_payload_row(ucanopen::CobTpdo tpdo) {
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::TextUnformatted("Сообщение [hex]");
    ImGui::TableSetColumnIndex(1);
    auto payload = server_->tpdo_service.data(tpdo);
    ImGui::Text("%02X %02X %02X %02X %02X %02X %02X %02X",
                payload[0],
                payload[1],
                payload[2],
                payload[3],
                payload[4],
                payload[5],
                payload[6],
                payload[7]);
}

void DataPanel::draw_tpdo1_table() {
    draw_pdo_header(ucanopen::CobTpdo::tpdo1, "TPDO1");

    static ImGuiTableFlags flags =
            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
    if (ImGui::BeginTable("tpdo1_table", 2, flags)) {
        ImGui::TableSetupColumn("Параметр");
        ImGui::TableSetupColumn("Значение");
        ImGui::TableHeadersRow();

        draw_payload_row(ucanopen::CobTpdo::tpdo1);

        struct Row {
            const char* title;
            bool active;
            ImU32 bg;
        };
        const std::array<Row, 4> rows = {
                Row{"Предупреждения",
                    server_->has_any_warning(),
                    ui::colors::table_bg_yellow},
                Row{"Ошибки", server_->has_error(), ui::colors::table_bg_red},
                Row{"Крит. Ошибки",
                    server_->has_critical(),
                    ui::colors::table_bg_red},
                Row{"Аварии",
                    server_->has_emergency(),
                    ui::colors::table_bg_red}};

        for (auto const& row : rows) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted(row.title);
            ImGui::TableSetColumnIndex(1);
            if (row.active) {
                ImGui::TextUnformatted("да");
                ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, row.bg);
            } else {
                ImGui::TextUnformatted("нет");
            }
        }

        ImGui::EndTable();
    }
}

void DataPanel::draw_branch_rows(const char* branch_name,
                                 float voltage,
                                 float current,
                                 ::pdu::ContactorPosition main_command,
                                 ::pdu::ContactorPosition main_feedback,
                                 ::pdu::ContactorPosition precharge_command,
                                 ::pdu::ContactorPosition precharge_feedback,
                                 ::pdu::BranchState state) {
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::Text("Напряжение %s [В]", branch_name);
    ImGui::TableSetColumnIndex(1);
    ImGui::Text("%.1f", voltage);

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::Text("Ток %s [А]", branch_name);
    ImGui::TableSetColumnIndex(1);
    ImGui::Text("%.1f", current);

    const std::array<std::pair<const char*, ::pdu::ContactorPosition>, 4>
            contactors = {
                    std::pair{"Главный: команда", main_command},
                    std::pair{"Главный: обратная связь", main_feedback},
                    std::pair{"Предзаряд: команда", precharge_command},
                    std::pair{"Предзаряд: обратная связь",
                              precharge_feedback}};

    for (auto const& [title, position] : contactors) {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted(title);
        ImGui::TableSetColumnIndex(1);
        ImGui::TextUnformatted(
                ::pdu::Server::contactor_position_str(position).data());
        if (position == ::pdu::ContactorPosition::closed) {
            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg,
                                   ui::colors::table_bg_green);
        }
    }

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::TextUnformatted("Состояние ветви");
    ImGui::TableSetColumnIndex(1);
    ImGui::TextUnformatted(::pdu::Server::branch_state_str(state).data());
    if (state == ::pdu::BranchState::lockout) {
        ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg,
                               ui::colors::table_bg_red);
    }
}

void DataPanel::draw_tpdo2_table() {
    draw_pdo_header(ucanopen::CobTpdo::tpdo2, "TPDO2 - ЭХГ");

    static ImGuiTableFlags flags =
            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
    if (ImGui::BeginTable("tpdo2_table", 2, flags)) {
        ImGui::TableSetupColumn("Параметр");
        ImGui::TableSetupColumn("Значение");
        ImGui::TableHeadersRow();

        draw_payload_row(ucanopen::CobTpdo::tpdo2);
        draw_branch_rows("ЭХГ",
                         server_->fuelcell_voltage(),
                         server_->fuelcell_current(),
                         server_->fuelcell_main_command(),
                         server_->fuelcell_main_feedback(),
                         server_->fuelcell_precharge_command(),
                         server_->fuelcell_precharge_feedback(),
                         server_->fuelcell_state());

        ImGui::EndTable();
    }
}

void DataPanel::draw_tpdo3_table() {
    draw_pdo_header(ucanopen::CobTpdo::tpdo3, "TPDO3 - Инвертор");

    static ImGuiTableFlags flags =
            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
    if (ImGui::BeginTable("tpdo3_table", 2, flags)) {
        ImGui::TableSetupColumn("Параметр");
        ImGui::TableSetupColumn("Значение");
        ImGui::TableHeadersRow();

        draw_payload_row(ucanopen::CobTpdo::tpdo3);
        draw_branch_rows("инвертора",
                         server_->inverter_voltage(),
                         server_->inverter_current(),
                         server_->inverter_main_command(),
                         server_->inverter_main_feedback(),
                         server_->inverter_precharge_command(),
                         server_->inverter_precharge_feedback(),
                         server_->inverter_state());

        ImGui::EndTable();
    }
}

void DataPanel::draw_tpdo4_table() {
    draw_pdo_header(ucanopen::CobTpdo::tpdo4, "TPDO4");

    static ImGuiTableFlags flags =
            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
    if (ImGui::BeginTable("tpdo4_table", 2, flags)) {
        ImGui::TableSetupColumn("Параметр");
        ImGui::TableSetupColumn("Значение");
        ImGui::TableHeadersRow();

        draw_payload_row(ucanopen::CobTpdo::tpdo4);

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Напряжение батареи [В]");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%.1f", server_->battery_voltage());

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Температура МК [°C]");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%.1f", server_->mcu_temperature());

        ImGui::EndTable();
    }
}

} // namespace pdu
} // namespace ui
