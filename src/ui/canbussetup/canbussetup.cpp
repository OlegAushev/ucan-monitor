#include "canbussetup.h"


namespace ui {


void CanBusSetup::show(bool* p_open) {
    if (!ImGui::Begin("CAN Bus Setup", p_open))
    {
        ImGui::End();
        return;
    }

    ImGui::End();
}


} // namespace ui
