#pragma once


#include <imgui.h>
#include <cansocket/cansocket.h>
#include <ucanopen/client/client.h>


namespace ui {


class CanBusSetup {
private:
    CanBusSetup() = default;
    std::shared_ptr<can::Socket> _socket;
    std::shared_ptr<ucanopen::Client> _client;

    int _selected_interface_idx = 0;
    int _selected_bitrate_idx = 0;
    can::Error _error = can::Error::none;
public:
    static CanBusSetup& instance() {
        static CanBusSetup s;
        return s;
    }

    void init(std::shared_ptr<can::Socket> socket, std::shared_ptr<ucanopen::Client> ucanopen_client);

    void show(bool* p_open);
private:
    void _show_socketcan_tab();
    void _show_ucanopen_tab();
    void _show_server_settings(const std::string& server);
};


} // namespace ui
