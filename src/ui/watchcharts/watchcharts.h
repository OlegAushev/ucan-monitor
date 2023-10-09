#pragma once


#include <imgui.h>
#include <implot.h>
#include <ucanopen/server/server.h>
#include <memory>


namespace ui {


class WatchCharts {
private:
    std::shared_ptr<::ucanopen::Server> _server;
public:
    WatchCharts(std::shared_ptr<::ucanopen::Server> server) : _server(server) {}
    void draw(bool& open);
};


} // namespace ui
