#include "pdmpanel.h"
#include "imgui.h"
#include "ucanopen_servers/atvvcu/atvvcu_def.h"
#include <string_view>
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


void PdmPanel::draw(bool& open) {
    ImGui::Begin(_window_title.c_str(), &open);

    ToggleButton(ICON_MDI_CAR_BATTERY" Battery       ", _contactor_refstates[std::to_underlying(::atvvcu::PdmContactor::battery_connect)]);
    ToggleButton(ICON_MDI_NUMERIC_1_BOX_OUTLINE" Motor 1 Bypass", _contactor_refstates[std::to_underlying(::atvvcu::PdmContactor::motor1_bypass)]);
    ToggleButton(ICON_MDI_NUMERIC_2_BOX_OUTLINE" Motor 2 Bypass", _contactor_refstates[std::to_underlying(::atvvcu::PdmContactor::motor2_bypass)]);
    ToggleButton(ICON_MDI_NUMERIC_3_BOX_OUTLINE" Motor 3 Bypass", _contactor_refstates[std::to_underlying(::atvvcu::PdmContactor::motor3_bypass)]);
    ToggleButton(ICON_MDI_NUMERIC_4_BOX_OUTLINE" Motor 4 Bypass", _contactor_refstates[std::to_underlying(::atvvcu::PdmContactor::motor4_bypass)]);
    ToggleButton(ICON_MDI_STAR_THREE_POINTS_OUTLINE" 3ph Charge    ", _contactor_refstates[std::to_underlying(::atvvcu::PdmContactor::charge_mode)]);
    ToggleButton(ICON_MDI_EV_STATION" Allow Charge  ", _contactor_refstates[std::to_underlying(::atvvcu::PdmContactor::charge_allow)]);
    ToggleButton(ICON_MDI_DEVICES" Equip Bypass  ", _contactor_refstates[std::to_underlying(::atvvcu::PdmContactor::equip_bypass)]);

    _server->set_pdm_contactor_states(_contactor_refstates);

    ImGui::NewLine();
    _draw_contactor_states();

    ImGui::End();
}


void PdmPanel::_draw_contactor_states() {
    _contactor_states = _server->pdm_contactor_states();

    const std::array<std::string_view, ::atvvcu::pdm_contactor_count> contactor_names = {
        "Battery", "Motor 1 Bypass", "Motor 2 Bypass", "Motor 3 Bypass", "Motor 4 Bypass",
        "3ph Charge", "Allow Charge", "Equip Bypass"  
    };

    static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
    if (ImGui::BeginTable("contactor_table", 2, flags)) {
        ImGui::TableSetupColumn("Contactor");
        ImGui::TableSetupColumn("State");
        ImGui::TableHeadersRow();

        for (size_t row = 0; row < ::atvvcu::pdm_contactor_count; ++row) {
            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted(contactor_names[row].data());

            ImGui::TableSetColumnIndex(1);
            if (_contactor_states[row]) {
                ImGui::TextUnformatted("closed");
                ImU32 cell_bg_color = ImGui::GetColorU32(ImVec4(0.3f, 0.7f, 0.3f, 0.65f));
                ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, cell_bg_color);
            } else {
                ImGui::TextUnformatted("open");
                ImU32 cell_bg_color = ImGui::GetColorU32(ImVec4(0.7f, 0.3f, 0.3f, 0.65f));
                ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, cell_bg_color);
            }
        }

        ImGui::EndTable();
    }
}


} // namespace atvvcu
} // namespace ui
