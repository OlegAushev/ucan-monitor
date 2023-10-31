#pragma once


#include <imgui.h>
#include <string>
#include <icons_font_awesome/IconsFontAwesome6.h>


namespace ui {


class View {
protected:
    std::string _menu_title;
    std::string _window_title;
public:
    View(const std::string& menu_title, const std::string& window_title, bool show_by_default)
            : _menu_title(menu_title)
            , _window_title(window_title)
            , show(show_by_default)
    {}

    virtual void draw(bool& open) = 0;

    bool show;
    const std::string& menu_title() const { return _menu_title; }
    const std::string& window_title() const { return _window_title; }
};


} // namespace ui
