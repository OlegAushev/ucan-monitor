#include "pdmpanel.h"
#include <ui/components/togglebutton.h>


namespace ui {
namespace atvvcu {


PdmPanel::PdmPanel(std::shared_ptr<::atvvcu::Server> server,
                   const std::string& menu_title,
                   const std::string& window_title,
                   bool show_by_default)
        : View(menu_title, window_title, show_by_default)
        , _server(server)
{}


void PdmPanel::draw() {
    ImGui::Begin(_window_title.c_str(), &show);

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
    std::copy(_server->pdm_contactor_state.begin(), _server->pdm_contactor_state.end(), _contactor_state.begin());

    static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
    if (ImGui::BeginTable("contactor_table", 2, flags)) {
        ImGui::TableSetupColumn("Contactor");
        ImGui::TableSetupColumn("State", ImGuiTableColumnFlags_WidthFixed, 80.0f);
        ImGui::TableHeadersRow();

        for (size_t row = 0; row < ::atvvcu::pdm_contactor_count; ++row) {
            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            ImGui::PushID(row);
            if (_contactor_ref_state[row]) {
                ui::ToggleSmallButton(ICON_MDI_ELECTRIC_SWITCH_CLOSED"##", _contactor_ref_state[row]);
            } else {
                ui::ToggleSmallButton(ICON_MDI_ELECTRIC_SWITCH"##", _contactor_ref_state[row]);
            }
            ImGui::PopID();
            ImGui::SameLine();
            ImGui::TextUnformatted(_contactor_labels[row].data());

            ImGui::TableSetColumnIndex(1);
            if (_contactor_state[row]) {
                ImGui::TextUnformatted(ICON_MDI_ELECTRIC_SWITCH_CLOSED" On");
                ImU32 cell_bg_color = ImGui::GetColorU32(ImVec4(0.3f, 0.7f, 0.3f, 0.65f));
                ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, cell_bg_color);
            } else {
                ImGui::TextUnformatted(ICON_MDI_ELECTRIC_SWITCH" Off");
                ImU32 cell_bg_color = ImGui::GetColorU32(ImVec4(0.7f, 0.3f, 0.3f, 0.65f));
                ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, cell_bg_color);
            }
        }

        ImGui::EndTable();
    }

    std::copy(_contactor_ref_state.begin(), _contactor_ref_state.end(), _server->pdm_contactor_ref_state.begin());
}


} // namespace atvvcu
} // namespace ui
