#pragma once


#include <imgui.h>


namespace ui {


class DrawableWindow {
public:
    virtual void draw(bool& open) = 0;
};


} // namespace ui
