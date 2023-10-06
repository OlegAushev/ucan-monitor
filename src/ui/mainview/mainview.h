#pragma once


#include <imgui.h>
#include "../components/drawable.h"
#include <memory>


namespace ui {


class MainView {
private:
    MainView() = default;

    bool _should_close = false;
    bool _show_options = false;
    bool _show_log = true;

    bool _show_control = false;
    bool _show_data = false;
    bool _show_setup = false;
    bool _show_charts = false;

    bool _show_demo = false;

    std::shared_ptr<ui::DrawableWindow> _data_window;
    std::shared_ptr<ui::DrawableWindow> _ctl_window;
public:
    static MainView& instance() {
        static MainView s;
        return s;
    }

    void draw();
    bool should_close() { return _should_close; }
private:
    void _draw_menubar(); 
};


} // namespace ui
