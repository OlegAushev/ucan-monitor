#include "controlpanel.hpp"
#include <ui/util/togglebutton.h>
#include <ui/util/util.h>

namespace ui {
namespace cshpp {

ControlPanel::ControlPanel(std::shared_ptr<::cshpp::Server> server,
                           const std::string& menu_title,
                           const std::string& window_title,
                           bool open)
        : View(menu_title, window_title, open), _server(server) {}

void ControlPanel::_reset_refs() {
    _valves_open.fill(false);
    _contactors_closed.fill(false);
}

void ControlPanel::_update_refs() {
    _mode = static_cast<::cshpp::Mode>(_mode_v);

    // Связь с установкой потеряна или она уже под аварией — держать
    // выставленные уровни незачем: сбрасываем их, чтобы возврат связи не
    // подал старую команду.
    if (!_server->tpdo_service.good(ucanopen::CobTpdo::tpdo1) ||
        _server->has_critical() || _server->has_emergency()) {
        _reset_refs();
    }

    // Ручные уровни действуют только в продувке; вне её они не должны
    // копиться на панели.
    if (_mode != ::cshpp::Mode::purge) {
        _reset_refs();
    }

    _server->set_mode(_mode);
    for (auto i = 0uz; i < _valves_open.size(); ++i) {
        _server->set_valve_level(static_cast<::cshpp::Valve>(i),
                                 _valves_open[i]
                                         ? ::cshpp::ValvePosition::open
                                         : ::cshpp::ValvePosition::closed);
    }
    for (auto i = 0uz; i < _contactors_closed.size(); ++i) {
        _server->set_contactor_level(
                static_cast<::cshpp::Contactor>(i),
                _contactors_closed[i] ? ::cshpp::ContactorPosition::closed
                                      : ::cshpp::ContactorPosition::open);
    }
}

void ControlPanel::draw() {
    ImGui::Begin(_window_title.c_str(), &_opened);

    _read_keyboard();

    _draw_dash();
    _draw_controls();
    _draw_substitutes();

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

    // Режим, сообщаемый установкой
    ImGui::SameLine();
    std::string mode(::cshpp::Server::mode_str(_server->mode()));
    ImGui::PushItemWidth(140);
    ImGui::InputText("##mode",
                     mode.data(),
                     mode.size(),
                     ImGuiInputTextFlags_ReadOnly);
    ImGui::PopItemWidth();
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNone)) {
        ImGui::SetTooltip("Режим, сообщаемый БКСГЭУ");
    }

    // emergency
    if (ImGui::Button(ICON_MDI_ALERT_OUTLINE "   АВАРИЯ   " ICON_MDI_ALERT_OUTLINE,
                      ImVec2{200, 0})) {
        _server->exec("ctl", "sys", "emergency");
    }
    ImGui::SameLine();
    ImGui::TextDisabled("(F2)");

    // Этап внутри режима
    ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_yellow);
    ImGui::Text("Этап: %s (%u)",
                _server->stage_str().data(),
                static_cast<unsigned>(_server->stage()));
    ImGui::PopStyleColor();

    // Линия водорода: К1-3 на подаче, К1-4 на входе ЭХГ
    _draw_valve("К1-3", _server->hydrogen_supply_valve());
    ImGui::SameLine();
    _draw_valve("К1-4", _server->fuelcell_inlet_valve());
    ImGui::SameLine();
    ImGui::Text("Р1-1: %.1f атм  ИР1-1: %.1f л/мин",
                _server->hydrogen_pressure(),
                _server->hydrogen_flow());
}

