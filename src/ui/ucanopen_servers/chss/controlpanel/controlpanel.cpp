#include "controlpanel.hpp"
#include <ui/util/togglebutton.h>
#include <ui/util/util.h>

namespace ui {
namespace chss {

ControlPanel::ControlPanel(std::shared_ptr<::chss::Server> server,
                           const std::string& menu_title,
                           const std::string& window_title,
                           bool open)
        : View(menu_title, window_title, open), _server(server) {}

void ControlPanel::_reset_refs() {
    _inlet_open = false;
    _outlet_open = false;
}

void ControlPanel::_update_refs() {
    _mode = static_cast<::chss::Mode>(_mode_v);

    // Связь с устройством потеряна или СХКВ уже под аварией — держать
    // выставленные уровни незачем: сбрасываем их, чтобы возврат связи не
    // подал старую команду.
    if (!_server->tpdo_service.good(ucanopen::CobTpdo::tpdo1) ||
        _server->has_critical() || _server->has_emergency()) {
        _reset_refs();
    }

    // Ручные уровни действуют только в продувке; вне её они не должны
    // копиться на панели.
    if (_mode != ::chss::Mode::purge) {
        _reset_refs();
    }

    auto position = [](bool open) {
        return open ? ::chss::ValvePosition::open
                    : ::chss::ValvePosition::closed;
    };

    _server->set_mode(_mode);
    _server->set_inlet(position(_inlet_open));
    _server->set_outlet(position(_outlet_open));
}

void ControlPanel::draw() {
    ImGui::Begin(_window_title.c_str(), &_opened);

    _read_keyboard();

    _draw_dash();
    _draw_controls();

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

    // Режим, сообщаемый СХКВ
    ImGui::SameLine();
    std::string mode(::chss::Server::mode_str(_server->mode()));
    ImGui::PushItemWidth(140);
    ImGui::InputText("##mode",
                     mode.data(),
                     mode.size(),
                     ImGuiInputTextFlags_ReadOnly);
    ImGui::PopItemWidth();
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNone)) {
        ImGui::SetTooltip("Режим, сообщаемый СХКВ");
    }

    // emergency
    if (ImGui::Button(ICON_MDI_ALERT_OUTLINE "   АВАРИЯ   " ICON_MDI_ALERT_OUTLINE,
                      ImVec2{200, 0})) {
        _server->exec("ctl", "sys", "emergency");
    }
    ImGui::SameLine();
    ImGui::TextDisabled("(F2)");

    // Стадия внутри режима
    auto stage = _server->stage();
    if (stage == std::to_underlying(::chss::Stage::lockout)) {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_red);
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_yellow);
    }
    ImGui::Text("Стадия: %s (%u)",
                _server->stage_str().data(),
                static_cast<unsigned>(stage));
    ImGui::PopStyleColor();

    _draw_fact(ICON_MDI_PIPE_VALVE " Подача установлена",
               _server->supply_ready());
    ImGui::SameLine();
    _draw_fact(ICON_MDI_GAUGE_FULL " Ресивер полон",
               _server->receiver_full());

    // Заправка: ресивер Е1 через К7
    _draw_valve("К7", _server->inlet_valve());
    ImGui::SameLine();
    ImGui::Text("Р1-1: %.1f атм  Р1: %.1f атм  ИР1: %.1f л/мин",
                _server->receiver_pressure(),
                _server->fill_line_pressure(),
                _server->inflow_rate());

    // Подача: выпуск через К9 на редуктор
    _draw_valve("К9", _server->outlet_valve());
    ImGui::SameLine();
    ImGui::Text("Р2: %.1f атм  Р3: %.1f атм",
                _server->pressure_before_reducer(),
                _server->pressure_after_reducer());

    ImGui::Text(ICON_MDI_THERMOMETER " МК: %.1f °C",
                _server->mcu_temperature());
}

void ControlPanel::_draw_fact(const char* title, bool active) {
    ImGui::PushStyleColor(ImGuiCol_Text,
                          active ? ui::colors::icon_green
                                 : ui::colors::icon_inactive);
    ImGui::TextUnformatted(title);
    ImGui::PopStyleColor();
}

void ControlPanel::_draw_valve(const char* title,
                               ::chss::ValvePosition position) {
    bool const open = position == ::chss::ValvePosition::open;
    ImGui::PushStyleColor(ImGuiCol_Text,
                          open ? ui::colors::icon_green
                               : ui::colors::icon_inactive);
    ImGui::Text("%s %s",
                open ? ICON_MDI_VALVE_OPEN : ICON_MDI_VALVE_CLOSED,
                title);
    ImGui::PopStyleColor();
}

void ControlPanel::_read_keyboard() {
    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_F2))) {
        _server->exec("ctl", "sys", "emergency");
    }
}

