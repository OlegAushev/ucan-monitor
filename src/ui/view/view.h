#pragma once


#include <imgui.h>
#include <string>
#include <icons/IconsMaterialDesignIcons.h>


namespace ui {


class View {
protected:
    std::string _menu_title;
    std::string _window_title;
    bool _opened;
public:
    View(const std::string& menu_title, const std::string& window_title, bool open)
            : _menu_title(menu_title)
            , _window_title(window_title)
            , _opened(open)
    {}

    virtual void draw() = 0;

    const std::string& menu_title() const { return _menu_title; }
    const std::string& window_title() const { return _window_title; }
    bool opened() const { return _opened; }
    void toggle(bool open) { _opened = open; }
};


} // namespace ui
