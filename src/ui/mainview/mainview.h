#pragma once


#include <imgui.h>
#include <implot.h>
#include "../tool/tool.h"
#include "../view/view.h"
#include "../options/options.h"
#include "../watchplot/watchplot.h"
#include <memory>


namespace ui {


class MainView {
private:
    std::shared_ptr<ui::Options> _options;
    std::vector<std::shared_ptr<View>> _views;
    std::vector<std::shared_ptr<Tool>> _tools;
    std::vector<std::shared_ptr<WatchPlot>> _watchplots;
    int _watchplot_count = 1;

    bool _should_close{false};
    bool _show_options{false};
    bool _show_watchplots{true};
    bool _show_demo{false};
public:
    MainView(std::shared_ptr<Options> options,
             const std::vector<std::shared_ptr<View>>& views,
             const std::vector<std::shared_ptr<WatchPlot>>& watchplots);
    void draw();
    bool should_close() { return _should_close; }
private:
    void _draw_menubar(); 
};


} // namespace ui
