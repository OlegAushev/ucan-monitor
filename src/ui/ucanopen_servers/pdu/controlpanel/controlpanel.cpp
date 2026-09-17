#include "controlpanel.hpp"
#include <ui/util/togglebutton.h>
#include <ui/util/util.h>

namespace ui {
namespace pdu {

ControlPanel::ControlPanel(std::shared_ptr<::pdu::Server> server,
                           const std::string& menu_title,
                           const std::string& window_title,
                           bool open)
        : View(menu_title, window_title, open), _server(server) {}

void ControlPanel::_reset_refs() {
    _inverter_request = false;
    _fuelcell_main = false;
    _fuelcell_precharge = false;
    _inverter_main = false;
    _inverter_precharge = false;
}

void ControlPanel::_update_refs() {
    _mode = static_cast<::pdu::Mode>(_mode_v);

    // Связь с устройством потеряна или ветвь уже под аварией — держать
    // выставленные уровни незачем: сбрасываем их, чтобы возврат связи не
    // подал старую команду.
    if (!_server->tpdo_service.good(ucanopen::CobTpdo::tpdo1) ||
        _server->has_critical() || _server->has_emergency()) {
        _reset_refs();
    }

    // Ручные уровни действуют только в продувке; вне её они не должны
    // копиться на панели.
    if (_mode != ::pdu::Mode::purge) {
        _fuelcell_main = false;
        _fuelcell_precharge = false;
        _inverter_main = false;
        _inverter_precharge = false;
    }

    auto position = [](bool closed) {
        return closed ? ::pdu::ContactorPosition::closed
                      : ::pdu::ContactorPosition::open;
    };

    _server->set_mode(_mode);
    _server->toggle_inverter_request(_inverter_request);
    _server->set_fuelcell_main(position(_fuelcell_main));
    _server->set_fuelcell_precharge(position(_fuelcell_precharge));
    _server->set_inverter_main(position(_inverter_main));
    _server->set_inverter_precharge(position(_inverter_precharge));
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

    // Заявленный режим установки
    ImGui::SameLine();
    std::string mode(_server->mode_str());
    ImGui::PushItemWidth(140);
    ImGui::InputText("##mode",
                     mode.data(),
                     mode.size(),
                     ImGuiInputTextFlags_ReadOnly);
    ImGui::PopItemWidth();

    // emergency
    if (ImGui::Button(ICON_MDI_ALERT_OUTLINE "   АВАРИЯ   " ICON_MDI_ALERT_OUTLINE,
                      ImVec2{200, 0})) {
        _server->exec("ctl", "sys", "emergency");
    }
    ImGui::SameLine();
    ImGui::TextDisabled("(F2)");

    _draw_branch_state("Ветвь ЭХГ",
                       _server->fuelcell_state(),
                       _server->fuelcell_connected(),
                       _server->fuelcell_main_feedback(),
                       _server->fuelcell_precharge_feedback(),
                       _server->fuelcell_voltage(),
                       _server->fuelcell_current());

    _draw_branch_state("Ветвь инвертора",
                       _server->inverter_state(),
                       _server->inverter_connected(),
                       _server->inverter_main_feedback(),
                       _server->inverter_precharge_feedback(),
                       _server->inverter_voltage(),
                       _server->inverter_current());

    ImGui::Text(ICON_MDI_BATTERY " Батарея: %.1f В  " ICON_MDI_THERMOMETER
                                 " МК: %.1f °C",
                _server->battery_voltage(),
                _server->mcu_temperature());
}

void ControlPanel::_draw_branch_state(const char* title,
                                      ::pdu::BranchState state,
                                      bool connected,
                                      ::pdu::ContactorPosition main_feedback,
                                      ::pdu::ContactorPosition
                                              precharge_feedback,
                                      float voltage,
                                      float current) {
    auto icon_color = [](::pdu::ContactorPosition position) {
        return position == ::pdu::ContactorPosition::closed
                       ? ui::colors::icon_green
                       : ui::colors::icon_inactive;
    };

    ImGui::PushStyleColor(ImGuiCol_Text, icon_color(main_feedback));
    ImGui::TextUnformatted(ICON_MDI_ELECTRIC_SWITCH);
    ImGui::PopStyleColor();
    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Text, icon_color(precharge_feedback));
    ImGui::TextUnformatted(ICON_MDI_ELECTRIC_SWITCH);
    ImGui::PopStyleColor();
    ImGui::SameLine();

    if (connected) {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_green);
    } else if (state == ::pdu::BranchState::lockout) {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_red);
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_yellow);
    }
    ImGui::Text("%s: %s",
                title,
                ::pdu::Server::branch_state_str(state).data());
    ImGui::PopStyleColor();
    ImGui::SameLine();
    ImGui::Text("[%.1f В, %.1f А]", voltage, current);
}

void ControlPanel::_read_keyboard() {
    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_F2))) {
        _server->exec("ctl", "sys", "emergency");
    }

    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_F3))) {
        _inverter_request = !_inverter_request;
    }
}

