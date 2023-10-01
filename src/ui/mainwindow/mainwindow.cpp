#include "mainwindow.h"
#include <GLFW/glfw3.h>


namespace ui {


void MainWindow::show() {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(ImVec2{0, 0});
    ImGui::SetNextWindowSize(viewport->WorkSize);
        
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar;
    ImGui::Begin("uCAN-Monitor", nullptr, window_flags);

    _show_menubar();

    if (_should_show_can_bus_setup) {
        CanBusSetup::instance().show(&_should_show_can_bus_setup);
    }

    ImGui::End();
}


void MainWindow::_show_menubar() {
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Menu")) { 
            ImGui::MenuItem("CAN Bus Setup", nullptr, &_should_show_can_bus_setup);
            ImGui::Separator();
            ImGui::MenuItem("Quit", "Alt+F4", &_closed);
            
            ImGui::EndMenu();
        }
        
        ImGui::EndMenuBar();
    }


    
}


} // namespace ui