void ControlPanel::_draw_valve(const char* title,
                               ::cshpp::ValvePosition position) {
    bool const open = position == ::cshpp::ValvePosition::open;
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
    ImGui::SeparatorText("Режим БКСГЭУ");

    // Командует установкой кто-то один — монитор или станция оператора: второй
    // источник сбивает счётчик кадров, и установка отбрасывает команды обоих.
    bool commanding = _server->commanding();
    if (ImGui::Checkbox("Командовать БКСГЭУ", &commanding)) {
        _server->set_commanding(commanding);
    }
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNone)) {
        ImGui::SetTooltip("Монитор передаёт команды станции оператора: режим "
                          "(RPDO1)\nи ручные уровни продувки (RPDO2, RPDO3).\n"
                          "Снимите, если на шине станция: командует она.\n"
                          "Без команд БКСГЭУ только отмечает потерю связи "
                          "со станцией.");
    }

    util::Switchable modes(commanding, [this]() {
        ImGui::RadioButton("Ожидание",
                           &_mode_v,
                           std::to_underlying(::cshpp::Mode::idle));
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNone)) {
            ImGui::SetTooltip("Линия закрыта, СХКВ снята с выдачи");
        }
        ImGui::RadioButton("Заправка",
                           &_mode_v,
                           std::to_underlying(::cshpp::Mode::filling));
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNone)) {
            ImGui::SetTooltip("СХКВ заправляется от электролизёра, установка "
                              "ждёт.\nОтклоняется, если ресивер полон.");
        }
        ImGui::RadioButton("Генерация",
                           &_mode_v,
                           std::to_underlying(::cshpp::Mode::generation));
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNone)) {
            ImGui::SetTooltip("Выдача СХКВ, проверка утечки, запуск ЭХГ.\n"
                              "Отклоняется, если Р1-1 ниже порога запуска.");
        }
        ImGui::RadioButton("Продувка",
                           &_mode_v,
                           std::to_underlying(::cshpp::Mode::purge));
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNone)) {
            ImGui::SetTooltip("Клапаны и контакторы PDU отдаются ручным "
                              "уровням");
        }
    });

    // Вместо выведенного из работы датчика установка берёт значение из потока
    // подстановок и работает на нём как на показании: все этапы и проверки
    // остаются в силе, а на панели видно именно оно.
    if (_server->active(::cshpp::status::sensor_bypassed{})) {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_yellow);
        ImGui::TextUnformatted(ICON_MDI_ALERT_OUTLINE
                               " Датчики выведены из работы");
        ImGui::PopStyleColor();
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNone)) {
            ImGui::SetTooltip("Показания выведенных датчиков идут потоком "
                              "подстановок.\nКакие датчики выведены — в "
                              "настройке, категория sensor;\nзамещающие "
                              "значения — в разделе «Замещающие Показания».");
        }
    }

    // Установка меняет режим только по смене команды: остановленная защитой,
    // она не запустится от команды, оставшейся с прошлого раза, а первую
    // команду после сброса лишь запоминает.
    if (commanding && _server->mode() != _mode) {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_yellow);
        ImGui::Text(ICON_MDI_ALERT_OUTLINE
                    " Запрошен режим «%s», БКСГЭУ в «%s»",
                    ::cshpp::Server::mode_str(_mode).data(),
                    ::cshpp::Server::mode_str(_server->mode()).data());
        ImGui::PopStyleColor();
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNone)) {
            ImGui::SetTooltip("БКСГЭУ меняет режим только по смене команды.\n"
                              "Чтобы повторить команду, выберите другой режим "
                              "и вернитесь.");
        }
    }

    if (ImGui::CollapsingHeader(ICON_MDI_PIPE_VALVE
                                " Ручное Управление Продувкой",
                                ImGuiTreeNodeFlags_Framed)) {
        if (!commanding) {
            ImGui::TextDisabled("Команды выключены");
        } else if (_mode != ::cshpp::Mode::purge) {
            ImGui::TextDisabled("Доступно только в режиме продувки");
        }

        // До клапанов СХКВ и контакторов PDU станция дотягивается через
        // установку, но лишь до уровней: режимы узлов установка выбирает сама.
        util::Switchable manual_levels(
                commanding && _mode == ::cshpp::Mode::purge, [this]() {
                    ToggleButton("К1-3 " ICON_MDI_PIPE " Подача водорода",
                                 _valves_open[std::to_underlying(
                                         ::cshpp::Valve::hydrogen_supply)],
                                 ImVec2{260, 0});
                    ToggleButton("К1-4 " ICON_MDI_PIPE " Вход ЭХГ",
                                 _valves_open[std::to_underlying(
                                         ::cshpp::Valve::fuelcell_inlet)],
                                 ImVec2{260, 0});
                    ToggleButton("К7 " ICON_MDI_GAS_CYLINDER
                                 " Заправка ресивера",
                                 _valves_open[std::to_underlying(
                                         ::cshpp::Valve::storage_inlet)],
                                 ImVec2{260, 0});
                    ToggleButton("К9 " ICON_MDI_PIPE " Выпуск из ресивера",
                                 _valves_open[std::to_underlying(
                                         ::cshpp::Valve::storage_outlet)],
                                 ImVec2{260, 0});

                    ToggleButton("KM1 " ICON_MDI_GAS_CYLINDER " Главный ЭХГ",
                                 _contactors_closed[std::to_underlying(
                                         ::cshpp::Contactor::fuelcell_main)],
                                 ImVec2{260, 0});
                    ToggleButton("KM2 " ICON_MDI_GAS_CYLINDER
                                 " Предзаряд ЭХГ",
                                 _contactors_closed[std::to_underlying(
                                         ::cshpp::Contactor::
                                                 fuelcell_precharge)],
                                 ImVec2{260, 0});
                    ToggleButton("KM3 " ICON_MDI_TRANSMISSION_TOWER
                                 " Главный инвертора",
                                 _contactors_closed[std::to_underlying(
                                         ::cshpp::Contactor::inverter_main)],
                                 ImVec2{260, 0});
                    ToggleButton("KM4 " ICON_MDI_TRANSMISSION_TOWER
                                 " Предзаряд инвертора",
                                 _contactors_closed[std::to_underlying(
                                         ::cshpp::Contactor::
                                                 inverter_precharge)],
                                 ImVec2{260, 0});
                });
    }
}

