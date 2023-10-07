#include "mainview.h"
#include "../components/togglebutton.h"
#include <icons_font_awesome/IconsFontAwesome6.h>


namespace ui {


MainView::MainView(std::shared_ptr<ui::Options> options, std::shared_ptr<ui::Log> log, std::shared_ptr<ui::DataPanelInterface> datapanel)
        : _options(options)
        , _log(log)
        , _datapanel(datapanel)
{}


void MainView::draw() {
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar
                                  | ImGuiWindowFlags_NoDocking
                                  | ImGuiWindowFlags_NoResize
                                  | ImGuiWindowFlags_NoMove
                                  | ImGuiWindowFlags_NoTitleBar
                                  | ImGuiWindowFlags_NoCollapse
                                  | ImGuiWindowFlags_NoBringToFrontOnFocus
                                  | ImGuiWindowFlags_NoNavFocus;
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("uCAN-Monitor Main View", nullptr, window_flags);
    ImGui::PopStyleVar();

    // Submit the DockSpace
    ImGuiID dockspace_id = ImGui::GetID("DockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

    _draw_menubar();

    if (_show_options) { _options->draw(_show_options); }
    if (_show_log) { _log->draw(_show_log); }
    if (_show_demo) { ImGui::ShowDemoWindow(); }
    if (_show_data) { _datapanel->draw(_show_data); }

    ImGui::End();
}


void MainView::_draw_menubar() {
    if (ImGui::BeginMenuBar()) {

        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV( 0.f, 0.6f, 0.6f) );
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV( 0.f, 0.7f, 0.7f) );
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV( 0.f, 0.8f, 0.8f) );
        if(ImGui::Button(ICON_FA_POWER_OFF)) { _should_close = true; }
        ImGui::PopStyleColor(3);

        ToggleButton(ICON_FA_GEAR           " Options", _show_options);
        ToggleButton(ICON_FA_MESSAGE        " Log    ", _show_log);
        ToggleButton(ICON_FA_GAMEPAD        " Control", _show_control);
        ToggleButton(ICON_FA_TABLE          " Data   ", _show_data);
        ToggleButton(ICON_FA_TTY            " Setup  ", _show_setup);
        ToggleButton(ICON_FA_CHART_AREA     " Charts ", _show_charts);
        ToggleButton(ICON_FA_INFO           " Example", _show_demo);
        ImGui::EndMenuBar();
    }
}


} // namespace ui
