#include "controlpanel.hpp"
#include "imgui.h"
#include <ui/util/togglebutton.h>
#include <ui/util/util.h>

#include <imguifiledialog/ImGuiFileDialog.h>

using namespace sevpress;

namespace ui {
namespace sevpress {

ControlPanel::ControlPanel(std::shared_ptr<::sevpress::Server> server,
                           const std::string& menu_title,
                           const std::string& window_title,
                           bool open)
        : View(menu_title, window_title, open), _server(server) {}

void ControlPanel::_reset_refs() {
    _start = false;
    _ref_torque_pct = 0;
    _ref_speed = 0;
    _ref_angle = 0;
    _ref_current_pct = 0;
    _ref_voltage_pct = 0;
}

void ControlPanel::_update_refs() {
    if (!_server->tpdo_service.good(ucanopen::CobTpdo::tpdo1) ||
        _server->has_any_error() != 0) {
        _start = false;
    }

    _server->set_opmode(_opmode);

    _server->toggle_emergency(_emergency);
    if (_emergency) {
        _server->exec("ctl", "sys", "emergency");
    }

    _server->toggle_power(_power);
    _server->toggle_start(_start);

    _ctlmode = static_cast<ControlMode>(_ctlmode_v);
    _ctlloop = static_cast<ControlLoop>(_ctlloop_v);
    _server->set_ctlmode(_ctlmode);
    _server->set_ctlloop(_ctlloop);

    _server->set_ref_torque(_ref_torque_pct / 100.0f);
    _server->set_ref_speed(_ref_speed);
    _server->set_ref_angle(_ref_angle);
    _server->set_ref_current(_ref_current_pct / 100.0f);
    _server->set_ref_voltage(_ref_voltage_pct / 100.0f);
}

void ControlPanel::draw() {
    ImGui::Begin(_window_title.c_str(), &_opened);

    _read_keyboard();

    _draw_dash();
    _draw_normal_mode_controls();
    _draw_testing_mode_controls();

    _draw_actions();
    _draw_popups();

    _update_refs();

    ImGui::End();
}

void ControlPanel::_draw_dash() {
    // Uptime
    ImGui::TextUnformatted(ICON_MDI_TIMER_OUTLINE);
    ImGui::SameLine();
    ImGui::PushItemWidth(140);
    auto uptime = _server->watch_service.string_value("sys", "uptime");
    ImGui::InputText("##uptime",
                     uptime.data(),
                     uptime.size(),
                     ImGuiInputTextFlags_ReadOnly);
    ImGui::PopItemWidth();

    // Heartbeat indicator
    if (_server->heartbeat_service.good()) {
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
    std::string state(_server->drive_state_str());
    ImGui::PushItemWidth(140);
    ImGui::InputText("##state",
                     state.data(),
                     state.size(),
                     ImGuiInputTextFlags_ReadOnly);
    ImGui::PopItemWidth();

    // Operation mode selection
    ImGui::PushItemWidth(200);
    auto opmode_preview = opmode_names.at(_opmode).data();
    if (ImGui::BeginCombo("Режим Работы", opmode_preview)) {
        for (const auto& mode : opmode_names) {
            bool is_selected = (mode.first == _opmode);
            if (ImGui::Selectable(mode.second.data(), is_selected)) {
                _opmode = mode.first;
                _reset_refs();
            }
        }
        ImGui::EndCombo();
    }
    ImGui::PopItemWidth();

    // emergency
    ToggleButton(ICON_MDI_ALERT_OUTLINE "   АВАРИЯ   " ICON_MDI_ALERT_OUTLINE,
                 _emergency,
                 ImVec2{200, 0},
                 ui::ToggleButtonColors{ui::colors::red_button,
                                        ui::colors::red_button_hovered,
                                        ui::colors::red_button_active});
    ImGui::SameLine();
    ImGui::TextDisabled("(F2)");

    // wakeup
    ToggleButton(ICON_MDI_POWER_ON " ВХ.ПИТАНИЕ " ICON_MDI_POWER_OFF,
                 _power,
                 ImVec2{200, 0});
    ImGui::SameLine();
    ImGui::TextDisabled("(F3)");

    // start/stop
    ToggleButton(ICON_MDI_PLAY_CIRCLE_OUTLINE
                 " СТАРТ/СТОП " ICON_MDI_STOP_CIRCLE_OUTLINE,
                 _start,
                 ImVec2{200, 0});
    ImGui::SameLine();
    ImGui::TextDisabled("(F4)");
}

void ControlPanel::_read_keyboard() {
    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_F2))) {
        _emergency = !_emergency;
    }

    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_F3))) {
        _power = !_power;
    }

    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_F4))) {
        _start = !_start;
    }
}