void ControlPanel::_draw_controls() {
    ImGui::SeparatorText("Режим СХКВ");

    ImGui::RadioButton("Хранение",
                       &_mode_v,
                       std::to_underlying(::chss::Mode::storage));
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNone)) {
        ImGui::SetTooltip("К7 и К9 закрыты, ресивер под контролем утечки");
    }
    ImGui::RadioButton("Заправка",
                       &_mode_v,
                       std::to_underlying(::chss::Mode::filling));
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNone)) {
        if (_server->receiver_full()) {
            ImGui::SetTooltip("Ресивер полон: СХКВ откажет в заправке");
        } else {
            ImGui::SetTooltip("К7 открыт до заполнения ресивера");
        }
    }
    ImGui::RadioButton("Подача",
                       &_mode_v,
                       std::to_underlying(::chss::Mode::supply));
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNone)) {
        ImGui::SetTooltip("К9 открыт, водород идёт на редуктор");
    }
    ImGui::RadioButton("Продувка",
                       &_mode_v,
                       std::to_underlying(::chss::Mode::purge));
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNone)) {
        ImGui::SetTooltip("Клапаны отдаются ручным уровням");
    }

    if (_server->mode() != _mode) {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_yellow);
        ImGui::Text(ICON_MDI_ALERT_OUTLINE " Запрошен режим «%s», СХКВ в «%s»",
                    ::chss::Server::mode_str(_mode).data(),
                    ::chss::Server::mode_str(_server->mode()).data());
        ImGui::PopStyleColor();
    }

    if (ImGui::CollapsingHeader(ICON_MDI_PIPE_VALVE
                                " Ручное Управление Клапанами",
                                ImGuiTreeNodeFlags_Framed)) {
        if (_mode != ::chss::Mode::purge) {
            ImGui::TextDisabled("Доступно только в режиме продувки");
        }

        util::Switchable manual_valves(
                _mode == ::chss::Mode::purge, [this]() {
                    ToggleButton("К7 " ICON_MDI_GAS_CYLINDER
                                 " Заправка ресивера",
                                 _inlet_open,
                                 ImVec2{260, 0});
                    ToggleButton("К9 " ICON_MDI_PIPE
                                 " Выпуск из ресивера",
                                 _outlet_open,
                                 ImVec2{260, 0});
                });
    }
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

    if (ImGui::Button(ICON_MDI_RESTART " Перезапустить СХКВ",
                      ImVec2{-1.f, 0.f})) {
        ImGui::OpenPopup("Внимание!##reset_device");
    }

    ImGui::PopStyleColor(3);

    ImGui::PopStyleVar();

    if (ImGui::CollapsingHeader(ICON_MDI_CAR_WRENCH " Доп. Действия",
                                ImGuiTreeNodeFlags_Framed)) {
        ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2{0.f, 0.5f});

        if (ImGui::Button(ICON_MDI_CONTENT_SAVE_OUTLINE
                          " Сохранить Параметры",
                          ImVec2{-1.f, 0.f})) {
            ImGui::OpenPopup("Внимание!##save_all_parameters");
        }

        if (ImGui::Button(ICON_MDI_RESTORE
                          " Восстановить Параметры По Умолчанию",
                          ImVec2{-1.f, 0.f})) {
            ImGui::OpenPopup("Внимание!##restore_all_default_parameters");
        }

        if (ImGui::Button(ICON_MDI_EMOTICON_DEAD " Очистить Память",
                          ImVec2{-1.f, 0.f})) {
            ImGui::OpenPopup("Внимание!##erase_all_parameters");
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
        ImGui::Text("СХКВ будет перезапущена. Продолжить?");
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

    if (ImGui::BeginPopupModal("Внимание!##save_all_parameters",
                               NULL,
                               ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Параметры будут сохранены, СХКВ перезапустится. "
                    "Продолжить?");
        ImGui::Separator();

        if (ImGui::Button(ICON_MDI_CANCEL " Нет", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button(ICON_MDI_CHECK " Да", ImVec2(120, 0))) {
            _server->exec("ctl", "sys", "save_all_parameters");
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal("Внимание!##restore_all_default_parameters",
                               NULL,
                               ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Все параметры будут заменены значениями по умолчанию. "
                    "Продолжить?");
        ImGui::Separator();

        if (ImGui::Button(ICON_MDI_CANCEL " Нет", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button(ICON_MDI_CHECK " Да", ImVec2(120, 0))) {
            _server->exec("ctl", "sys", "restore_all_default_parameters");
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal("Внимание!##erase_all_parameters",
                               NULL,
                               ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Энергонезависимая память будет очищена. Продолжить?");
        ImGui::Separator();

        if (ImGui::Button(ICON_MDI_CANCEL " Нет", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button(ICON_MDI_CHECK " Да", ImVec2(120, 0))) {
            _server->exec("ctl", "sys", "erase_all_parameters");
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

} // namespace chss
} // namespace ui
