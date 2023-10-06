#pragma once


#include <imgui.h>


namespace ui {


class MainView {
private:
    MainView() = default;

    bool _show_options = false;

    //bool _closed = false;
    //bool _should_show_can_bus_setup = false;
public:
    static MainView& instance() {
        static MainView s;
        return s;
    }

    void draw();
    //bool closed() { return _closed; }
private:
    //void _show_menubar(); 
};


} // namespace ui