void ControlPanel::_draw_normal_mode_controls() {
    bool enabled = _server->opmode() == OperatingMode::normal;
    if (enabled) {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_green);
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_red);
    }
    ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED);
    ImGui::PopStyleColor();

    bool selected = _opmode == OperatingMode::normal;
    ui::util::Switchable run_mode_header(selected, []() {
        ImGui::SameLine();
        ImGui::SeparatorText("Нормальный Режим");
    });

    if (!selected) {
        return;
    }

    // torque input
    ImGui::RadioButton("##torque_ctlmode",
                       &_ctlmode_v,
                       std::to_underlying(ControlMode::torque));
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNone)) {
        ImGui::SetTooltip("Управление моментом");
    }
    ImGui::SameLine();

    util::Switchable torque_input(_ctlmode == ControlMode::torque, [this]() {
        ImGui::PushItemWidth(140);
        if (ImGui::InputFloat("Момент [%]",
                              &_ref_torque_pct,
                              1.0f,
                              100.0f,
                              "%.2f",
                              ImGuiInputTextFlags_EnterReturnsTrue)) {
            _ref_torque_pct = std::clamp(_ref_torque_pct, -100.0f, 100.0f);
        }
        if (_ctlmode != ControlMode::torque) {
            _ref_torque_pct = 0;
        }
        ImGui::PopItemWidth();
    });

    // speed input
    ImGui::RadioButton("##speed_ctlmode",
                       &_ctlmode_v,
                       std::to_underlying(::sevpress::ControlMode::speed));
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNone)) {
        ImGui::SetTooltip("Управление скоростью");
    }
    ImGui::SameLine();

    util::Switchable speed_input(_ctlmode == ControlMode::speed, [this]() {
        ImGui::PushItemWidth(140);
        if (ImGui::InputScalar("Скорость [об/мин]",
                               ImGuiDataType_S16,
                               &_ref_speed,
                               NULL,
                               NULL,
                               NULL,
                               ImGuiInputTextFlags_EnterReturnsTrue)) {
            _ref_speed = std::clamp(_ref_speed, int16_t(-5000), int16_t(5000));
        }
        if (_ctlmode != ControlMode::speed) {
            _ref_speed = 0;
        }
        ImGui::PopItemWidth();
    });

    if (ImGui::CollapsingHeader(ICON_MDI_CAMERA_CONTROL " Режим СУ",
                                ImGuiTreeNodeFlags_Framed)) {
        // control loop
        ImGui::RadioButton("Замкнутый",
                           &_ctlloop_v,
                           std::to_underlying(::sevpress::ControlLoop::closed));
        ImGui::RadioButton("Разомкнутый, ток",
                           &_ctlloop_v,
                           std::to_underlying(::sevpress::ControlLoop::open));
        ImGui::RadioButton(
                "Замкнутый + iD",
                &_ctlloop_v,
                std::to_underlying(::sevpress::ControlLoop::semiclosed));
        ImGui::RadioButton("Разомкнутый, напр.",
                           &_ctlloop_v,
                           std::to_underlying(::sevpress::ControlLoop::openvolt));

        ImGui::PushItemWidth(200);

        switch (_ctlloop) {
        case ControlLoop::closed:
            // do nothing
            break;
        case ControlLoop::open:
            if (ImGui::InputFloat("Ток D [%]",
                                  &_ref_current_pct,
                                  0.1f,
                                  100.0f,
                                  "%.1f",
                                  ImGuiInputTextFlags_EnterReturnsTrue)) {
                _ref_current_pct = std::clamp(_ref_current_pct, 0.0f, 100.0f);
            }
            if (_ref_speed == 0) {
                if (ImGui::InputInt("Угол [°]",
                                    &_ref_angle,
                                    1,
                                    100,
                                    ImGuiInputTextFlags_EnterReturnsTrue)) {
                    // _ref_angle = std::clamp(_ref_angle, 0, 360);
                }
            }
            break;
        case ControlLoop::semiclosed:
            if (ImGui::InputFloat("Ток D [%]",
                                  &_ref_current_pct,
                                  0.1f,
                                  100.0f,
                                  "%.1f",
                                  ImGuiInputTextFlags_EnterReturnsTrue)) {
                _ref_current_pct =
                        std::clamp(_ref_current_pct, -100.0f, 100.0f);
            }
            break;
        case ControlLoop::openvolt:
            if (ImGui::InputFloat("Напряжение [%]",
                                  &_ref_voltage_pct,
                                  0.1f,
                                  100.0f,
                                  "%.1f",
                                  ImGuiInputTextFlags_EnterReturnsTrue)) {
                _ref_voltage_pct = std::clamp(_ref_voltage_pct, 0.0f, 100.0f);
            }
            if (_ref_speed == 0) {
                if (ImGui::InputInt("Угол [°]",
                                    &_ref_angle,
                                    1,
                                    100,
                                    ImGuiInputTextFlags_EnterReturnsTrue)) {
                    // _ref_angle = std::clamp(_ref_angle, 0, 360);
                }
            }
            break;
        }

        ImGui::PopItemWidth();
    }

    if (ImGui::CollapsingHeader(ICON_MDI_ANGLE_ACUTE " Коррекция угла ДПР",
                                ImGuiTreeNodeFlags_Framed)) {
        static float angle{
            _server->watch_service.value("model", "angCorr").f32()};
        static std::chrono::time_point<std::chrono::steady_clock>
                update_timepoint{std::chrono::steady_clock::now()};
        auto now{std::chrono::steady_clock::now()};
        if (now - update_timepoint > std::chrono::milliseconds{1000}) {
            angle = _server->watch_service.value("model", "angCorr").f32();
            update_timepoint = now;
        }

        ImGui::PushItemWidth(-1);
        if (ImGui::SliderFloat("##angle_correction",
                               &angle,
                               -180.0f,
                               180.0f,
                               "%.0f° эл.")) {
            _server->write("ctl",
                           "drive",
                           "set_angle_correction",
                           ucanopen::ExpeditedSdoData{angle});
            update_timepoint = now;
        }
        ImGui::PopItemWidth();
    }

    if (ImGui::CollapsingHeader(ICON_MDI_KNOB " Ограничение vD",
                                ImGuiTreeNodeFlags_Framed)) {
        static float vD_limit{
            _server->watch_service.value("model", "vDLim").f32()};
        static std::chrono::time_point<std::chrono::steady_clock>
                update_timepoint{std::chrono::steady_clock::now()};
        auto now{std::chrono::steady_clock::now()};
        if (now - update_timepoint > std::chrono::milliseconds{1000}) {
            vD_limit = _server->watch_service.value("model", "vDLim").f32();
            update_timepoint = now;
        }

        ImGui::PushItemWidth(-1);
        if (ImGui::SliderFloat("##vD_limit", &vD_limit, 0.0f, 1.0f, "%.3f")) {
            _server->write("ctl",
                           "drive",
                           "set_vD_limit_factor",
                           ucanopen::ExpeditedSdoData{vD_limit});
            update_timepoint = now;
        }
        ImGui::PopItemWidth();
    }
}

