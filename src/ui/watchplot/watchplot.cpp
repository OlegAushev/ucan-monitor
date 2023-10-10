#include "watchplot.h"


namespace ui {


void WatchPlot::draw() {
    ImGui::Begin(_header_id.c_str());

    _draw_panel();
    ImGui::SameLine();

    switch (_mode) {
    case std::to_underlying(Mode::y_t):
        _draw_plot_yt();
        break;
    case std::to_underlying(Mode::y_x):

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
}


void WatchPlot::_draw_panel() {
    // child window to serve as initial source for our DND items
    ImGui::BeginChild(_dndleft_id.c_str(), ImVec2(200, -1));

    if (ImGui::RadioButton("y(t)", &_mode, std::to_underlying(Mode::y_t))) { _reset(); }
    ImGui::SameLine();
    if (ImGui::RadioButton("y(x)", &_mode, std::to_underlying(Mode::y_x))) { _reset(); }

    ImGui::PushItemWidth(50);
    if (ImGui::InputFloat("Depth [s]", &_time_depth, 0, 0, "%.0f", ImGuiInputTextFlags_EnterReturnsTrue)) {
        _time_depth = std::clamp(_time_depth, 1.0f, 600.0f);
    }
    ImGui::PopItemWidth();

    if (ImGui::Button("Reset")) {
        _reset();         
    }

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

    ImGui::EndChild();

    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(_dnd_id.c_str())) {
            int i = *(int*)payload->Data;
            _charts[i].on_plot = false;
            _charts[i].y_axis = ImAxis_Y1;

        }
        ImGui::EndDragDropTarget();
    }
}


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

        float now = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - _server->watch_service.history_start()).count()/1000000.0f;
        ImPlot::SetupAxisLimits(ImAxis_X1, now - _time_depth, now, ImGuiCond_Always);

        // draw lines
        for (size_t i = 0; i < _charts.size(); ++i) {
            const auto& chart = _charts[i];
            const auto* history = _server->watch_service.history(chart.subcategory, chart.name);
            const float* p_time = &(history->array_two().first->x());
            const float* p_value = &(history->array_two().first->y());
            auto size = history->size();
            size_t offset = history->array_one().first - history->array_two().first;

            if (chart.on_plot && size > 0) {
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


} // namespace ui