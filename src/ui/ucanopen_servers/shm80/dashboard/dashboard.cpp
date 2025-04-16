#include "dashboard.hpp"
#include <ui/util/checkbox_tristate.h>
#include <ui/util/style.h>
#include <ui/util/util.h>

using namespace shm80;

namespace ui {
namespace shm80 {

Dashboard::Dashboard(std::shared_ptr<::shm80::Server> server,
                     const std::string& menu_title,
                     const std::string& window_title,
                     bool open)
        : View(menu_title, window_title, open), server_(server) {}

void Dashboard::draw() {
    // ImGuiWindow* window = ImGui::GetCurrentWindow();
    // float menubar_height = window->MenuBarHeight;
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowSize(ImGui::GetWindowSize()); // GetIO().DisplaySize);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration;
    ImGui::Begin(_window_title.c_str(), &_opened, flags);
    ImGui::End();
}

} // namespace shm80
} // namespace ui
