#pragma once

#include "../options/options.h"
#include "../view/view.h"
#include "../watchplot/watchplot.h"
#include <imgui.h>
#include <implot.h>
#include <memory>

namespace ui {

class MainView {
public:
    static constexpr ImGuiWindowFlags default_window_flags =
            ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking |
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_NoNavFocus;
private:
    std::shared_ptr<ui::Options> _options;
    std::vector<std::shared_ptr<View>> _views;
    std::vector<std::shared_ptr<View>> _tools;
    std::vector<std::shared_ptr<WatchPlot>> _watchplots;
    int _watchplot_count = 1;

    bool _should_close{false};
    bool _show_options{false};
    bool _show_watchplots{true};
    bool _show_demo{false};

    ImGuiWindowFlags _window_flags;
public:
    MainView(std::shared_ptr<Options> options,
             const std::vector<std::shared_ptr<View>>& views,
             const std::vector<std::shared_ptr<View>>& tools,
             const std::vector<std::shared_ptr<WatchPlot>>& watchplots,
             ImGuiWindowFlags window_flags = default_window_flags);
    void draw();
    bool should_close() { return _should_close; }
private:
    void _draw_menubar();
};

} // namespace ui
