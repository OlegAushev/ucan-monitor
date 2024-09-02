#include "controlpanel.h"
#include <ui/util/togglebutton.h>
#include <ui/util/util.h>


using namespace loco;


namespace ui {
namespace loco {


ControlPanel::ControlPanel(std::shared_ptr<::loco::Server> server,
                           const std::string& menu_title,
                           const std::string& window_title,
                           bool open)
        : View(menu_title, window_title, open)
        , _server(server)
{}


void ControlPanel::_reset_refs() {
    _start = false;
    _ref_torque_pct = 0;
    _ref_speed = 0;
    _ref_d_angle_deg = 0;
    _ref_d_current_pct = 0;
    _ref_f_current_pct = 0;
}


void ControlPanel::_update_refs() {

}




} // namespace loco
} // namespace ui
