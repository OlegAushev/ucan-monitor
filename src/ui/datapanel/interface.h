#pragma once


#include <imgui.h>


namespace ui {


class DataPanelInterface {
public:
    virtual void draw(bool& open) = 0;
};


}
