#pragma once


#include <imgui.h>
#include <imgui_internal.h>


namespace ui {


bool CheckBoxTristate(const char* label, int& v_tristate)
{
    bool ret;
    if (v_tristate == -1) {
        ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, true);
        bool b = false;
        ret = ImGui::Checkbox(label, &b);
        if (ret) {
            v_tristate = 1;
        }
        ImGui::PopItemFlag();
    } else {
        bool b = (v_tristate != 0);
        ret = ImGui::Checkbox(label, &b);
        if (ret) {
            v_tristate = (int)b;
        }
    }
    return ret;
}


} // namespace ui
