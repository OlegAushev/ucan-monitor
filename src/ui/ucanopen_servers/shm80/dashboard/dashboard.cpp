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
                          ImVec2(600, 720),
                          ImGuiChildFlags_Border,
                          window_flags);
        ImGui::SeparatorText("Преобразователь");

        // Heartbeat indicator
        if (server_->heartbeat_service.good()) {
            ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_green);
            ImGui::TextUnformatted(ICON_MDI_CLOSE_NETWORK);
            ImGui::PopStyleColor();
        } else {
            ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_red);
            ImGui::TextUnformatted(ICON_MDI_CLOSE_NETWORK);
            ImGui::PopStyleColor();
        }

        // Drive state indicator
        ImGui::SameLine();
        ImGui::PushItemWidth(460);
        std::string network_status;
        if (server_->heartbeat_service.good()) {
            network_status = "В СЕТИ";
        } else {
            network_status = "НЕ В СЕТИ";
        }

        std::string state;
        if (server_->has_error()) {
            state = "ОШИБКА";
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ui::colors::icon_red);
        } else {
            state = server_->drive_state_str_upper();
            auto color = ImGui::GetStyleColorVec4(ImGuiCol_FrameBg);
            ImGui::PushStyleColor(ImGuiCol_FrameBg, color);
        }
        std::string status = network_status + " | " + state;

        ImGui::InputText("##status",
                         status.data(),
                         status.size(),
                         ImGuiInputTextFlags_ReadOnly);
        ImGui::PopStyleColor();
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

        draw_discretes();
        draw_pdu();

        ImGui::EndChild();
    }

    ImGui::SameLine();

    {
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;
        ImGui::BeginChild("Двигатель",
                          ImVec2(600, 720),
                          ImGuiChildFlags_Border,
                          window_flags);
        ImGui::SeparatorText("Двигатель");

        if (server_->throttle_good()) {
            ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_green);
            ImGui::TextUnformatted(ICON_MDI_SPEEDOMETER);
            ImGui::PopStyleColor();
        } else {
            ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_red);
            ImGui::TextUnformatted(ICON_MDI_SPEEDOMETER);
            ImGui::PopStyleColor();
        }

        ImGui::SameLine();
        float throttle_pct = server_->throttle();
        ImGui::PushItemWidth(200);
        ImGui::SliderFloat("##accl_slider",
                           &throttle_pct,
                           0.f,
                           100.0f,
                           "%.0f",
                           ImGuiSliderFlags_NoInput);
        ImGui::PopItemWidth();

        ImGui::SameLine();
        Gear gear = server_->gear();

        if (gear == Gear::reverse || gear == Gear::reverse_pre) {
            ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_green);
        } else {
            ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_red);
        }
        ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED "R");
        ImGui::PopStyleColor();

        ImGui::SameLine();
        if (gear == Gear::neutral) {
            ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_green);
        } else {
            ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_red);
        }
        ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED "N");
        ImGui::PopStyleColor();

        ImGui::SameLine();
        if (gear == Gear::drive || gear == Gear::drive_pre) {
            ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_green);
        } else {
            ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_red);
        }
        ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED "D");
        ImGui::PopStyleColor();

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
            ImGui::Text("%d", server_->speed());

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted(ICON_MDI_CURRENT_AC " Ток ОЯ [A]");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%.1f", server_->stator_current());

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted(ICON_MDI_CURRENT_DC " Ток ОВ [A]");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%.1f", server_->field_current());

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted(ICON_MDI_THERMOMETER " Температура ОЯ [°C]");
            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted(
                    server_->watch_service.string_value("motor", "tAW")
                            .c_str());
            color_cell(server_->watch_service.value("motor", "tAW").f32(),
                       motor_warn_temp,
                       motor_err_temp);

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted(ICON_MDI_THERMOMETER " Температура ОВ [°C]");
            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted(
                    server_->watch_service.string_value("motor", "tFW")
                            .c_str());
            color_cell(server_->watch_service.value("motor", "tFW").f32(),
                       motor_warn_temp,
                       motor_err_temp);

            ImGui::EndTable();
        }
        ImGui::EndChild();
    }

    ImGui::End();
}

