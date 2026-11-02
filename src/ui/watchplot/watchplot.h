#pragma once


#include <imgui.h>
#include <implot.h>
#include <ui/view/view.h>
#include <ucanopen/server/server.h>
#include <algorithm>
#include <deque>
#include <memory>


namespace ui {


class WatchPlot : public View {
private:
    static inline int _plotid_count{0};
    int _plotid;
    std::shared_ptr<::ucanopen::Server> _server;
    float _now{0.0f};
    float _time_depth{60.0f};
    static constexpr float _max_time_depth{10000.0f};

    bool _running{true};
    std::map<ucanopen::ServerLogService::LogKey, ucanopen::ServerLogService::LogBuf, std::less<>> _log_snapshot;

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
              bool open);
    virtual void draw();
    std::shared_ptr<::ucanopen::Server> server() const { return _server; };
private:
    void _draw_menubar();
    //void _draw_panel();
    void _draw_plot_yt();
    void _draw_plot_yx();
    void _reset();
private:
    struct Chart {
        std::string path; // of the object, the key of its history
        std::string label;
        bool on_plot;
        ImAxis y_axis;
        int idx;
    };
    // A deque: a chart added keeps the others where they are, and
    // _p_xchart and _p_ychart point into it.
    std::deque<Chart> _charts;
    unsigned _charts_version{0}; // of the list of objects taken

    int _chart_idx = 0; // for dnd purpose

    // pointers for XY-plot
    Chart* _p_xchart{nullptr};
    Chart* _p_ychart{nullptr};

    // Adds a chart for each object the history has and the charts lack: the
    // elements of arrays come as the watch learns of them.
    void _init_charts() {
        _charts_version = _server->log_service.version();
        auto objects = _server->log_service.objects();
        for (const auto& obj : objects) {
            bool const charted = std::any_of(_charts.begin(), _charts.end(), [&obj](const Chart& c) {
                return c.path == obj.path;
            });
            if (charted) {
                continue;
            }
            Chart chart;
            chart.path = obj.path;
            chart.label = obj.label.ru.empty() ? obj.path : obj.label.ru;
            if (obj.kind == ucanopen::od::Item::Kind::element) {
                chart.label += " " + std::to_string(obj.element);
            }
            const auto& unit = obj.unit.ru.empty() ? obj.unit.en : obj.unit.ru;
            if (!unit.empty()) {
                chart.label += " [" + unit + "]";
            }
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
