#pragma once


#include <imgui.h>
#include "../log/log.h"
#include "../options/options.h"
#include "../datapanel/interface.h"
#include "../controlpanel/interface.h"
#include "../statuspanel/interface.h"
#include <memory>


namespace ui {


class MainView {
private:
    bool _should_close = false;
    bool _show_options = false;
    bool _show_log = true;

    bool _show_control = true;
    bool _show_data = true;
    bool _show_status = false;
    bool _show_setup = false;
    bool _show_charts = false;

    bool _show_demo = false;

    std::shared_ptr<ui::Options> _options;
    std::shared_ptr<ui::Log> _log;
    std::shared_ptr<ui::DataPanelInterface> _datapanel;
    std::shared_ptr<ui::ControlPanelInterface> _controlpanel;
    std::shared_ptr<ui::StatusPanelInterface> _statuspanel;
public:
    MainView(std::shared_ptr<ui::Options> options,
             std::shared_ptr<ui::Log> log,
             std::shared_ptr<ui::DataPanelInterface> datapanel,
             std::shared_ptr<ui::ControlPanelInterface> controlpanel,
             std::shared_ptr<ui::StatusPanelInterface> statuspanel);
    void draw();
    bool should_close() { return _should_close; }
private:
    void _draw_menubar(); 
};


} // namespace ui
