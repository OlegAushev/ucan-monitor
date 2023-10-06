#pragma once


#include <imgui.h>
#include "../../../components/drawable.h"


namespace ui {
namespace srmdrive {

class Data : public DrawableWindow {
private:
    Data() = default;
public:
    static Data& instance() {
        static Data s;
        return s;
    }

    virtual void draw(bool& open) override;
};


} // namespace srmdrive
} // namespace ui
