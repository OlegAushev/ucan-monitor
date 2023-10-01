#pragma once


#include <imgui.h>


namespace ui {


class CanBusSetup {
private:
    CanBusSetup() = default;
public:
    static CanBusSetup& instance() {
        static CanBusSetup s;
        return s;
    }

    void show(bool* p_open);
};


} // namespace ui
