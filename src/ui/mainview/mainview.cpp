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
                   std::shared_ptr<ui::WatchPlot> watchplot)
        : _options(options)
        , _log(log)
        , _datapanel(datapanel)
        , _controlpanel(controlpanel)
        , _statuspanel(statuspanel)
        , _serversetup(serversetup)
{
    _watchplots.push_back(watchplot);
}


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
    if (_show_watchplots)   { for (auto& plot : _watchplots) { plot->draw(); } }
    if (_show_demo)         { ImGui::ShowDemoWindow(); ImPlot::ShowDemoWindow(); }
    
    ImGui::End();
}


void MainView::_draw_menubar() {
    if (ImGui::BeginMenuBar()) {

        if (ImGui::BeginMenu("Menu")) {
            ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, ImVec2( 2, 2 ) );
            if (ImGui::MenuItem(ICON_FA_GEAR " Options")) { _show_options = true; }
            ImGui::PopStyleVar();
            ImGui::Separator();
            if (ImGui::MenuItem("Quit", "Alt+F4")) { _should_close = true; }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem(ICON_FA_MESSAGE" Log", nullptr, &_show_log);
            ImGui::MenuItem(ICON_FA_GAMEPAD" Control", nullptr, &_show_control);
            ImGui::MenuItem(ICON_FA_TABLE" Data", nullptr, &_show_data);
            ImGui::MenuItem(ICON_FA_CIRCLE_INFO" Status", nullptr, &_show_status);
            ImGui::MenuItem(ICON_FA_SCREWDRIVER_WRENCH" Setup", nullptr, &_show_setup);
            
            if (ImGui::BeginMenu(ICON_FA_CHART_LINE" Charts")) {
                ImGui::Checkbox("Enabled##", &_show_watchplots);
                ImGui::PushItemWidth(80);
                if (ImGui::InputInt("Number Of Plots##", &_watchplot_count)) {
                    _watchplot_count = std::clamp(_watchplot_count, 1, 4);
                    if (size_t(_watchplot_count) > _watchplots.size()) {
                        _watchplots.push_back(std::make_shared<WatchPlot>(_watchplots.front()->server()));
                    } else if (size_t(_watchplot_count) < _watchplots.size()) {
                        _watchplots.pop_back();
                    }
                }
                ImGui::PopItemWidth();
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Tools")) {
            ImGui::MenuItem(ICON_FA_INFO" Example", nullptr, &_show_demo);
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }
}


} // namespace ui
