#include "pdmpanel.h"
#include <ui/util/togglebutton.h>


namespace ui {
namespace atvvcu {


PdmPanel::PdmPanel(std::shared_ptr<::atvvcu::Server> server,
                   const std::string& menu_title,
                   const std::string& window_title,
                   bool open)
        : View(menu_title, window_title, open)
        , _server(server)
{}


void PdmPanel::draw() {
    ImGui::Begin(_window_title.c_str(), &_opened);

    // ToggleButton(ICON_MDI_BATTERY_POSITIVE      " Battery +   ", _contactor_ref_state[std::to_underlying(::atvvcu::PdmContactor::battery_p)]);
    // ToggleButton(ICON_MDI_BATTERY_NEGATIVE      " Battery -   ", _contactor_ref_state[std::to_underlying(::atvvcu::PdmContactor::battery_n)]);
    // ToggleButton(ICON_MDI_NUMERIC_1_BOX_OUTLINE " Front Bypass", _contactor_ref_state[std::to_underlying(::atvvcu::PdmContactor::front_bypass)]);
    // ToggleButton(ICON_MDI_NUMERIC_2_BOX_OUTLINE " Back Bypass ", _contactor_ref_state[std::to_underlying(::atvvcu::PdmContactor::back_bypass)]);
    // ToggleButton(ICON_MDI_NUMERIC_3_BOX_OUTLINE " Aux Bypass  ", _contactor_ref_state[std::to_underlying(::atvvcu::PdmContactor::aux_bypass)]);
    // ToggleButton(ICON_MDI_EV_STATION" Allow Charge  ", _contactor_ref_state[std::to_underlying(::atvvcu::PdmContactor::charge_allow)]);
    // ToggleButton(ICON_MDI_STAR_THREE_POINTS_OUTLINE" 3ph Charge    ", _contactor_ref_state[std::to_underlying(::atvvcu::PdmContactor::charge_mode)]);

    _draw_contactor_states();

    ImGui::End();
}


void PdmPanel::_draw_contactor_states() {
    _contactor_feedback_state = _server->pdm.contactor_feedback_state();

    static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
    if (ImGui::BeginTable("contactor_table", 2, flags)) {
        ImGui::TableSetupColumn("Contactor");
        ImGui::TableSetupColumn("State", ImGuiTableColumnFlags_WidthFixed, 80.0f);
        ImGui::TableHeadersRow();

        for (size_t row = 0; row < ::atvvcu::pdm::contactor_count; ++row) {
            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            if (!_server->pdm.debug_mode()) {ImGui::BeginDisabled(); }
            ImGui::PushID(row);
            if (_contactor_ref_state[row]) {
                ui::ToggleSmallButton(ICON_MDI_ELECTRIC_SWITCH_CLOSED"##", _contactor_ref_state[row]);
            } else {
                ui::ToggleSmallButton(ICON_MDI_ELECTRIC_SWITCH"##", _contactor_ref_state[row]);
            }
            ImGui::PopID();
            if (!_server->pdm.debug_mode()) { ImGui::EndDisabled(); }
            ImGui::SameLine();
            ImGui::TextUnformatted(_contactor_labels[row].data());

            ImGui::TableSetColumnIndex(1);
            if (_contactor_feedback_state[row]) {
                ImGui::TextUnformatted(ICON_MDI_ELECTRIC_SWITCH_CLOSED" ON");
                ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ui::colors::table_bg_green);
            } else {
                ImGui::TextUnformatted(ICON_MDI_ELECTRIC_SWITCH" OFF");
                ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ui::colors::table_bg_red);
            }
        }

        ImGui::EndTable();
    }

    _server->pdm.set_contactor_ref_state(_contactor_ref_state);
}


} // namespace atvvcu
} // namespace ui
