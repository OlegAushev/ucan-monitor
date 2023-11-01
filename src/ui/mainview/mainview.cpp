#include "mainview.h"
#include <ui/components/togglebutton.h>
#include <icons/IconsMaterialDesignIcons.h>


namespace ui {


MainView::MainView(std::shared_ptr<ui::Options> options,
                   const std::vector<std::shared_ptr<View>>& views,
                   std::shared_ptr<ui::WatchPlot> watchplot)
        : _options(options)
        , _views(views)
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

    for (auto& view : _views) {
        if (view->show) {
            view->draw(view->show);
        }
    }

    if (_show_options)      { _options->draw(_show_options); }
    if (_show_watchplots)   { for (auto& plot : _watchplots) { plot->draw(); } }
    if (_show_demo)         { ImGui::ShowDemoWindow(); ImPlot::ShowDemoWindow(); }
    
    ImGui::End();
}


void MainView::_draw_menubar() {
    if (ImGui::BeginMenuBar()) {

        if (ImGui::BeginMenu("Menu")) {
            ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, ImVec2( 2, 2 ) );
            if (ImGui::MenuItem(ICON_MDI_COG " Options")) { _show_options = true; }
            ImGui::PopStyleVar();
            ImGui::Separator();
            if (ImGui::MenuItem("Quit", "Alt+F4")) { _should_close = true; }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {
            for (auto& view : _views) {
                ImGui::MenuItem(view->menu_title().c_str(), nullptr, &view->show);
            }
            
            if (ImGui::BeginMenu(ICON_MDI_CHART_LINE" Charts")) {
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
            ImGui::MenuItem(ICON_MDI_HELP" Example", nullptr, &_show_demo);
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }
}


} // namespace ui
