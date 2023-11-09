#include "watchplot.h"
#include <icons/IconsMaterialDesignIcons.h>
#include <csv_writer/csv_writer.h>


namespace ui {


void WatchPlot::draw() {
    ImGui::Begin(_header_id.c_str(), nullptr, ImGuiWindowFlags_MenuBar);

    _draw_menubar();
    //_draw_panel();
    ImGui::SameLine();

    switch (_mode) {
    case std::to_underlying(Mode::y_t):
        _draw_plot_yt();
        break;
    case std::to_underlying(Mode::y_x):
        _draw_plot_yx();
        break;
    default:
        break;
    }

    ImGui::End();
}


void WatchPlot::_reset() {
    for (auto& chart : _charts) {
        chart.on_plot = false;
        chart.y_axis = ImAxis_Y1;
    }
    _p_xchart = nullptr;
    _p_ychart = nullptr;
}


void WatchPlot::_draw_menubar() {
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Options")) {
            if (ImGui::BeginMenu("Mode")) {
                if (ImGui::RadioButton("y(t)", &_mode, std::to_underlying(Mode::y_t))) { _reset(); }
                if (ImGui::RadioButton("y(x)", &_mode, std::to_underlying(Mode::y_x))) { _reset(); }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Depth [s]")) {
                //ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.6f, 0.6f));
                ImGui::PushItemWidth(120);
                if (ImGui::InputFloat("##depth", &_time_depth, 1, 100, "%.1f", ImGuiInputTextFlags_EnterReturnsTrue)) {
                    _time_depth = std::clamp(_time_depth, 0.1f, 600.0f);
                }
                ImGui::PopItemWidth();
                //ImGui::PopStyleVar();
                ImGui::EndMenu();
            }

            if (ImGui::MenuItem("Export to CSV", nullptr, nullptr, !_running)) {
                _export_to_csv();
            }

            if (ImGui::MenuItem("Reset")) {
                _reset();        
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Charts")) {
            for (size_t i = 0; i < _charts.size(); ++i) {
                if (_charts[i].on_plot) {
                    continue;
                }

                ImGui::Selectable(_charts[i].label.c_str(), false, 0, ImVec2(200, 0));
                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
                    ImGui::SetDragDropPayload(_dnd_id.c_str(), &i, sizeof(int));
                    ImGui::TextUnformatted(_charts[i].label.c_str());
                    ImGui::EndDragDropSource();
                }
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(_dnd_id.c_str())) {
                int i = *(int*)payload->Data;
                _charts[i].on_plot = false;
                _charts[i].y_axis = ImAxis_Y1;

            }
            ImGui::EndDragDropTarget();
        }

        if (_running) {
            if (ImGui::MenuItem(ICON_MDI_PAUSE"Pause ")) {
                _log_snapshot = _server->log_service.get_log();
                _running = false;
            }
        } else {
            if (ImGui::MenuItem(ICON_MDI_PLAY"Resume")) {
                _running = true;
            }
        }

        ImGui::EndMenuBar();
    }
}


// void WatchPlot::_draw_panel() {
//     // child window to serve as initial source for our DND items
//     ImGui::BeginChild(_dndleft_id.c_str(), ImVec2(200, -1));

//     if (ImGui::RadioButton("y(t)", &_mode, std::to_underlying(Mode::y_t))) { _reset(); }
//     ImGui::SameLine();
//     if (ImGui::RadioButton("y(x)", &_mode, std::to_underlying(Mode::y_x))) { _reset(); }

//     ImGui::PushItemWidth(50);
//     if (ImGui::InputFloat("Depth [s]", &_time_depth, 0, 0, "%.0f", ImGuiInputTextFlags_EnterReturnsTrue)) {
//         _time_depth = std::clamp(_time_depth, 1.0f, 600.0f);
//     }
//     ImGui::PopItemWidth();

//     if (ImGui::Button("Reset")) {
//         _reset();         
//     }

//     for (size_t i = 0; i < _charts.size(); ++i) {
//         if (_charts[i].on_plot) {
//             continue;
//         }

//         ImGui::Selectable(_charts[i].label.c_str(), false, 0, ImVec2(200, 0));
//         if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
//             ImGui::SetDragDropPayload(_dnd_id.c_str(), &i, sizeof(int));
//             ImGui::TextUnformatted(_charts[i].label.c_str());
//             ImGui::EndDragDropSource();
//         }
//     }

//     ImGui::EndChild();

//     if (ImGui::BeginDragDropTarget()) {
//         if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(_dnd_id.c_str())) {
//             int i = *(int*)payload->Data;
//             _charts[i].on_plot = false;
//             _charts[i].y_axis = ImAxis_Y1;

