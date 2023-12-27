#pragma once


#include <imgui.h>
#include <implot.h>
#include <ui/view/view.h>
#include <ucanopen/server/server.h>
#include <memory>


namespace ui {


class WatchPlot : public View {
private:
    static inline int _plotid_count{0};
    int _plotid;
    std::shared_ptr<::ucanopen::Server> _server;
    float _now{0.0f};
    float _time_depth{60.0f};

    bool _running{true};
    std::map<ucanopen::ServerLogService::LogKey, ucanopen::ServerLogService::LogBuf> _log_snapshot;

    enum class Mode {
        y_t,
        y_x
    };
    int _mode{std::to_underlying(Mode::y_t)};

    std::string _header_id;
    std::string _dndleft_id;
    std::string _dnd_id;
public:
    WatchPlot(std::shared_ptr<::ucanopen::Server> server,
              const std::string& menu_title,
              const std::string& window_title,
              bool show_by_default);
    void draw();
    void draw(bool& open) {}
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
        auto objects = _server->log_service.objects();
        for (const auto& obj : objects) {
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
private:
    void _export_to_csv();
};


} // namespace ui
