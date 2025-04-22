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
        : View(menu_title, window_title, open), server_(server) {
    error_update_timepoint_ = std::chrono::steady_clock::now();
}

void Dashboard::draw() {
    // ImGuiWindow* window = ImGui::GetCurrentWindow();
    // float menubar_height = window->MenuBarHeight;
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowSize(ImGui::GetWindowSize()); // GetIO().DisplaySize);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration;
    ImGui::Begin(_window_title.c_str(), &_opened, flags);

    draw_converter();
    ImGui::SameLine();
    draw_motor();

    draw_pdu();
    ImGui::SameLine();
    draw_throttle();

    ImGui::End();
}

void Dashboard::draw_converter() {
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;
    ImGui::BeginChild("Преобразователь",
                      topcell_size,
                      ImGuiChildFlags_Border,
                      window_flags);
    ImGui::SeparatorText("Преобразователь");

    // Heartbeat indicator
    if (server_->heartbeat_service.good()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_green);
        ImGui::TextUnformatted(ICON_MDI_NETWORK);
        ImGui::PopStyleColor();
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_red);
        ImGui::TextUnformatted(ICON_MDI_CLOSE_NETWORK);
        ImGui::PopStyleColor();
    }

    // Drive state indicator
    ImGui::SameLine();
    ImGui::PushItemWidth(-1);
    std::string network_status;
    if (server_->heartbeat_service.good()) {
        network_status = "В СЕТИ";
    } else {
        network_status = "НЕ В СЕТИ";
    }

    std::string state;
    bool const is_in_error =
            server_->has_error() || !server_->heartbeat_service.good();
    if (is_in_error) {
        state = "ОШИБКА";
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ui::colors::icon_red);
    } else {
        state = server_->drive_state_str_upper();
    }
    std::string status = network_status + " | " + state;

    ImGui::InputText("##status",
                     status.data(),
                     status.size(),
                     ImGuiInputTextFlags_ReadOnly);
    if (is_in_error) {
        ImGui::PopStyleColor();
    }
    ImGui::PopItemWidth();

    static ImGuiTableFlags flags =
            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
    if (ImGui::BeginTable("converter_table", 2, flags)) {
        ImGui::TableSetupColumn("Параметр");
        ImGui::TableSetupColumn("Значение",
                                ImGuiTableColumnFlags_WidthFixed,
                                table_val_width);
        // ImGui::TableHeadersRow();

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted(ICON_MDI_ALPHA_V_CIRCLE_OUTLINE
                               " Напряжение ЗПТ [В]");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%.1f", server_->dc_voltage());

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted(ICON_MDI_THERMOMETER " Температура A [°C]");
        ImGui::TableSetColumnIndex(1);
        ImGui::TextUnformatted(
                server_->watch_service.string_value("converter", "tA").c_str());
        color_cell(server_->watch_service.value("converter", "tA").f32(),
                   converter_warn_temp,
                   converter_err_temp);

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted(ICON_MDI_THERMOMETER " Температура B [°C]");
        ImGui::TableSetColumnIndex(1);
        ImGui::TextUnformatted(
                server_->watch_service.string_value("converter", "tB").c_str());
        color_cell(server_->watch_service.value("converter", "tB").f32(),
                   converter_warn_temp,
                   converter_err_temp);

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted(ICON_MDI_THERMOMETER " Температура C [°C]");
        ImGui::TableSetColumnIndex(1);
        ImGui::TextUnformatted(
                server_->watch_service.string_value("converter", "tC").c_str());
        color_cell(server_->watch_service.value("converter", "tC").f32(),
                   converter_warn_temp,
                   converter_err_temp);

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted(ICON_MDI_THERMOMETER " Температура F [°C]");
        ImGui::TableSetColumnIndex(1);
        ImGui::TextUnformatted(
                server_->watch_service.string_value("converter", "tF").c_str());
        color_cell(server_->watch_service.value("converter", "tF").f32(),
                   converter_warn_temp,
                   converter_err_temp);

        ImGui::EndTable();
    }

    draw_discretes();

    draw_errors();

    ImGui::EndChild();
}

