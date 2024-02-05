#include "bmspanel.h"
#include <ui/components/togglebutton.h>


namespace ui {
namespace atvvcu {


BmsPanel::BmsPanel(std::shared_ptr<::atvvcu::Server> server,
                   const std::string& menu_title,
                   const std::string& window_title,
                   bool open)
        : View(menu_title, window_title, open)
        , _server(server)
{}


void BmsPanel::draw() {
    ImGui::Begin(_window_title.c_str(), &is_open);

    ImGui::TextUnformatted("Voltage[V]:");
    ImGui::SameLine();
    auto voltage = _server->bms_voltage.load();
    ImGui::Text("%.1f", voltage);

    ImGui::TextUnformatted("Charge[%]:");
    ImGui::SameLine();
    auto charge = _server->bms_charge_pct.load();
    ImGui::Text("%.0f", charge);

    ImGui::End();
}


} // namespace atvvcu
} // namespace ui