//         }
//         ImGui::EndDragDropTarget();
//     }
// }


//----------------------------------------------------------------------------------------------------------------------
void WatchPlot::_draw_plot_yt() {
    ImPlotAxisFlags xflags = ImPlotAxisFlags_None;
    ImPlotAxisFlags y1flags = ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_RangeFit;
    ImPlotAxisFlags y2flags = ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_RangeFit;
    ImPlotAxisFlags y3flags = ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_RangeFit;
      
    if (ImPlot::BeginPlot("##", ImVec2(-1, -1))) {
        ImPlot::SetupAxis(ImAxis_X1, nullptr, xflags);
        ImPlot::SetupAxis(ImAxis_Y1, nullptr, y1flags);
        ImPlot::SetupAxis(ImAxis_Y2, nullptr, y2flags | ImPlotAxisFlags_Opposite);
        ImPlot::SetupAxis(ImAxis_Y3, nullptr, y3flags | ImPlotAxisFlags_Opposite);

        if (_running) {
            _now = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - _server->log_service.log_start()).count()/1000000.0f;
        }

        ImPlot::SetupAxisLimits(ImAxis_X1, _now - _time_depth, _now, ImGuiCond_Always);

        // draw lines
        for (size_t i = 0; i < _charts.size(); ++i) {
            const auto& chart = _charts[i];

            // get pointer to log circular buffer
            const ucanopen::ServerLogService::LogBuf* log_buf = nullptr;
            if (_running) {
                log_buf = _server->log_service.get_log(chart.subcategory, chart.name);
            } else {
                auto log_iter = _log_snapshot.find({chart.subcategory, chart.name});
                if (log_iter == _log_snapshot.end()) {
                    log_buf = nullptr;
                } else {
                    log_buf = &log_iter->second;
                }
            }

            if (!log_buf) {
                continue;
            }

            auto& mtx = _server->log_service.log_mtx();
            std::lock_guard<std::mutex> lock(mtx);
            auto size = log_buf->size();

            if (chart.on_plot && size > 0) {
                const float* p_time = &(log_buf->array_two().first->x());
                const float* p_value = &(log_buf->array_two().first->y());
                size_t offset = log_buf->array_one().first - log_buf->array_two().first;

                ImPlot::SetAxis(chart.y_axis);
                //ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle);
                ImPlot::PlotLine(chart.label.c_str(), p_time, p_value, size, 0, offset, sizeof(boost::geometry::model::d2::point_xy<float>));
                // // allow legend item labels to be DND sources
                if (ImPlot::BeginDragDropSourceItem(chart.label.c_str())) {
                    ImGui::SetDragDropPayload(_dnd_id.c_str(), &i, sizeof(int));
                    ImGui::TextUnformatted(chart.label.c_str());
                    ImPlot::EndDragDropSource();
                }
            }
        }

        // allow the main plot area to be a DND target
        if (ImPlot::BeginDragDropTargetPlot()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(_dnd_id.c_str())) {
                int i = *(int*)payload->Data;
                _charts[i].on_plot = true;
                _charts[i].y_axis = ImAxis_Y1;
            }
            ImPlot::EndDragDropTarget();
        }

        // allow each y-axis to be a DND target
        for (int y = ImAxis_Y1; y <= ImAxis_Y3; ++y) {
            if (ImPlot::BeginDragDropTargetAxis(y)) {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(_dnd_id.c_str())) {
                    int i = *(int*)payload->Data;
                    _charts[i].on_plot = true;
                    _charts[i].y_axis = y;
                }
                ImPlot::EndDragDropTarget();
            }
        }

        // allow the legend to be a DND target
        if (ImPlot::BeginDragDropTargetLegend()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(_dnd_id.c_str())) {
                int i = *(int*)payload->Data;
                _charts[i].on_plot = true;
                _charts[i].y_axis = ImAxis_Y1;
            }
            ImPlot::EndDragDropTarget();
        }

        ImPlot::EndPlot();
    }
}


