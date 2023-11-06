#pragma once


#include <imgui.h>
#include <implot.h>
#include <cansocket/cansocket.h>
#include <ucanopen/client/client.h>


namespace ui {


class Options {
private:
    std::shared_ptr<can::Socket> _socket;
    std::shared_ptr<ucanopen::Client> _client;

    size_t _selected_interface_idx{0};
    size_t _selected_bitrate_idx{0};
    can::Error _error{can::Error::none};

    int _client_sync_period{200};
    int _server_watch_period{10};
    int _server_log_capacity{1000};
public:
    Options(std::shared_ptr<can::Socket> socket, std::shared_ptr<ucanopen::Client> ucanopen_client);
    void draw(bool& open);
private:
    void _draw_socketcan_tab();
    void _draw_ucanopen_tab();
    void _draw_appearance_tab();
    void _draw_server_settings(const std::string& server);
};


} // namespace ui
