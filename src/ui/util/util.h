#pragma once


#include <imgui.h>
#include <imgui_internal.h>
#include <functional>


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


class Switchable {
private:
    const bool _enabled;
public:
    Switchable(bool enabled, std::function<void()> imgui_widgets) : _enabled(enabled) {
        if (!_enabled) {
            ImGui::BeginDisabled();
        }
        
        imgui_widgets();

        if (!_enabled) {
            ImGui::EndDisabled();
        }
    }
};


} // namespace utils
} // namespace ui
