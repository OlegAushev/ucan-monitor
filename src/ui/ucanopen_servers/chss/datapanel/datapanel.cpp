#include "datapanel.hpp"
#include <ui/util/style.h>
#include <ui/util/util.h>

#include <array>
#include <utility>

namespace ui {
namespace chss {

DataPanel::DataPanel(std::shared_ptr<::chss::Server> server,
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

void DataPanel::draw_value_row(const char* title, float value) {
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::TextUnformatted(title);
    ImGui::TableSetColumnIndex(1);
    ImGui::Text("%.1f", value);
}

void DataPanel::draw_valve_row(const char* title,
                               ::chss::ValvePosition position) {
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::TextUnformatted(title);
    ImGui::TableSetColumnIndex(1);
    ImGui::TextUnformatted(
            ::chss::Server::valve_position_str(position).data());
    if (position == ::chss::ValvePosition::open) {
        ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg,
                               ui::colors::table_bg_green);
    }
}

void DataPanel::draw_fact_row(const char* title, bool active) {
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::TextUnformatted(title);
    ImGui::TableSetColumnIndex(1);
    if (active) {
        ImGui::TextUnformatted("да");
        ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg,
                               ui::colors::table_bg_green);
    } else {
        ImGui::TextUnformatted("нет");
    }
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

void DataPanel::draw_tpdo2_table() {
    draw_pdo_header(ucanopen::CobTpdo::tpdo2, "TPDO2 - Заправка");

    static ImGuiTableFlags flags =
            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
    if (ImGui::BeginTable("tpdo2_table", 2, flags)) {
        ImGui::TableSetupColumn("Параметр");
        ImGui::TableSetupColumn("Значение");
        ImGui::TableHeadersRow();

        draw_payload_row(ucanopen::CobTpdo::tpdo2);
        draw_value_row("Давление в ресивере Р1-1 [атм]",
                       server_->receiver_pressure());
        draw_value_row("Расход на входе ИР1 [л/мин]", server_->inflow_rate());
        draw_value_row("Давление в линии заправки Р1 [атм]",
                       server_->fill_line_pressure());
        draw_valve_row("Клапан заправки К7", server_->inlet_valve());

        ImGui::EndTable();
    }
}

void DataPanel::draw_tpdo3_table() {
    draw_pdo_header(ucanopen::CobTpdo::tpdo3, "TPDO3 - Подача");

    static ImGuiTableFlags flags =
            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
    if (ImGui::BeginTable("tpdo3_table", 2, flags)) {
        ImGui::TableSetupColumn("Параметр");
        ImGui::TableSetupColumn("Значение");
        ImGui::TableHeadersRow();

        draw_payload_row(ucanopen::CobTpdo::tpdo3);
        draw_value_row("Давление до редуктора Р2 [атм]",
                       server_->pressure_before_reducer());
        draw_value_row("Давление после редуктора Р3 [атм]",
                       server_->pressure_after_reducer());
        draw_valve_row("Клапан выпуска К9", server_->outlet_valve());

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
        draw_value_row("Температура МК [°C]", server_->mcu_temperature());

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Режим");
        ImGui::TableSetColumnIndex(1);
        ImGui::TextUnformatted(
                ::chss::Server::mode_str(server_->mode()).data());

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Стадия");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%s (%u)",
                    server_->stage_str().data(),
                    static_cast<unsigned>(server_->stage()));
        if (server_->stage() == std::to_underlying(::chss::Stage::lockout)) {
            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg,
                                   ui::colors::table_bg_red);
        }

        draw_fact_row("Подача установлена", server_->supply_ready());
        draw_fact_row("Ресивер полон", server_->receiver_full());

        ImGui::EndTable();
    }
}

} // namespace chss
} // namespace ui
