#pragma once


#include <imgui.h>
#include <implot.h>
#include <ucanopen/server/server.h>
#include <memory>


namespace ui {


class WatchPlot {
private:
    static inline int _plotid_count = 0;
    int _plotid;
    std::shared_ptr<::ucanopen::Server> _server;
    float _time_depth = 60;
public:
    WatchPlot(std::shared_ptr<::ucanopen::Server> server) : _server(server) {
        _plotid = ++_plotid_count;
        _init_charts();
    }
    void draw();
    std::shared_ptr<::ucanopen::Server> server() const { return _server; };
private:
    void _draw_plot();
private:
    struct Chart {
        std::string_view subcategory;
        std::string_view name;
        std::string label;
        bool on_plot;
        ImAxis y_axis;

    };
    std::vector<Chart> _charts;

    void _init_charts() {
        for (const auto& obj : _server->watch_service.objects()) {
            Chart chart;
            chart.subcategory = obj->subcategory;
            chart.name = obj->name;
            chart.label = std::string(obj->subcategory) + "::" + std::string(obj->name);    
            chart.on_plot = false;            
            chart.y_axis = ImAxis_Y1;
            _charts.push_back(chart);
        }
    }
};


} // namespace ui