void Dashboard::draw_motor() {
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;
    ImGui::BeginChild("Двигатель",
                      topcell_size,
                      ImGuiChildFlags_Border,
                      window_flags);
    ImGui::SeparatorText("Двигатель");

    ImGui::TextUnformatted(ICON_MDI_SPEEDOMETER);
    ImGui::SameLine();
    int16_t const speed = server_->speed();
    ImGui::ProgressBar(std::fabs(static_cast<float>(speed)) / motor_speed_max);

    static ImGuiTableFlags flags =
            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
    if (ImGui::BeginTable("motor_table", 2, flags)) {
        ImGui::TableSetupColumn("Параметр");
        ImGui::TableSetupColumn("Значение",
                                ImGuiTableColumnFlags_WidthFixed,
                                table_val_width);
        // ImGui::TableHeadersRow();

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted(ICON_MDI_SPEEDOMETER " Частота [об/мин]");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%d", server_->speed());

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted(ICON_MDI_ALPHA_I_CIRCLE_OUTLINE " Ток ОЯ [A]");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%.1f", server_->stator_current());

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted(ICON_MDI_ALPHA_I_CIRCLE_OUTLINE " Ток ОВ [A]");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%.1f", server_->field_current());

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted(ICON_MDI_THERMOMETER " Температура ОЯ [°C]");
        ImGui::TableSetColumnIndex(1);
        ImGui::TextUnformatted(
                server_->watch_service.string_value("motor", "tAW").c_str());
        color_cell(server_->watch_service.value("motor", "tAW").f32(),
                   motor_warn_temp,
                   motor_err_temp);

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted(ICON_MDI_THERMOMETER " Температура ОВ [°C]");
        ImGui::TableSetColumnIndex(1);
        ImGui::TextUnformatted(
                server_->watch_service.string_value("motor", "tFW").c_str());
        color_cell(server_->watch_service.value("motor", "tFW").f32(),
                   motor_warn_temp,
                   motor_err_temp);

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        char const* icon1;
        if (!server_->gearsensors_good()) {
            icon1 = ICON_MDI_CLOSE_NETWORK;
        } else {
            icon1 = ICON_MDI_THERMOMETER;
        }
        ImGui::Text("%s %s", icon1, "Температура ОЖ1 [°C]");
        ImGui::TableSetColumnIndex(1);

        ImGui::TextUnformatted(
                server_->watch_service.string_value("reduction_gear", "tIn")
                        .c_str());
        color_cell(server_->watch_service.value("reduction_gear", "tIn").f32(),
                   reduction_gear_warn_temp,
                   reduction_gear_err_temp);

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        char const* icon2;
        if (!server_->gearsensors_good()) {
            icon2 = ICON_MDI_CLOSE_NETWORK;
        } else {
            icon2 = ICON_MDI_THERMOMETER;
        }
        ImGui::Text("%s %s", icon2, "Температура ОЖ2 [°C]");
        ImGui::TableSetColumnIndex(1);

        ImGui::TextUnformatted(
                server_->watch_service.string_value("reduction_gear", "tOut")
                        .c_str());
        color_cell(server_->watch_service.value("reduction_gear", "tOut").f32(),
                   reduction_gear_warn_temp,
                   reduction_gear_err_temp);

        ImGui::EndTable();
    }
    ImGui::EndChild();
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
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;
    ImGui::BeginChild("ПДУ",
                      bottomcell_size,
                      ImGuiChildFlags_Border,
                      window_flags);
    ImGui::SeparatorText("ПДУ");

    if (server_->pdu_good()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_green);
        ImGui::TextUnformatted(ICON_MDI_NETWORK);
        ImGui::PopStyleColor();
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_red);
        ImGui::TextUnformatted(ICON_MDI_CLOSE_NETWORK);
        ImGui::PopStyleColor();
    }

    ImGui::SameLine();
    ImGui::PushItemWidth(-1);
    std::string network_status;
    if (!server_->pdu_good()) {
        network_status = "НЕ В СЕТИ";
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ui::colors::icon_red);
    } else {
        network_status = "В СЕТИ";
    }

    ImGui::InputText("##status",
                     network_status.data(),
                     network_status.size(),
                     ImGuiInputTextFlags_ReadOnly);
    if (!server_->pdu_good()) {
        ImGui::PopStyleColor();
    }
    ImGui::PopItemWidth();

    static ImGuiTableFlags flags =
            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
    if (ImGui::BeginTable("converter_table", 2, flags)) {
        ImGui::TableSetupColumn("Параметр");
        ImGui::TableSetupColumn("Значение",
                                ImGuiTableColumnFlags_WidthFixed,
                                table_val_width);
        // ImGui::TableHeadersRow();

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted(ICON_MDI_CAR_BATTERY " Напряжение АБ [В]");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%.1f",
                    server_->watch_service.value("pdu", "v_batt").f32());

        ImGui::EndTable();
    }

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

    ImGui::EndChild();
}

