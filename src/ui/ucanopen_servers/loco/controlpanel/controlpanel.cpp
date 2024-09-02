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










} // namespace loco
} // namespace ui
