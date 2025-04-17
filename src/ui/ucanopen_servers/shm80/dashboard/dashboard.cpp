#include "dashboard.hpp"
#include <ui/util/checkbox_tristate.h>
#include <ui/util/util.h>

using namespace shm80;

namespace ui {
namespace shm80 {

Dashboard::Dashboard(std::shared_ptr<::shm80::Server> server,
                     const std::string& menu_title,
                     const std::string& window_title,
                     bool open)
        : View(menu_title, window_title, open), server_(server) {}

void Dashboard::draw() {
    // ImGuiWindow* window = ImGui::GetCurrentWindow();
    // float menubar_height = window->MenuBarHeight;
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowSize(ImGui::GetWindowSize()); // GetIO().DisplaySize);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration;
    ImGui::Begin(_window_title.c_str(), &_opened, flags);

    {
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;
        ImGui::BeginChild("Преобразователь",
                          ImVec2(500, 560),
                          ImGuiChildFlags_Border,
                          window_flags);
        ImGui::SeparatorText("Преобразователь");

        // Heartbeat indicator
        if (server_->heartbeat_service.good()) {
            ui::util::BlinkingText(ICON_MDI_NETWORK,
                                   std::chrono::milliseconds{750},
                                   ui::colors::icon_green,
                                   ui::colors::icon_inactive);
        } else {
            ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_red);
            ImGui::TextUnformatted(ICON_MDI_CLOSE_NETWORK);
            ImGui::PopStyleColor();
        }

        // Drive state indicator
        ImGui::SameLine();
        ImGui::PushItemWidth(300);
        std::string network_status;
        if (server_->heartbeat_service.good()) {
            network_status = "В СЕТИ";
        } else {
            network_status = "НЕ В СЕТИ";
        }

        std::string state(server_->drive_state_str_upper());
        std::string status = network_status + " | " + state;

        ImGui::InputText("##status",
                         status.data(),
                         status.size(),
                         ImGuiInputTextFlags_ReadOnly);
        ImGui::PopItemWidth();

        static ImGuiTableFlags flags =
                ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
        if (ImGui::BeginTable("converter_table", 2, flags)) {
            ImGui::TableSetupColumn("Параметр");
            ImGui::TableSetupColumn("Значение",
                                    ImGuiTableColumnFlags_WidthFixed,
                                    140.f);
            ImGui::TableHeadersRow();

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted(ICON_MDI_CAR_BATTERY " Напряжение [В]");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%.1f", server_->dc_voltage());

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted(ICON_MDI_THERMOMETER " Температура A [°C]");
            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted(
                    server_->watch_service.string_value("converter", "tA")
                            .c_str());
            color_cell(server_->watch_service.value("converter", "tA").f32(),
                       converter_warn_temp,
                       converter_err_temp);

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted(ICON_MDI_THERMOMETER " Температура B [°C]");
            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted(
                    server_->watch_service.string_value("converter", "tB")
                            .c_str());
            color_cell(server_->watch_service.value("converter", "tB").f32(),
                       converter_warn_temp,
                       converter_err_temp);

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted(ICON_MDI_THERMOMETER " Температура C [°C]");
            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted(
                    server_->watch_service.string_value("converter", "tC")
                            .c_str());
            color_cell(server_->watch_service.value("converter", "tC").f32(),
                       converter_warn_temp,
                       converter_err_temp);

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted(ICON_MDI_THERMOMETER " Температура F [°C]");
            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted(
                    server_->watch_service.string_value("converter", "tF")
                            .c_str());
            color_cell(server_->watch_service.value("converter", "tF").f32(),
                       converter_warn_temp,
                       converter_err_temp);

            ImGui::EndTable();
        }

        // Heartbeat indicator
        if (true) {
            ui::util::BlinkingText(ICON_MDI_NETWORK,
                                   std::chrono::milliseconds{750},
                                   ui::colors::icon_green,
                                   ui::colors::icon_inactive);
        } else {
            ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_red);
            ImGui::TextUnformatted(ICON_MDI_CLOSE_NETWORK);
            ImGui::PopStyleColor();
        }

        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_green);
        ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED);
        ImGui::PopStyleColor();
        ImGui::SameLine();
        ImGui::TextUnformatted("Аварийно низкое напряжение");

        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_green);
        ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED);
        ImGui::PopStyleColor();
        ImGui::SameLine();
        ImGui::TextUnformatted("Аварийно высокое напряжение");

        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_green);
        ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED);
        ImGui::PopStyleColor();
        ImGui::SameLine();
        ImGui::TextUnformatted("Аварийно высокий фазный ток");

        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_yellow);
        ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED);
        ImGui::PopStyleColor();
        ImGui::SameLine();
        ImGui::TextUnformatted("Аварийно высокий ток возб.");

        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_green);
        ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED);
        ImGui::PopStyleColor();
        ImGui::SameLine();
        ImGui::TextUnformatted("Перегрев");

        ImGui::EndChild();
    }

    ImGui::SameLine();

    {
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;
        ImGui::BeginChild("Двигатель",
                          ImVec2(500, 560),
                          ImGuiChildFlags_Border,
                          window_flags);
        ImGui::SeparatorText("Двигатель");

        static ImGuiTableFlags flags =
                ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
        if (ImGui::BeginTable("motor_table", 2, flags)) {
            ImGui::TableSetupColumn("Параметр");
            ImGui::TableSetupColumn("Значение",
                                    ImGuiTableColumnFlags_WidthFixed,
                                    140.f);
            ImGui::TableHeadersRow();

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted(ICON_MDI_SPEEDOMETER " Частота [об/мин]");
            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted("1802");

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted(ICON_MDI_CURRENT_AC " Ток ОС [A]");
            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted("145");

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted(ICON_MDI_CURRENT_DC " Ток ОВ [A]");
            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted("8");

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted(ICON_MDI_THERMOMETER " Температура ОС [°C]");
            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted("102");

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted(ICON_MDI_THERMOMETER " Температура ОВ [°C]");
            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted("185");

            ImGui::EndTable();
        }

        float throttle_pct = 0.73f * 200.f - 100.f;
        ImGui::PushItemWidth(200);
        ImGui::SliderFloat(ICON_MDI_SPEEDOMETER "##accl_slider",
                           &throttle_pct,
                           -100.f,
                           100.0f,
                           "%.0f",
                           ImGuiSliderFlags_NoInput);
        ImGui::PopItemWidth();

        float speed = 1802.f;

        ImGui::SameLine();
        if (speed < 0) {
            ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_green);
        } else {
            ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_red);
        }
        ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED " R");
        ImGui::PopStyleColor();

        ImGui::SameLine();
        if (speed == 0) {
            ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_green);
        } else {
            ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_red);
        }
        ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED " N");
        ImGui::PopStyleColor();

        ImGui::SameLine();
        if (speed > 0) {
            ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_green);
        } else {
            ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_red);
        }
        ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED " D");
        ImGui::PopStyleColor();

        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_green);
        ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED);
        ImGui::PopStyleColor();
        ImGui::SameLine();
        ImGui::TextUnformatted("Отказ датчика положения");

        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_green);
        ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED);
        ImGui::PopStyleColor();
        ImGui::SameLine();
        ImGui::TextUnformatted("Перегрев обмотки статора");

        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_red);
        ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED);
        ImGui::PopStyleColor();
        ImGui::SameLine();
        ImGui::TextUnformatted("Перегрев обмотки возбуждения");

        ImGui::EndChild();
    }

    ImGui::End();
}

} // namespace shm80
} // namespace ui