void Dashboard::draw_throttle() {
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;
    ImGui::BeginChild("Контроллер хода",
                      bottomcell_size,
                      ImGuiChildFlags_Border,
                      window_flags);
    ImGui::SeparatorText("Контроллер хода");

    if (server_->throttle_good()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_green);
        ImGui::TextUnformatted(ICON_MDI_NETWORK);
        ImGui::PopStyleColor();
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_red);
        ImGui::TextUnformatted(ICON_MDI_CLOSE_NETWORK);
        ImGui::PopStyleColor();
    }

    ImGui::SameLine();
    ImGui::PushItemWidth(-1);
    std::string network_status;
    if (!server_->throttle_good()) {
        network_status = "НЕ В СЕТИ";
        network_status.append(" | ВКЛ N");
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ui::colors::icon_red);
    } else {
        network_status = "В СЕТИ";
        if ((server_->warnings()[10] & (uint16_t(1) << 0)) != 0) {
            network_status.append(" | ВКЛ N");
        }
    }

    ImGui::InputText("##status",
                     network_status.data(),
                     network_status.size(),
                     ImGuiInputTextFlags_ReadOnly);
    if (!server_->throttle_good()) {
        ImGui::PopStyleColor();
    }
    ImGui::PopItemWidth();

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

    ImGui::SameLine();

    float throttle_pct = server_->throttle();
    ImGui::ProgressBar(throttle_pct / 100.f);

    ImGui::EndChild();
}

void Dashboard::draw_errors() {
    static size_t domain = 0;
    if (std::chrono::steady_clock::now() - error_update_timepoint_ >
        error_update_period_) {
        domain = (domain + 1) % ::shm80::syslog::domains.size();
        error_update_timepoint_ = std::chrono::steady_clock::now();
    }

    uint32_t const errors = server_->errors()[domain];
    uint16_t const warnings = server_->warnings()[domain];
    std::stringstream stream;
    stream << std::setfill('0') << std::setw(2) << std::dec << domain << ":"
           << "0x" << std::setfill('0') << std::setw(8) << std::hex << errors
           << '|' << "0x" << std::setfill('0') << std::setw(4) << std::hex
           << warnings;
    std::string errors_str = stream.str();

    if (errors != 0) {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_red);
    } else if (warnings != 0) {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_yellow);
    }

    ImGui::TextUnformatted(ICON_MDI_ALERT);
    if (errors != 0 || warnings != 0) {
        ImGui::PopStyleColor();
    }

    ImGui::SameLine();

    ImGui::PushItemWidth(-1);
    ImGui::InputText("##errors", errors_str.data(), errors_str.size());
    ImGui::PopItemWidth();
}

} // namespace shm80
} // namespace ui