void ControlPanel::_draw_controls() {
    ImGui::SeparatorText("Режим установки");

    ImGui::RadioButton("Ожидание",
                       &_mode_v,
                       std::to_underlying(::pdu::Mode::idle));
    ImGui::RadioButton("Заправка",
                       &_mode_v,
                       std::to_underlying(::pdu::Mode::filling));
    ImGui::RadioButton("Генерация",
                       &_mode_v,
                       std::to_underlying(::pdu::Mode::generation));
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNone)) {
        ImGui::SetTooltip("Ветвь ЭХГ подключается в генерации");
    }
    ImGui::RadioButton("Продувка",
                       &_mode_v,
                       std::to_underlying(::pdu::Mode::purge));
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNone)) {
        ImGui::SetTooltip("Контакторы отдаются ручным уровням");
    }

    // Вместо выведенного из работы датчика PDU читает замещающее напряжение,
    // записанное по SDO, а без обратной связи контакторов считает их положение
    // равным команде. Предзаряд и переключение идут как обычно, а на панели
    // видно именно то, на чём работает PDU.
    if (_server->active(::pdu::status::sensor_bypassed{})) {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_yellow);
        ImGui::TextUnformatted(ICON_MDI_ALERT_OUTLINE
                               " Датчики выведены из работы");
        ImGui::PopStyleColor();
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNone)) {
            ImGui::SetTooltip("Напряжения выведенных датчиков заданы вручную;\n"
                              "без обратной связи контакторов их положение "
                              "следует команде.\nЧто выведено — в настройке, "
                              "категория sensor;\nзамещающие напряжения — в "
                              "разделе «Замещающие Показания».");
        }
    }

    ImGui::SeparatorText("Управление");

    ToggleButton(ICON_MDI_TRANSMISSION_TOWER " ЗАПРОС ИНВЕРТОРА",
                 _inverter_request,
                 ImVec2{260, 0});
    ImGui::SameLine();
    ImGui::TextDisabled("(F3)");
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNone)) {
        ImGui::SetTooltip("Ветвь инвертора живёт не по режиму, а по запросу");
    }

    if (ImGui::CollapsingHeader(ICON_MDI_ELECTRIC_SWITCH
                                " Ручное Управление Контакторами",
                                ImGuiTreeNodeFlags_Framed)) {
        if (_mode != ::pdu::Mode::purge) {
            ImGui::TextDisabled(
                    "Доступно только в режиме продувки");
        }

        util::Switchable manual_contactors(
                _mode == ::pdu::Mode::purge, [this]() {
                    ToggleButton("KM1 " ICON_MDI_GAS_CYLINDER
                                 " Главный ЭХГ",
                                 _fuelcell_main,
                                 ImVec2{260, 0});
                    ToggleButton("KM2 " ICON_MDI_GAS_CYLINDER
                                 " Предзаряд ЭХГ",
                                 _fuelcell_precharge,
                                 ImVec2{260, 0});
                    ToggleButton("KM3 " ICON_MDI_TRANSMISSION_TOWER
                                 " Главный инвертора",
                                 _inverter_main,
                                 ImVec2{260, 0});
                    ToggleButton("KM4 " ICON_MDI_TRANSMISSION_TOWER
                                 " Предзаряд инвертора",
                                 _inverter_precharge,
                                 ImVec2{260, 0});
                });
    }
}

void ControlPanel::_draw_substitutes() {
    if (!ImGui::CollapsingHeader(ICON_MDI_BUG_OUTLINE " Замещающие Показания",
                                 ImGuiTreeNodeFlags_Framed)) {
        return;
    }

    // Напряжения выведенных из работы датчиков PDU берёт из этих значений, у
    // установленных датчиков они ни на что не влияют. Живут значения в RAM
    // PDU: после его перезапуска там снова нули, поэтому рядом с каждым полем
    // видно, что супервизор получает сейчас.
    bool const bypassed = _server->active(::pdu::status::sensor_bypassed{});
    if (!bypassed) {
        ImGui::TextDisabled("Доступно, когда датчики выведены из работы");
    }

    struct Field {
        char const* label;
        char const* name;
        float reading;
    };

    // Единицы — те же, что у объектов debug/substitute в прошивке. PDU примет
    // любое конечное значение.
    std::array const fields{
            Field{"Батарея [В]", "battery_voltage", _server->battery_voltage()},
            Field{"Ветвь ЭХГ [В]", "fuelcell_voltage",
                  _server->fuelcell_voltage()},
            Field{"Ветвь инвертора [В]", "inverter_voltage",
                  _server->inverter_voltage()},
    };

    util::Switchable substitutes(bypassed, [&]() {
        ImGui::PushItemWidth(160);
        for (auto i = 0uz; i < fields.size(); ++i) {
            auto const& field = fields[i];
            if (ImGui::InputFloat(field.label,
                                  &_substitutes[i],
                                  1.0f,
                                  10.0f,
                                  "%.1f",
                                  ImGuiInputTextFlags_EnterReturnsTrue)) {
                _server->write("debug",
                               "substitute",
                               field.name,
                               ucanopen::ExpeditedSdoData{_substitutes[i]});
            }
            ImGui::SameLine();
            ImGui::TextDisabled("сейчас %.1f", field.reading);
        }
        ImGui::PopItemWidth();
    });
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

    if (ImGui::Button(ICON_MDI_RESTART " Перезапустить PDU",
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
        ImGui::Text("PDU будет перезапущен. Продолжить?");
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
        ImGui::Text("Параметры будут сохранены, PDU перезапустится. "
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

} // namespace pdu
} // namespace ui
