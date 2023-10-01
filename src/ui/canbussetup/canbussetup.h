#pragma once


#include <imgui.h>
#include <cansocket/cansocket.h>


namespace ui {


class CanBusSetup {
private:
    CanBusSetup() = default;
    std::shared_ptr<can::Socket> _socket;

    int _selected_interface_idx = 0;
    int _selected_bitrate_idx = 0;
    can::Error _error = can::Error::none;
public:
    static CanBusSetup& instance() {
        static CanBusSetup s;
        return s;
    }

    void init(std::shared_ptr<can::Socket> socket);

    void show(bool* p_open);
private:
    void _show_socketcan_tab();
};


} // namespace ui
