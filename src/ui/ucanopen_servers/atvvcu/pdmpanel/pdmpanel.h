#pragma once


#include <imgui.h>
#include <string_view>
#include <ui/view/view.h>
#include <ucanopen_servers/atvvcu/atvvcu_server.h>
#include <memory>


namespace ui {
namespace atvvcu {


class PdmPanel : public View {
private:
    std::shared_ptr<::atvvcu::Server> _server;
public:
    PdmPanel(std::shared_ptr<::atvvcu::Server> server,
             const std::string& menu_title,
             const std::string& window_title,
             bool open);
    virtual void draw() override;
private:
    void _draw_contactor_states();
private:
    std::array<bool, ::atvvcu::pdm_contactor_count> _contactor_ref_state{};
    std::array<bool, ::atvvcu::pdm_contactor_count> _contactor_state{};
    std::array<std::string_view, ::atvvcu::pdm_contactor_count> _contactor_labels = {
        ICON_MDI_BATTERY_POSITIVE" Battery +",
        ICON_MDI_BATTERY_NEGATIVE" Battery -",
        ICON_MDI_NUMERIC_1_BOX_OUTLINE" Front Bypass",
        ICON_MDI_NUMERIC_2_BOX_OUTLINE" Back Bypass",
        ICON_MDI_NUMERIC_3_BOX_OUTLINE" Aux Bypass",
        ICON_MDI_EV_STATION" Allow Charge",
        ICON_MDI_STAR_THREE_POINTS_OUTLINE" 3ph Charge"
    };
};


} // namespace atvvcu
} // namespace ui