//----------------------------------------------------------------------------------------------------------------------
void WatchPlot::_draw_plot_yx() {
    ImPlotAxisFlags xflags = ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_RangeFit;
    ImPlotAxisFlags yflags = ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_RangeFit;

    if (ImPlot::BeginPlot("##", ImVec2(-1, -1))) {
        ImPlot::SetupAxis(ImAxis_X1, _p_xchart == nullptr ? "[drop here]" : _p_xchart->label.c_str(), xflags);
        ImPlot::SetupAxis(ImAxis_Y1, _p_ychart == nullptr ? "[drop here]" : _p_ychart->label.c_str(), yflags);

        if (_p_xchart != nullptr && _p_ychart != nullptr) {
            // get pointers to log circular buffers
            const ucanopen::ServerLogService::LogBuf* xbuf;
            const ucanopen::ServerLogService::LogBuf* ybuf;

            if (_running) {
                xbuf = _server->log_service.get_log(_p_xchart->subcategory, _p_xchart->name);
                ybuf = _server->log_service.get_log(_p_ychart->subcategory, _p_ychart->name);
            } else{
                auto xbuf_iter = _log_snapshot.find({_p_xchart->subcategory, _p_xchart->name});
                auto ybuf_iter = _log_snapshot.find({_p_ychart->subcategory, _p_ychart->name});
                
                if (xbuf_iter == _log_snapshot.end()) {
                    xbuf = nullptr;
                } else {
                    xbuf = &xbuf_iter->second;
                }

                if (ybuf_iter == _log_snapshot.end()) {
                    ybuf = nullptr;
                } else {
                    ybuf = &ybuf_iter->second;
                }
            }

            if (xbuf && ybuf) {
                auto& mtx = _server->log_service.log_mtx();
                std::lock_guard<std::mutex> lock(mtx);

                auto xsize = xbuf->size();
                auto ysize = ybuf->size();
                
                size_t xoffset = xbuf->array_one().first - xbuf->array_two().first;
                size_t yoffset = ybuf->array_one().first - ybuf->array_two().first;

                if (xsize == ysize && xoffset == yoffset) {
                    const float* p_xvalues = &(xbuf->array_two().first->y());
                    const float* p_yvalues = &(ybuf->array_two().first->y());
                    ImPlot::PlotLine("##xy", p_xvalues, p_yvalues, xsize, 0, xoffset, sizeof(boost::geometry::model::d2::point_xy<float>));
                }
            }
        }

        // allow the x-axis to be a DND target
        if (ImPlot::BeginDragDropTargetAxis(ImAxis_X1)) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(_dnd_id.c_str())) {
                int i = *(int*)payload->Data; _p_xchart = &_charts[i];
            }
            ImPlot::EndDragDropTarget();
        }
        // allow the x-axis to be a DND source
        if (_p_xchart != nullptr && ImPlot::BeginDragDropSourceAxis(ImAxis_X1)) {
            ImGui::SetDragDropPayload(_dnd_id.c_str(), &_p_xchart->idx, sizeof(int));
            ImGui::TextUnformatted(_p_xchart->label.c_str());
            ImPlot::EndDragDropSource();
        }
        // allow the y-axis to be a DND target
        if (ImPlot::BeginDragDropTargetAxis(ImAxis_Y1)) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(_dnd_id.c_str())) {
                int i = *(int*)payload->Data; _p_ychart = &_charts[i];
            }
            ImPlot::EndDragDropTarget();
        }
        // allow the y-axis to be a DND source
        if (_p_ychart != nullptr && ImPlot::BeginDragDropSourceAxis(ImAxis_Y1)) {
            ImGui::SetDragDropPayload(_dnd_id.c_str(), &_p_ychart->idx, sizeof(int));
            ImGui::TextUnformatted(_p_ychart->label.c_str());
            ImPlot::EndDragDropSource();
        }
        // allow the plot area to be a DND target
        if (ImPlot::BeginDragDropTargetPlot()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(_dnd_id.c_str())) {
                int i = *(int*)payload->Data; _p_xchart = _p_ychart = &_charts[i];
            }
        }
        // allow the plot area to be a DND source
        if (ImPlot::BeginDragDropSourcePlot()) {
            ImGui::TextUnformatted("Yes, you can\ndrag this!");
            ImPlot::EndDragDropSource();
        }

        ImPlot::EndPlot();
    }
}


void WatchPlot::_export_to_csv() {
    auto snapshot = _log_snapshot;
    for (auto& buf : snapshot) {
        buf.second.linearize();
    }

    static int counter = 1;
    std::string filename = "data_" + std::to_string(counter++) + ".csv";
    CsvWriter csv(filename);

    for (const auto& buf : snapshot) {
        csv << "time" << std::string(buf.first.first) + "::" + std::string(buf.first.second);
    }
    csv.next_row();
    
    for (size_t row = 0; row < _server->log_service.log_capacity(); ++row) {
        for (const auto& buf : snapshot) {
            if (row < buf.second.size()) {
                csv << buf.second.array_one().first[row].x() << buf.second.array_one().first[row].y();
            } else {
                csv << "" << "";
            }
        }
        csv.next_row();
    }

    bsclog::success("Data exported to {}.", filename);  
}


} // namespace ui