void Dashboard::draw_discretes() {
    ImGui::TextUnformatted("ВХ/ВЫХ:");

    ImGui::SameLine();

    if (server_->din_ship_failure()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_red);
    } else if (server_->din_ship_failure_warning()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_yellow);
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_inactive);
    }
    ImGui::TextUnformatted(ICON_MDI_FERRY);
    ImGui::PopStyleColor();

    ImGui::SameLine();

    if (server_->dout_drive_failure()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_red);
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_inactive);
    }
    ImGui::TextUnformatted(ICON_MDI_ENGINE);
    ImGui::PopStyleColor();

    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_inactive);
    ImGui::TextUnformatted(ICON_MDI_DRAG_VERTICAL_VARIANT);
    ImGui::PopStyleColor();
    ImGui::SameLine();

    if (server_->dout_power_request()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_green);
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_inactive);
    }
    ImGui::TextUnformatted(ICON_MDI_POWER);
    ImGui::PopStyleColor();

    ImGui::SameLine();

    if (server_->dout_drive_ready()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_green);
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_inactive);
    }
    ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED);
    ImGui::PopStyleColor();

    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_inactive);
    ImGui::TextUnformatted(ICON_MDI_DRAG_VERTICAL_VARIANT);
    ImGui::PopStyleColor();
    ImGui::SameLine();

    if (server_->din_start()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_green);
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_inactive);
    }
    ImGui::TextUnformatted(ICON_MDI_SPEEDOMETER);
    ImGui::PopStyleColor();

    ImGui::SameLine();

    if (server_->dout_drive_started()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_green);
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_inactive);
    }
    ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED);
    ImGui::PopStyleColor();
}

void Dashboard::draw_pdu() {
    if (server_->pdu_good()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_green);
        ImGui::TextUnformatted(ICON_MDI_CLOSE_NETWORK);
        ImGui::PopStyleColor();
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_red);
        ImGui::TextUnformatted(ICON_MDI_CLOSE_NETWORK);
        ImGui::PopStyleColor();
    }

    ImGui::SameLine();
    ImGui::TextUnformatted("ПДУ:");
    ImGui::SameLine();

    if (server_->pdu_main_contactor()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_green);
        ImGui::TextUnformatted("ВХ" ICON_MDI_ELECTRIC_SWITCH_CLOSED);
        ImGui::PopStyleColor();
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_red);
        ImGui::TextUnformatted("ВХ" ICON_MDI_ELECTRIC_SWITCH);
        ImGui::PopStyleColor();
    }

    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_inactive);
    ImGui::TextUnformatted(ICON_MDI_DRAG_VERTICAL_VARIANT);
    ImGui::PopStyleColor();
    ImGui::SameLine();

    if (server_->pdu_charging_contactor()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_green);
        ImGui::TextUnformatted("ЗАРЯД" ICON_MDI_ELECTRIC_SWITCH_CLOSED);
        ImGui::PopStyleColor();
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_red);
        ImGui::TextUnformatted("ЗАРЯД" ICON_MDI_ELECTRIC_SWITCH);
        ImGui::PopStyleColor();
    }

    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_inactive);
    ImGui::TextUnformatted(ICON_MDI_DRAG_VERTICAL_VARIANT);
    ImGui::PopStyleColor();
    ImGui::SameLine();

    if (server_->pdu_bypassing_contactor()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_green);
        ImGui::TextUnformatted("ШУНТ" ICON_MDI_ELECTRIC_SWITCH_CLOSED);
        ImGui::PopStyleColor();
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_red);
        ImGui::TextUnformatted("ШУНТ" ICON_MDI_ELECTRIC_SWITCH);
        ImGui::PopStyleColor();
    }
}

} // namespace shm80
} // namespace ui
