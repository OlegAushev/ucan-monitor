#pragma once


#include <imgui.h>
#include <implot.h>
#include "../log/log.h"
#include "../options/options.h"
#include "../datapanel/interface.h"
#include "../controlpanel/interface.h"
#include "../statuspanel/interface.h"
#include "../serversetup/serversetup.h"
#include "../watchplot/watchplot.h"
#include <memory>
#include <list>


namespace ui {


class MainView {
private:
    bool _should_close{false};
    bool _show_options{false};
    bool _show_log{true};

    bool _show_control{true};
    bool _show_data{true};
    bool _show_status{false};
    bool _show_setup{false};
    bool _show_watchplots{false};

    bool _show_demo{false};

    std::shared_ptr<ui::Options> _options;
    std::shared_ptr<ui::Log> _log;
    std::shared_ptr<ui::DataPanelInterface> _datapanel;
    std::shared_ptr<ui::ControlPanelInterface> _controlpanel;
    std::shared_ptr<ui::StatusPanelInterface> _statuspanel;
    std::shared_ptr<ui::ServerSetup> _serversetup;
    
    std::list<std::shared_ptr<ui::WatchPlot>> _watchplots;
    int _watchplot_count = 1;
public:
    MainView(std::shared_ptr<ui::Options> options,
             std::shared_ptr<ui::Log> log,
             std::shared_ptr<ui::DataPanelInterface> datapanel,
             std::shared_ptr<ui::ControlPanelInterface> controlpanel,
             std::shared_ptr<ui::StatusPanelInterface> statuspanel,
             std::shared_ptr<ui::ServerSetup> serversetup,
             std::shared_ptr<ui::WatchPlot> watchplot);
    void draw();
    bool should_close() { return _should_close; }
private:
    void _draw_menubar(); 
};


} // namespace ui
