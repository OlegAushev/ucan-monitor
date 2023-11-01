#include "motorcontrolpanel.h"


namespace ui {
namespace atvvcu {


MotorControlPanel::MotorControlPanel(std::shared_ptr<::atvvcu::Server> server,
                                     const std::string& menu_title,
                                     const std::string& window_title,
                                     bool show_by_default)
        : View(menu_title, window_title, show_by_default)
        , _server(server)
{}


void MotorControlPanel::draw(bool& open) {

}


} // namespace atvvcu
} // namespace ui
