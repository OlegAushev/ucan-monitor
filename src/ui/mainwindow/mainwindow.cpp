#include "mainwindow.h"


namespace ui {


void MainWindow::show() {
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar;
    ImGui::Begin("uCAN-Monitor", nullptr, window_flags);

    _show_menubar();

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