void ControlPanel::_draw_testing_mode_controls() {
    bool enabled = _server->opmode() == ::sevpress::OperatingMode::testing;
    if (enabled) {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_green);
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_red);
    }
    ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED);
    ImGui::PopStyleColor();

    bool selected = _opmode == ::sevpress::OperatingMode::testing;
    ui::util::Switchable hwtest_mode_header(selected, []() {
        ImGui::SameLine();
        ImGui::SeparatorText("Режим Тестирования");
    });

    if (!selected) {
        return;
    }

    ImGui::PushItemWidth(140);
    if (ImGui::InputFloat("Коэф. Заполнения [%]",
                          &_ref_torque_pct,
                          1.0f,
                          100.0f,
                          "%.2f",
                          ImGuiInputTextFlags_EnterReturnsTrue)) {
        _ref_torque_pct = std::clamp(_ref_torque_pct, -100.0f, 100.0f);
    }
    ImGui::PopItemWidth();
}

void ControlPanel::_draw_actions() {
    ImGui::SeparatorText("");

    ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2{0.f, 0.5f});

    ImGui::PushStyleColor(ImGuiCol_Button, ui::colors::yellow_button);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                          ui::colors::yellow_button_hovered);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                          ui::colors::yellow_button_active);

    if (ImGui::Button(ICON_MDI_BROOM " Сбросить Ошибки", ImVec2{-1.f, 0.f})) {
        _server->exec("ctl", "sys", "clear_errors");
    }

    ImGui::PopStyleColor(3);

    ImGui::PushStyleColor(ImGuiCol_Button, ui::colors::red_button);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                          ui::colors::red_button_hovered);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ui::colors::red_button_active);

    if (ImGui::Button(ICON_MDI_RESTART " Перезапустить Инвертор",
                      ImVec2{-1.f, 0.f})) {
        ImGui::OpenPopup("Внимание!##reset_device");
    }

    ImGui::PopStyleColor(3);

    ImGui::PopStyleVar();

    if (ImGui::CollapsingHeader(ICON_MDI_CAR_WRENCH " Доп. Действия",
                                ImGuiTreeNodeFlags_Framed)) {
        ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2{0.f, 0.5f});

        if (ImGui::Button(ICON_MDI_CHIP " Сбросить Ошибки Драйверов",
                          ImVec2(-1.f, 0.f))) {
            _server->exec("ctl", "drive", "reset_driver_fault");
        }

        if (ImGui::Button(ICON_MDI_COMPASS_OUTLINE " Калибровать ДПР",
                          ImVec2{-1.f, 0.f})) {
            ImGui::OpenPopup("Внимание!##calibrate_resolver");
        }

        if (ImGui::Button(ICON_MDI_CONTENT_SAVE_OUTLINE
                          " Сохранить Результаты Калибровки",
                          ImVec2{-1.f, 0.f})) {
            ImGui::OpenPopup("Внимание!##save_resolver_config");
        }

        ImGui::PopStyleVar();
    }

    if (ImGui::CollapsingHeader(ICON_MDI_BUG " Отладка",
                                ImGuiTreeNodeFlags_Framed)) {
        ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2{0.f, 0.5f});

        if (ImGui::Button(ICON_MDI_NUMERIC_3_BOX_OUTLINE
                          " Отключить 3ф Драйвер",
                          ImVec2{-1.f, 0.f})) {
            _server->exec("ctl", "drive", "disable_phase_driver");
        }

        if (ImGui::Button(ICON_MDI_NUMERIC_1_BOX_OUTLINE
                          " Отключить 1ф Драйвер",
                          ImVec2{-1.f, 0.f})) {
            _server->exec("ctl", "drive", "disable_field_driver");
        }

        ImGui::PopStyleVar();
    }
}

void ControlPanel::_draw_popups() {
    // Always center this window when appearing
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Внимание!##reset_device",
                               NULL,
                               ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Инвертор будет перезапущен. Продолжить?");
        ImGui::Separator();

        if (ImGui::Button(ICON_MDI_CANCEL " Нет", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button(ICON_MDI_CHECK " Да", ImVec2(120, 0))) {
            _server->exec("ctl", "sys", "reset_device");
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal("Внимание!##calibrate_resolver",
                               NULL,
                               ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Запускается процедура калибровки. Продолжить?");
        ImGui::Separator();

        if (ImGui::Button(ICON_MDI_CANCEL " Нет", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button(ICON_MDI_CHECK " Да", ImVec2(120, 0))) {
            _server->exec("ctl", "drive", "calibrate_resolver");
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal("Внимание!##save_resolver_config",
                               NULL,
                               ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Результаты калибровки будут перезаписаны. Продолжить?");
        ImGui::Separator();

        if (ImGui::Button(ICON_MDI_CANCEL " Нет", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button(ICON_MDI_CHECK " Да", ImVec2(120, 0))) {
            _server->exec("ctl", "drive", "save_resolver_config");
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

} // namespace sevpress
} // namespace ui
