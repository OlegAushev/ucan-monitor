#pragma once


#include <imgui.h>
#include <ucanopen/server/server.h>


namespace ui {


class ServerSetup {
private:
    std::shared_ptr<ucanopen::Server> _server;

    std::string _device_name;
    std::string _hardware_version;
    std::string _software_version;
    std::string _device_sn;
public:
    ServerSetup(std::shared_ptr<ucanopen::Server> server) : _server(server) {}
    void draw(bool& open);
private:
    void _draw_about();
    void _draw_setup();
    void _draw_popups();
};


} // namespace ui
