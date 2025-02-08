#include "mainview.h"
#include <icons/IconsMaterialDesignIcons.h>
#include <ui/util/togglebutton.h>

namespace ui {

MainView::MainView(std::shared_ptr<ui::Options> options,
                   const std::vector<std::shared_ptr<View>>& views,
                   const std::vector<std::shared_ptr<View>>& tools,
                   const std::vector<std::shared_ptr<WatchPlot>>& watchplots)
        : _options(options),
          _views(views),
          _tools(tools),
          _watchplots(watchplots) {}

void MainView::draw() {
    ImGuiWindowFlags window_flags =
            ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking |
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_NoNavFocus;
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
        if (view->opened()) {
            view->draw();
        }
    }

    for (auto& tool : _tools) {
        if (tool->opened()) {
            tool->draw();
        }
    }

    for (auto& watchplot : _watchplots) {
        if (watchplot->opened()) {
            watchplot->draw();
        }
    }

    if (_show_options) {
        _options->draw(_show_options);
    }
    if (_show_demo) {
        ImGui::ShowDemoWindow();
        ImPlot::ShowDemoWindow();
    }

    ImGui::End();
}

void MainView::_draw_menubar() {
    if (ImGui::BeginMenuBar()) {

        if (ImGui::BeginMenu("Меню")) {
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
            if (ImGui::MenuItem(ICON_MDI_COG " Настройка")) {
                _show_options = true;
            }
            ImGui::PopStyleVar();
            ImGui::Separator();
            if (ImGui::MenuItem("Выход", "Alt+F4")) {
                _should_close = true;
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Панели")) {
            for (auto& view : _views) {
                auto opened = view->opened();
                ImGui::MenuItem(view->menu_title().c_str(), nullptr, &opened);
                view->toggle(opened);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Инструменты")) {
            for (auto& tool : _tools) {
                auto opened = tool->opened();
                ImGui::MenuItem(tool->menu_title().c_str(), nullptr, &opened);
                tool->toggle(opened);
            }
            ImGui::MenuItem(ICON_MDI_HELP " Пример", nullptr, &_show_demo);
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Диаграммы")) {
            if (ImGui::BeginMenu(ICON_MDI_CHART_LINE " Набл. Переменные")) {
                for (auto& watchplot : _watchplots) {
                    auto opened = watchplot->opened();
                    ImGui::MenuItem(watchplot->menu_title().c_str(),
                                    nullptr,
                                    &opened);
                    watchplot->toggle(opened);
                }
                // ImGui::Checkbox("Enabled##", &_show_watchplots);
                // ImGui::PushItemWidth(80);
                // if (ImGui::InputInt("Number Of Plots##", &_watchplot_count)) {
                //     _watchplot_count = std::clamp(_watchplot_count, 1, 4);
                //     if (size_t(_watchplot_count) > _watchplots.size()) {
                //         _watchplots.push_back(std::make_shared<WatchPlot>(_watchplots.front()->server()));
                //     } else if (size_t(_watchplot_count) < _watchplots.size()) {
                //         _watchplots.pop_back();
                //     }
                // }
                // ImGui::PopItemWidth();
                ImGui::EndMenu();
            }

            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }
}

} // namespace ui