void ControlPanel::_draw_substitutes() {
    if (!ImGui::CollapsingHeader(ICON_MDI_BUG_OUTLINE " Замещающие Показания",
                                 ImGuiTreeNodeFlags_Framed)) {
        return;
    }

    // Подстановки идут потоком, пока он включён. Установка берёт из него
    // показания только выведенных из работы датчиков, у установленных значения
    // ни на что не влияют, поэтому их можно выставить заранее — до перезапуска
    // с новой конфигурацией. Рядом с каждым полем видно, что установка
    // получает сейчас.
    bool streaming = _server->streaming_substitutes();
    if (ImGui::Checkbox("Передавать подстановки", &streaming)) {
        _server->set_streaming_substitutes(streaming);
    }
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNone)) {
        ImGui::SetTooltip("Кадр RPDO4 каждые 100 мс.\n"
                          "Получив первый кадр, БКСГЭУ следит за потоком:\n"
                          "прерванный, он дорастает до нормального останова.");
    }

    if (!_server->active(::cshpp::status::sensor_bypassed{})) {
        ImGui::TextDisabled("Датчики в работе: БКСГЭУ подстановки не берёт");
    }

    struct Field {
        char const* label;
        ::cshpp::Substitute quantity;
        float reading;
    };

    // Единицы — как в телеметрии: давление в атм, расход в л/мин.
    std::array const fields{
            Field{"Р1-1 [атм]", ::cshpp::Substitute::hydrogen_pressure,
                  _server->hydrogen_pressure()},
            Field{"ИР1-1 [л/мин]", ::cshpp::Substitute::hydrogen_flow,
                  _server->hydrogen_flow()},
    };

    ImGui::PushItemWidth(160);
    for (auto const& field : fields) {
        float value = _server->substitute(field.quantity);
        if (ImGui::InputFloat(field.label,
                              &value,
                              0.1f,
                              1.0f,
                              "%.1f",
                              ImGuiInputTextFlags_EnterReturnsTrue)) {
            _server->set_substitute(field.quantity, value);
        }
        ImGui::SameLine();
        ImGui::TextDisabled("сейчас %.1f", field.reading);
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

    if (ImGui::Button(ICON_MDI_RESTART " Перезапустить БКСГЭУ",
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
        ImGui::Text("БКСГЭУ будет перезапущена. Продолжить?");
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
        ImGui::Text("Параметры будут сохранены, БКСГЭУ перезапустится. "
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

} // namespace cshpp
} // namespace ui
