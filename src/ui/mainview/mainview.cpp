#include "mainview.h"
#include <ui/components/togglebutton.h>
#include <icons_font_awesome/IconsFontAwesome6.h>


namespace ui {


MainView::MainView(std::shared_ptr<ui::Options> options,
                   std::shared_ptr<ui::Log> log,
                   std::shared_ptr<ui::DataPanelInterface> datapanel,
                   std::shared_ptr<ui::ControlPanelInterface> controlpanel,
                   std::shared_ptr<ui::StatusPanelInterface> statuspanel,
                   std::shared_ptr<ui::ServerSetup> serversetup,
                   std::shared_ptr<ui::WatchCharts> watchcharts)
        : _options(options)
        , _log(log)
        , _datapanel(datapanel)
        , _controlpanel(controlpanel)
        , _statuspanel(statuspanel)
        , _serversetup(serversetup)
        , _watchcharts(watchcharts)
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

    if (_show_options)      { _options->draw(_show_options); }
    if (_show_log)          { _log->draw(_show_log); }
    if (_show_control)      { _controlpanel->draw(_show_control); }
    if (_show_data)         { _datapanel->draw(_show_data); }
    if (_show_status)       { _statuspanel->draw(_show_status); }
    if (_show_setup)        { _serversetup->draw(_show_setup); }
    if (_show_charts)       { _watchcharts->draw(_show_charts); }
    if (_show_demo)         { ImGui::ShowDemoWindow(); ImPlot::ShowDemoWindow(); }
    
    ImGui::End();
}


void MainView::_draw_menubar() {
    if (ImGui::BeginMenuBar()) {

        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV( 0.f, 0.6f, 0.6f) );
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV( 0.f, 0.7f, 0.7f) );
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV( 0.f, 0.8f, 0.8f) );
        if(ImGui::Button(ICON_FA_POWER_OFF)) { _should_close = true; }
        ImGui::PopStyleColor(3);

        ToggleButton(ICON_FA_GEAR                   " Options", _show_options);
        ToggleButton(ICON_FA_MESSAGE                " Log    ", _show_log);
        ToggleButton(ICON_FA_GAMEPAD                " Control", _show_control);
        ToggleButton(ICON_FA_TABLE                  " Data   ", _show_data);
        ToggleButton(ICON_FA_CIRCLE_INFO            " Status ", _show_status);
        ToggleButton(ICON_FA_SCREWDRIVER_WRENCH     " Setup  ", _show_setup);
        ToggleButton(ICON_FA_CHART_LINE             " Charts ", _show_charts);
        ToggleButton(ICON_FA_INFO                   " Example", _show_demo);
        ImGui::EndMenuBar();
    }
}


} // namespace ui
