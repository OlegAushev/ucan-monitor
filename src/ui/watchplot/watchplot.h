#pragma once


#include <imgui.h>
#include <implot.h>
#include <ucanopen/server/server.h>
#include <memory>


namespace ui {


class WatchPlot {
private:
    static inline int _plotid_count{0};
    int _plotid;
    std::shared_ptr<::ucanopen::Server> _server;
    float _time_depth{60.0f};

    enum class Mode {
        y_t,
        y_x
    };
    int _mode{std::to_underlying(Mode::y_t)};

    std::string _header_id;
    std::string _dndleft_id;
    std::string _dnd_id;
public:
    WatchPlot(std::shared_ptr<::ucanopen::Server> server) : _server(server) {
        _plotid = ++_plotid_count;
        _init_charts();
        _header_id = std::format("Watch Plot##{}", _plotid);
        _dndleft_id = std::format("dndleft##{}", _plotid);
        _dnd_id = std::format("dnd##{}", _plotid);
    }
    void draw();
    std::shared_ptr<::ucanopen::Server> server() const { return _server; };
private:
    void _draw_menubar();
    //void _draw_panel();
    void _draw_plot_yt();
    void _draw_plot_yx();
    void _reset();
private:
    struct Chart {
        std::string_view subcategory;
        std::string_view name;
        std::string label;
        bool on_plot;
        ImAxis y_axis;
        int idx;
    };
    std::vector<Chart> _charts;

    int _chart_idx = 0; // for dnd purpose

    // pointers for XY-plot
    Chart* _p_xchart{nullptr};
    Chart* _p_ychart{nullptr};

    void _init_charts() {
        for (const auto& obj : _server->watch_service.objects()) {
            Chart chart;
            chart.subcategory = obj->subcategory;
            chart.name = obj->name;
            chart.label = std::string(obj->subcategory) + "::" + std::string(obj->name);    
            chart.on_plot = false;            
            chart.y_axis = ImAxis_Y1;
            chart.idx = _chart_idx++;
            _charts.push_back(chart);
        }
    }
};


} // namespace ui
