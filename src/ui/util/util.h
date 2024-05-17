#pragma once


#include <imgui.h>
#include <imgui_internal.h>
#include <chrono>
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


inline void BlinkingText(const char* text, std::chrono::milliseconds period, ImU32 active_color, ImU32 inactive_color) {
    static bool active = true;
    static auto timestamp = std::chrono::steady_clock::now();

    if (active) {
        ImGui::PushStyleColor(ImGuiCol_Text, active_color);
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, inactive_color);
    }
    ImGui::TextUnformatted(text);
    ImGui::PopStyleColor();        

    if (std::chrono::steady_clock::now() > timestamp + period) {
        active = !active;
        timestamp = std::chrono::steady_clock::now();
    }
}


} // namespace utils
} // namespace ui
