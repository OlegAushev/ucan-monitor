#include "mainview.h"
#include "../log/log.h"
#include "../options/options.h"
#include "../components/togglebutton.h"
#include <icons_font_awesome/IconsFontAwesome6.h>


namespace ui {


void MainView::draw() {
    // ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar
    //                               | ImGuiWindowFlags_NoDocking
    //                               | ImGuiWindowFlags_NoCollapse
    //                               | ImGuiWindowFlags_NoBringToFrontOnFocus
    //                               | ImGuiWindowFlags_MenuBar;
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

    if (ImGui::BeginMenuBar()) {
        ToggleButton(ICON_FA_GEAR " Options", _show_options);
        ToggleButton(ICON_FA_MESSAGE " Log    ", _show_log);
        ToggleButton(ICON_FA_INFO " Example", _show_demo);
        ImGui::EndMenuBar();
    }

    if (_show_options) { ui::Options::instance().draw(_show_options); }
    if (_show_log) { ui::Log::instance().draw(); }
    if (_show_demo) { ImGui::ShowDemoWindow();     }

    ImGui::End();
}


} // namespace ui
