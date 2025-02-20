#include "operatorpanel.hpp"
#include "imgui.h"
#include <ui/util/togglebutton.h>
#include <ui/util/util.h>

using namespace shm80;

namespace ui {
namespace shm80 {

OperatorPanel::OperatorPanel(std::shared_ptr<::shm80::Server> server,
                             const std::string& menu_title,
                             const std::string& window_title,
                             bool open)
        : View(menu_title, window_title, open), _server(server) {}

void OperatorPanel::draw() {
    ImGui::Begin(_window_title.c_str(),
                 &_opened,
                 ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoTitleBar);

    ImGui::SetWindowSize({1024, 576});

    {
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;
        ImGui::BeginChild("Преобразователь",
                          ImVec2(500, 560),
                          ImGuiChildFlags_Border,
                          window_flags);
        ImGui::SeparatorText("Преобразователь");

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
            ImGui::TextUnformatted("580");

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted(ICON_MDI_THERMOMETER " Температура A [°C]");
            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted("79");

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted(ICON_MDI_THERMOMETER " Температура B [°C]");
            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted("81");

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted(ICON_MDI_THERMOMETER " Температура C [°C]");
            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted("78");

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted(ICON_MDI_THERMOMETER " Температура F [°C]");
            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted("78");

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

        // Drive state indicator
        ImGui::SameLine();
        ImGui::PushItemWidth(300);
        std::string state = "В СЕТИ | ГОТОВ";
        ImGui::InputText("##state",
                         state.data(),
                         state.size(),
                         ImGuiInputTextFlags_ReadOnly);
        ImGui::PopItemWidth();

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

// void ControlPanel::_draw_dash() {
//     // Uptime
//     ImGui::TextUnformatted(ICON_MDI_TIMER_OUTLINE);
//     ImGui::SameLine();
//     ImGui::PushItemWidth(120);
//     auto uptime = _server->watch_service.string_value("sys", "uptime");
//     ImGui::InputText("##uptime",
//                      uptime.data(),
//                      uptime.size(),
//                      ImGuiInputTextFlags_ReadOnly);
//     ImGui::PopItemWidth();

//     // Heartbeat indicator
//     if (_server->heartbeat_service.good()) {
//         ui::util::BlinkingText(ICON_MDI_NETWORK,
//                                std::chrono::milliseconds{750},
//                                ui::colors::icon_green,
//                                ui::colors::icon_inactive);
//     } else {
//         ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_red);
//         ImGui::TextUnformatted(ICON_MDI_CLOSE_NETWORK);
//         ImGui::PopStyleColor();
//     }

//     // Drive state indicator
//     ImGui::SameLine();
//     std::string state(_server->drive_state_str());
//     ImGui::PushItemWidth(120);
//     ImGui::InputText("##state",
//                      state.data(),
//                      state.size(),
//                      ImGuiInputTextFlags_ReadOnly);
//     ImGui::PopItemWidth();

//     // Operation mode selection
//     ImGui::PushItemWidth(200);
//     auto opmode_preview = opmode_names.at(_opmode).data();
//     if (ImGui::BeginCombo("Operating Mode", opmode_preview)) {
//         for (const auto& mode : opmode_names) {
//             bool is_selected = (mode.first == _opmode);
//             if (ImGui::Selectable(mode.second.data(), is_selected)) {
//                 _opmode = mode.first;
//                 _reset_refs();
//             }
//         }
//         ImGui::EndCombo();
//     }
//     ImGui::PopItemWidth();

//     // emergency
//     ToggleButton(ICON_MDI_ALERT_OUTLINE " Emergency " ICON_MDI_ALERT_OUTLINE,
//                  _emergency,
//                  ImVec2{200, 0});
//     ImGui::SameLine();
//     ImGui::TextDisabled("(F2)");

//     // wakeup
//     ToggleButton(ICON_MDI_POWER_ON " Power On/Off " ICON_MDI_POWER_OFF,
//                  _power,
//                  ImVec2{200, 0});
//     ImGui::SameLine();
//     ImGui::TextDisabled("(F3)");

//     // start/stop
//     ToggleButton(ICON_MDI_PLAY_CIRCLE_OUTLINE
//                  " Start/Stop " ICON_MDI_STOP_CIRCLE_OUTLINE,
//                  _start,
//                  ImVec2{200, 0});
//     ImGui::SameLine();
//     ImGui::TextDisabled("(F4)");
// }

// void ControlPanel::_read_keyboard() {
//     if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_F2))) {
//         _emergency = !_emergency;
//     }

//     if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_F3))) {
//         _power = !_power;
//     }

//     if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_F4))) {
//         _start = !_start;
//     }
// }

// void ControlPanel::_draw_normal_mode_controls() {
//     bool enabled = _server->opmode() == OperatingMode::normal;
//     if (enabled) {
//         ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_green);
//     } else {
//         ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_red);
//     }
//     ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED);
//     ImGui::PopStyleColor();

//     bool selected = _opmode == OperatingMode::normal;
//     ui::util::Switchable run_mode_header(selected, []() {
//         ImGui::SameLine();
//         ImGui::SeparatorText("Normal Mode");
//     });

//     if (!selected) {
//         return;
//     }

//     // torque input
//     ImGui::RadioButton("##torque_ctlmode",
//                        &_ctlmode_v,
//                        std::to_underlying(ControlMode::torque));
//     if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNone)) {
//         ImGui::SetTooltip("Torque mode");
//     }
//     ImGui::SameLine();

//     util::Switchable torque_input(_ctlmode == ControlMode::torque, [this]() {
//         ImGui::PushItemWidth(200);
//         if (ImGui::InputFloat("Torque [%]",
//                               &_ref_torque_pct,
//                               1.0f,
//                               100.0f,
//                               "%.2f",
//                               ImGuiInputTextFlags_EnterReturnsTrue)) {
//             _ref_torque_pct = std::clamp(_ref_torque_pct, -100.0f, 100.0f);
//         }
//         if (_ctlmode != ControlMode::torque) {
//             _ref_torque_pct = 0;
//         }
//         ImGui::PopItemWidth();
//     });

//     // speed input
//     ImGui::RadioButton("##speed_ctlmode",
//                        &_ctlmode_v,
//                        std::to_underlying(::shm80::ControlMode::speed));
//     if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNone)) {
//         ImGui::SetTooltip("Speed mode");
//     }
//     ImGui::SameLine();

//     util::Switchable speed_input(_ctlmode == ControlMode::speed, [this]() {
//         ImGui::PushItemWidth(200);
//         if (ImGui::InputScalar("Speed [rpm]",
//                                ImGuiDataType_S16,
//                                &_ref_speed,
//                                NULL,
//                                NULL,
//                                NULL,
//                                ImGuiInputTextFlags_EnterReturnsTrue)) {
//             _ref_speed = std::clamp(_ref_speed, int16_t(-5000), int16_t(5000));
//         }
//         if (_ctlmode != ControlMode::speed) {
//             _ref_speed = 0;
//         }
//         ImGui::PopItemWidth();
//     });

//     if (ImGui::CollapsingHeader(ICON_MDI_CAMERA_CONTROL " Control Loop",
//                                 ImGuiTreeNodeFlags_Framed)) {
//         // control loop
//         ImGui::RadioButton("Closed Loop",
//                            &_ctlloop_v,
//                            std::to_underlying(::shm80::ControlLoop::closed));
//         ImGui::RadioButton("Open Loop",
//                            &_ctlloop_v,
//                            std::to_underlying(::shm80::ControlLoop::open));
//         ImGui::RadioButton(
//                 "Semi-Closed Loop",
//                 &_ctlloop_v,
//                 std::to_underlying(::shm80::ControlLoop::semiclosed));
//         ImGui::RadioButton("Voltage Loop",
//                            &_ctlloop_v,
//                            std::to_underlying(::shm80::ControlLoop::openvolt));

//         ImGui::PushItemWidth(200);

//         switch (_ctlloop) {
//         case ControlLoop::closed:
//             // do nothing
//             break;
//         case ControlLoop::open:
//             if (ImGui::InputFloat("D-Current [%]",
//                                   &_ref_current_pct,
//                                   0.1f,
//                                   100.0f,
//                                   "%.1f",
//                                   ImGuiInputTextFlags_EnterReturnsTrue)) {
//                 _ref_current_pct = std::clamp(_ref_current_pct, 0.0f, 100.0f);
//             }
//             if (_ref_speed == 0) {
//                 if (ImGui::InputInt("Angle [deg]",
//                                     &_ref_angle,
//                                     1,
//                                     100,
//                                     ImGuiInputTextFlags_EnterReturnsTrue)) {
//                     // _ref_angle = std::clamp(_ref_angle, 0, 360);
//                 }
//             }
//             break;
//         case ControlLoop::semiclosed:
//             if (ImGui::InputFloat("D-Current [%]",
//                                   &_ref_current_pct,
//                                   0.1f,
//                                   100.0f,
//                                   "%.1f",
//                                   ImGuiInputTextFlags_EnterReturnsTrue)) {
//                 _ref_current_pct =
//                         std::clamp(_ref_current_pct, -100.0f, 100.0f);
//             }
//             break;
//         case ControlLoop::openvolt:
//             if (ImGui::InputFloat("Voltage [%]",
//                                   &_ref_voltage_pct,
//                                   0.1f,
//                                   100.0f,
//                                   "%.1f",
//                                   ImGuiInputTextFlags_EnterReturnsTrue)) {
//                 _ref_voltage_pct = std::clamp(_ref_voltage_pct, 0.0f, 100.0f);
//             }
//             if (_ref_speed == 0) {
//                 if (ImGui::InputInt("Angle [deg]",
//                                     &_ref_angle,
//                                     1,
//                                     100,
//                                     ImGuiInputTextFlags_EnterReturnsTrue)) {
//                     // _ref_angle = std::clamp(_ref_angle, 0, 360);
//                 }
//             }
//             break;
//         }

//         ImGui::PopItemWidth();
//     }

//     if (ImGui::CollapsingHeader(ICON_MDI_INDUCTION " Field Control",
//                                 ImGuiTreeNodeFlags_Framed)) {
//         ImGui::Checkbox("Manual", &_manual_field);

//         util::Switchable field_control(_manual_field, [this]() {
//             ImGui::PushItemWidth(200);
//             if (ImGui::InputFloat("Field Current [%]",
//                                   &_ref_field_pct,
//                                   1.0f,
//                                   100.0f,
//                                   "%.1f",
//                                   ImGuiInputTextFlags_EnterReturnsTrue)) {
//                 _ref_field_pct = std::clamp(_ref_field_pct, -100.f, 100.f);
//             }
//             if (!_manual_field) {
//                 _ref_field_pct = 0;
//             }

//             ImGui::PopItemWidth();
//         });
//     }
// }

// void ControlPanel::_draw_testing_mode_controls() {
//     bool enabled = _server->opmode() == ::shm80::OperatingMode::testing;
//     if (enabled) {
//         ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_green);
//     } else {
//         ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_red);
//     }
//     ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED);
//     ImGui::PopStyleColor();

//     bool selected = _opmode == ::shm80::OperatingMode::testing;
//     ui::util::Switchable hwtest_mode_header(selected, []() {
//         ImGui::SameLine();
//         ImGui::SeparatorText("Hardware Test Mode");
//     });
// }

// void ControlPanel::_draw_actions() {
//     ImGui::SeparatorText("");

//     if (ImGui::CollapsingHeader(ICON_MDI_CAR_WRENCH " Service Actions",
//                                 ImGuiTreeNodeFlags_Framed)) {
//         if (ImGui::Button(ICON_MDI_BROOM " Clear Errors", ImVec2{-1.f, 0.f})) {
//             _server->exec("ctl", "sys", "clear_errors");
//         }

//         if (ImGui::Button(ICON_MDI_CHIP " Reset Driver Fault",
//                           ImVec2(-1.f, 0.f))) {
//             _server->exec("ctl", "drive", "reset_driver_fault");
//         }

//         if (ImGui::Button(ICON_MDI_RESTART " Reset MCU", ImVec2{-1.f, 0.f})) {
//             _server->exec("ctl", "sys", "reset_device");
//         }

//         if (ImGui::Button(ICON_MDI_COMPASS_OUTLINE " Calibrate Angle Sensor",
//                           ImVec2{-1.f, 0.f})) {
//             _server->exec("ctl", "drive", "calibrate");
//         }

//         if (ImGui::Button(ICON_MDI_CONTENT_SAVE_OUTLINE
//                           " Save Calibration Results",
//                           ImVec2{-1.f, 0.f})) {
//             _server->exec("ctl", "angsens", "save_calibration");
//         }
//     }
// }

// void ControlPanel::_draw_popups() {
//     // Always center this window when appearing
//     ImVec2 center = ImGui::GetMainViewport()->GetCenter();
//     ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

//     if (ImGui::BeginPopupModal("Warning!##calibration",
//                                NULL,
//                                ImGuiWindowFlags_AlwaysAutoResize)) {
//         ImGui::Text("Calibration procedure is about to begin.");
//         ImGui::Separator();

//         if (ImGui::Button("Cancel", ImVec2(120, 0))) {
//             ImGui::CloseCurrentPopup();
//         }
//         ImGui::SetItemDefaultFocus();
//         ImGui::SameLine();
//         if (ImGui::Button("Ok", ImVec2(120, 0))) {
//             _server->exec("ctl", "drive", "calibrate");
//             ImGui::CloseCurrentPopup();
//         }
//         ImGui::EndPopup();
//     }

//     if (ImGui::BeginPopupModal("Warning!##direction",
//                                NULL,
//                                ImGuiWindowFlags_AlwaysAutoResize)) {
//         ImGui::Text("Default rotation direction will be changed.");
//         ImGui::Separator();

//         if (ImGui::Button("Cancel", ImVec2(120, 0))) {
//             ImGui::CloseCurrentPopup();
//         }
//         ImGui::SetItemDefaultFocus();
//         ImGui::SameLine();
//         if (ImGui::Button("Ok", ImVec2(120, 0))) {
//             _server->exec("ctl", "drive", "invert_rotdir");
//             ImGui::CloseCurrentPopup();
//         }
//         ImGui::EndPopup();
//     }

//     if (ImGui::BeginPopupModal("Warning!##device_reset",
//                                NULL,
//                                ImGuiWindowFlags_AlwaysAutoResize)) {
//         ImGui::Text("Device will be reset.");
//         ImGui::Separator();

//         if (ImGui::Button("Cancel", ImVec2(120, 0))) {
//             ImGui::CloseCurrentPopup();
//         }
//         ImGui::SetItemDefaultFocus();
//         ImGui::SameLine();
//         if (ImGui::Button("Ok", ImVec2(120, 0))) {
//             _server->exec("ctl", "sys", "reset_device");
//             ImGui::CloseCurrentPopup();
//         }
//         ImGui::EndPopup();
//     }
// }

} // namespace shm80
} // namespace ui
