#pragma once


#include <imgui.h>


namespace ui {


inline void SetButtonHighlightColor()
{
    ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.35f, 0.6f, 0.6f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.35f, 0.8f, 0.8f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.35f, 0.7f, 0.7f));
}


inline void ToggleButton( const char* label, bool& toggle )
{
    const auto active = toggle;
    if(active) SetButtonHighlightColor();
    if(ImGui::Button(label)) toggle = !toggle;
    if(active) ImGui::PopStyleColor(3);
}


} // namespace ui
