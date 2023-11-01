#include "pdmcontrolpanel.h"


namespace ui {
namespace atvvcu {


PdmControlPanel::PdmControlPanel(std::shared_ptr<::atvvcu::Server> server,
                                 const std::string& menu_title,
                                 const std::string& window_title,
                                 bool show_by_default)
        : View(menu_title, window_title, show_by_default)
        , _server(server)
{}


void PdmControlPanel::draw(bool& open) {

}


} // namespace atvvcu
} // namespace ui
