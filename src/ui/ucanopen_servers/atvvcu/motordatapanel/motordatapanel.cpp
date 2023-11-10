#include "motordatapanel.h"


namespace ui {
namespace atvvcu {


MotorDataPanel::MotorDataPanel(std::shared_ptr<::atvvcu::Server> server,
                               const std::string& menu_title,
                               const std::string& window_title,
                               bool show_by_default)
        : View(menu_title, window_title, show_by_default)
        , _server(server)
{}


void MotorDataPanel::draw(bool& open) {
    for (size_t i = 0; i < 4; ++i) {
        ImGui::Begin(_window_titles[i].c_str(), &open);


        ImGui::End();
    }
}


} // namespace atvvcu
} // namespace ui
