#include "controlpanel.hpp"
#include "imgui.h"
#include <ui/util/togglebutton.h>
#include <ui/util/util.h>

#include <imguifiledialog/ImGuiFileDialog.h>

using namespace psfb;

namespace ui {
namespace psfb {

ControlPanel::ControlPanel(std::shared_ptr<::psfb::Server> server,
                           const std::string& menu_title,
                           const std::string& window_title,
                           bool open)
        : View(menu_title, window_title, open), _server(server) {}

void ControlPanel::_reset_refs() {
    _start = false;
    _ref_overlap1_pct = 0;
    _ref_overlap2_pct = 0;
}

void ControlPanel::_update_refs() {
    if (!_server->tpdo_service.good(ucanopen::CobTpdo::tpdo1) ||
        _server->has_critical() || _server->has_error()) {
        _start = false;
    }

    _server->toggle_emergency(_emergency);
    if (_emergency) {
        _server->exec("ctl", "sys", "emergency");
    }

    _server->toggle_power(_power);
    _server->toggle_start(_start);

    _server->set_overlap1_ref(static_cast<uint16_t>(
            _ref_overlap1_pct * ::psfb::overlap_scale / 100.0f));
    _server->set_overlap2_ref(static_cast<uint16_t>(
            _ref_overlap2_pct * ::psfb::overlap_scale / 100.0f));
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

    // Converter state indicator
    ImGui::SameLine();
    std::string state(_server->converter_state_str());
    ImGui::PushItemWidth(140);
    ImGui::InputText("##state",
                     state.data(),
                     state.size(),
                     ImGuiInputTextFlags_ReadOnly);
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

void ControlPanel::_draw_controls() {
    ImGui::SeparatorText("Управление");

    ImGui::PushItemWidth(140);

    // channel 1 overlap input
    if (ImGui::InputFloat("Перекрытие 1 [%]",
                          &_ref_overlap1_pct,
                          0.1f,
                          10.0f,
                          "%.2f",
                          ImGuiInputTextFlags_EnterReturnsTrue)) {
        _ref_overlap1_pct = std::clamp(_ref_overlap1_pct, 0.0f, 100.0f);
    }

    // channel 2 overlap input
    if (ImGui::InputFloat("Перекрытие 2 [%]",
                          &_ref_overlap2_pct,
                          0.1f,
                          10.0f,
                          "%.2f",
                          ImGuiInputTextFlags_EnterReturnsTrue)) {
        _ref_overlap2_pct = std::clamp(_ref_overlap2_pct, 0.0f, 100.0f);
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

    if (ImGui::Button(ICON_MDI_RESTART " Перезапустить Преобразователь",
                      ImVec2{-1.f, 0.f})) {
        ImGui::OpenPopup("Внимание!##reset_device");
    }

    ImGui::PopStyleColor(3);

    ImGui::PopStyleVar();

    if (ImGui::CollapsingHeader(ICON_MDI_CAR_WRENCH " Доп. Действия",
                                ImGuiTreeNodeFlags_Framed)) {
        ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2{0.f, 0.5f});

        if (ImGui::Button(ICON_MDI_EMOTICON_DEAD
                          " Очистить Память",
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
        ImGui::Text("Преобразователь будет перезапущен. Продолжить?");
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

} // namespace psfb
} // namespace ui
