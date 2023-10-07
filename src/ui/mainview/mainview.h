#pragma once


#include <imgui.h>
#include "../log/log.h"
#include "../options/options.h"
#include "../datapanel/interface.h"
#include <memory>


namespace ui {


class MainView {
private:
    bool _should_close = false;
    bool _show_options = false;
    bool _show_log = true;

    bool _show_control = false;
    bool _show_data = false;
    bool _show_setup = false;
    bool _show_charts = false;

    bool _show_demo = false;

    std::shared_ptr<ui::Options> _options;
    std::shared_ptr<ui::Log> _log;
    std::shared_ptr<ui::DataPanelInterface> _datapanel;
public:
    MainView(std::shared_ptr<ui::Options> options, std::shared_ptr<ui::Log> log, std::shared_ptr<ui::DataPanelInterface> datapanel);
    void draw();
    bool should_close() { return _should_close; }
private:
    void _draw_menubar(); 
};


} // namespace ui
