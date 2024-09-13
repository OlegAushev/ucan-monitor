#include "panel.h"
#include <ui/util/util.h>
#include <ui/util/style.h>


using namespace moyka;


namespace ui {
namespace moyka {


Panel::Panel(std::shared_ptr<::moyka::Server> server,
             const std::string& menu_title,
             const std::string& window_title,
             bool open)
        : View(menu_title, window_title, open)
        , _server(server) {}


void Panel::draw() {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    float menubar_height = window->MenuBarHeight;
    ImGui::SetNextWindowPos(ImVec2(0.0f, menubar_height));
    ImGui::SetNextWindowSize(ImGui::GetWindowSize());// GetIO().DisplaySize);
    
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration; 
    ImGui::Begin(_window_title.c_str(), &_opened, flags);
    
    if (_server->tpdo_service.good(ucanopen::CobTpdo::tpdo1)) {
        ui::util::BlinkingText(ICON_MDI_NETWORK, std::chrono::milliseconds{750},
                               ui::colors::icon_green, ui::colors::icon_inactive);
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_red);
        ImGui::TextUnformatted(ICON_MDI_CLOSE_NETWORK);
        ImGui::PopStyleColor();
    }
    
    // Drive state indicator
    ImGui::SameLine();
    std::string state(drive_state_names.at(_server->drive_state()));
    if (_server->errors() != 0) {
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ui::colors::icon_red);
    } else {
        auto color = ImGui::GetStyleColorVec4(ImGuiCol_FrameBg);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, color);
    }

    ImGui::PushItemWidth(200);
    ImGui::InputText("##state", state.data(), state.size(), ImGuiInputTextFlags_ReadOnly);
    ImGui::PopItemWidth();
    ImGui::PopStyleColor();

    ImGui::TextUnformatted(ICON_MDI_TIMER_OUTLINE" Время[сек]:");
    ImGui::SameLine();
    ImGui::TextUnformatted(_server->watch_service.string_value("WATCH", "UPTIME").c_str());
    
    ImGui::TextUnformatted(ICON_MDI_ALERT_OUTLINE" Ошибки:");
    ImGui::SameLine();
    uint32_t errors = _server->watch_service.value("WATCH", "FAULTS").u32();
    ImGui::Text("0x%08X", errors);
    
    ImGui::TextUnformatted(ICON_MDI_CAR_BATTERY" Напряжение[В]:");
    ImGui::SameLine();
    ImGui::TextUnformatted(_server->watch_service.string_value("WATCH", "DC_VOLTAGE").c_str());
    
    ImGui::TextUnformatted(ICON_MDI_THERMOMETER" Темп. мотора 1[oC]:");
    ImGui::SameLine();
    ImGui::TextUnformatted(_server->watch_service.string_value("WATCH", "MOTOR_S_TEMP").c_str());
    
    ImGui::TextUnformatted(ICON_MDI_THERMOMETER" Темп. мотора 2[oC]:");
    ImGui::SameLine();
    ImGui::TextUnformatted(_server->watch_service.string_value("WATCH", "MOTOR_FW_TEMP").c_str());
    
    ImGui::TextUnformatted(ICON_MDI_GAUGE" Скорость[об/мин]:");
    ImGui::SameLine();
    ImGui::TextUnformatted(_server->watch_service.string_value("WATCH", "SPEED_RPM").c_str());
    
    float throttle_pct = _server->throttle() * 200.f - 100.f;
    ImGui::PushItemWidth(100);
    ImGui::SliderFloat(ICON_MDI_SPEEDOMETER"##accl_slider", &throttle_pct, -100.f, 100.0f, "%.0f", ImGuiSliderFlags_NoInput);
    ImGui::PopItemWidth();

    float speed = _server->watch_service.value("WATCH", "SPEED_RPM").f32();

    ImGui::SameLine();
    if (speed < 0) {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_green);
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_red);
    }
    ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED" R"); 
    ImGui::PopStyleColor();

    ImGui::SameLine();
    if (speed == 0) {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_green);
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_red);
    }
    ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED" N"); 
    ImGui::PopStyleColor();
    
    ImGui::SameLine();
    if (speed > 0) {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_green);
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_red);
    }
    ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED" D"); 
    ImGui::PopStyleColor();

    ImGui::End();
}


} // namespace moyka
} // namespace ui
