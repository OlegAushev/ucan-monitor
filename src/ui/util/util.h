#pragma once


#include <imgui.h>
#include <imgui_internal.h>


namespace ui {
namespace util {


class DisableGuard {
private:
    const bool _disabled;
public:
    explicit DisableGuard(bool disabled) : _disabled(disabled) {
        if (_disabled) {
            ImGui::BeginDisabled();
        }
    }

    ~DisableGuard() {
        if (_disabled) {
            ImGui::EndDisabled();
        }
    }
};


} // namespace utils
} // namespace ui
