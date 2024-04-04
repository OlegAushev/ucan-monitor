#include "style.h"


namespace ui {


void apply_gnome_dark_palette()
{
    ImGuiStyle* style = &ImGui::GetStyle();
    ImVec4* colors = style->Colors;

    colors[ImGuiCol_Text]                   = ImGui::ColorConvertU32ToFloat4(colors::view_fg_dark);
    colors[ImGuiCol_WindowBg]               = ImGui::ColorConvertU32ToFloat4(colors::view_bg_dark);
    colors[ImGuiCol_PopupBg]                = ImGui::ColorConvertU32ToFloat4(colors::dialog_bg_dark);
    colors[ImGuiCol_TitleBg]                = ImGui::ColorConvertU32ToFloat4(colors::headerbar_backdrop_dark);
    colors[ImGuiCol_TitleBgActive]          = ImGui::ColorConvertU32ToFloat4(colors::headerbar_bg_dark);
    colors[ImGuiCol_TitleBgCollapsed]       = ImGui::ColorConvertU32ToFloat4(colors::headerbar_backdrop_dark);
    colors[ImGuiCol_MenuBarBg]              = ImGui::ColorConvertU32ToFloat4(colors::dialog_bg_dark);
}


}