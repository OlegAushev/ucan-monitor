#pragma once


#include <imgui.h>
#include "style.h"
#include <tuple>


namespace ui {


struct ToggleButtonColors {
    ImVec4 regular;
    ImVec4 hovered;
    ImVec4 active;
};


namespace impl {
inline void SetButtonColors(const ToggleButtonColors& colors) {
    ImGui::PushStyleColor(ImGuiCol_Button, colors.regular);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, colors.hovered);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, colors.active);
}
}


inline bool ToggleButton(const char* label, bool& toggle,
                         const ImVec2& size = ImVec2(0, 0),
                         const ToggleButtonColors& colors = ToggleButtonColors{ui::colors::green_toggle_button,
                                                                               ui::colors::green_toggle_button_hovered,
                                                                               ui::colors::green_toggle_button_active}) {
    bool pressed = false;
    const auto active = toggle;
    if(active) {
        impl::SetButtonColors(colors);
    }

    if(ImGui::Button(label, size)) {
        toggle = !toggle;
        pressed = true;
    }
    
    if(active) {
        ImGui::PopStyleColor(3);
    }

    return pressed;
}


inline bool ToggleSmallButton(const char* label, bool& toggle,
                              const ToggleButtonColors& colors = ToggleButtonColors{ui::colors::green_toggle_button,
                                                                                    ui::colors::green_toggle_button_hovered,
                                                                                    ui::colors::green_toggle_button_active}) {
    bool pressed = false;
    const auto active = toggle;
    if(active) {
        impl::SetButtonColors(colors);
    }

    if(ImGui::SmallButton(label)) {
        toggle = !toggle;
        pressed = true;
    }
    
    if(active) {
        ImGui::PopStyleColor(3);
    }

    return pressed;
}


} // namespace ui
