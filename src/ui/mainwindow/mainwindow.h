#pragma once


#include <imgui.h>


namespace ui {


class MainWindow {
private:
    static inline bool _closed = false;
    static inline bool _should_show_can_bus_setup = false;
public:
    static void show();
    static bool closed() { return _closed; }
private:
    static void _show_menubar(); 
};


} // namespace ui
