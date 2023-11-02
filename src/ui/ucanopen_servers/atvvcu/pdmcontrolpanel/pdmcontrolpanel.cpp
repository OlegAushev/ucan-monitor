#include "pdmcontrolpanel.h"
#include <ui/components/togglebutton.h>


namespace ui {
namespace atvvcu {


PdmControlPanel::PdmControlPanel(std::shared_ptr<::atvvcu::Server> server,
                                 const std::string& menu_title,
                                 const std::string& window_title,
                                 bool show_by_default)
        : View(menu_title, window_title, show_by_default)
        , _server(server)
{}


void PdmControlPanel::draw(bool& open) {
    ImGui::Begin(_window_title.c_str(), &open);
    
    _contactor_states = _server->pdm_contactor_states();

    ToggleButton(ICON_MDI_CAR_BATTERY" Battery       ", _contactor_states[std::to_underlying(::atvvcu::PdmContactor::battery_connect)]);
    ToggleButton(ICON_MDI_NUMERIC_1_BOX_OUTLINE" Motor 1 Bypass", _contactor_states[std::to_underlying(::atvvcu::PdmContactor::motor1_bypass)]);
    ToggleButton(ICON_MDI_NUMERIC_2_BOX_OUTLINE" Motor 2 Bypass", _contactor_states[std::to_underlying(::atvvcu::PdmContactor::motor2_bypass)]);
    ToggleButton(ICON_MDI_NUMERIC_3_BOX_OUTLINE" Motor 3 Bypass", _contactor_states[std::to_underlying(::atvvcu::PdmContactor::motor3_bypass)]);
    ToggleButton(ICON_MDI_NUMERIC_4_BOX_OUTLINE" Motor 4 Bypass", _contactor_states[std::to_underlying(::atvvcu::PdmContactor::motor4_bypass)]);
    ToggleButton(ICON_MDI_STAR_THREE_POINTS_OUTLINE" 3ph Charge    ", _contactor_states[std::to_underlying(::atvvcu::PdmContactor::charge_mode)]);
    ToggleButton(ICON_MDI_EV_STATION" Allow Charge  ", _contactor_states[std::to_underlying(::atvvcu::PdmContactor::charge_allow)]);
    ToggleButton(ICON_MDI_DEVICES" Equip Bypass  ", _contactor_states[std::to_underlying(::atvvcu::PdmContactor::equip_bypass)]);

    _server->set_pdm_contactor_states(_contactor_states);

    ImGui::End();
}


} // namespace atvvcu
} // namespace ui
