#pragma once


#include <imgui.h>
#include <implot.h>
#include "../view/view.h"
#include "../options/options.h"
#include "../watchplot/watchplot.h"
#include <memory>
#include <list>


namespace ui {


class MainView {
private:
    std::shared_ptr<ui::Options> _options;
    std::vector<std::shared_ptr<View>> _views;
    std::list<std::shared_ptr<ui::WatchPlot>> _watchplots;
    int _watchplot_count = 1;

    bool _should_close{false};
    bool _show_options{false};
    bool _show_watchplots{true};
    bool _show_demo{false};
public:
    MainView(std::shared_ptr<ui::Options> options,
             const std::vector<std::shared_ptr<View>>& views,
             std::shared_ptr<ui::WatchPlot> watchplot);
    void draw();
    bool should_close() { return _should_close; }
private:
    void _draw_menubar(); 
};


} // namespace ui
