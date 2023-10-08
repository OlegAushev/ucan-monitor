#pragma once


#include <imgui.h>


namespace ui {


class StatusPanelInterface {
public:
    virtual void draw(bool& open) = 0;
};


} // namespace ui
