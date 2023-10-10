#include "watchcharts.h"


namespace ui {


void WatchCharts::draw(bool& open) {
    ImGui::Begin("Watch Charts##", &open);

    // static ImPlotAxisFlags xflags = ImPlotAxisFlags_None;
    // static ImPlotAxisFlags yflags = ImPlotAxisFlags_AutoFit|ImPlotAxisFlags_RangeFit;

    // const auto* history = _server->watch_service.history("sys", "uptime");

    // const float* time = &(history->array_two().first->x());
    // const float* value = &(history->array_two().first->y());
    // const auto size = history->size();
    // float now = history->back().x();
    // size_t offset = history->array_one().first - history->array_two().first;
    
    // if (ImPlot::BeginPlot("##Scrolling", ImVec2(-1,150))) {
    //     ImPlot::SetupAxes(nullptr, nullptr, xflags, yflags);
    //     ImPlot::SetupAxisLimits(ImAxis_X1,now - 60, now, ImGuiCond_Always);
    //     ImPlot::SetupAxisLimits(ImAxis_Y1,0,1);
    //     ImPlot::SetNextFillStyle(IMPLOT_AUTO_COL,0.5f);
    //     ImPlot::PlotLine("uptime", time, value, size, 0, offset, sizeof(boost::geometry::model::d2::point_xy<float>));
    //     ImPlot::EndPlot();
    // }
    _draw_plot();

    ImGui::End();
}


void WatchCharts::_draw_plot() {
    // child window to serve as initial source for our DND items
    ImGui::BeginChild("dnd_srea_left", ImVec2(200, -1));
    for (size_t i = 0; i < _charts.size(); ++i) {
        if (_charts[i].on_plot) {
            continue;
        }

        ImGui::Selectable(_charts[i].label.c_str(), false, 0, ImVec2(200, 0));
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
            ImGui::SetDragDropPayload("dnd_area", &i, sizeof(int));
            ImGui::TextUnformatted(_charts[i].label.c_str());
            ImGui::EndDragDropSource();
        }
    }
    ImGui::EndChild();

    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("dnd_area")) {
            int i = *(int*)payload->Data;
            _charts[i].on_plot = false;
            _charts[i].y_axis = ImAxis_Y1;

        }
        ImGui::EndDragDropTarget();
    }

    ImGui::SameLine();

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
        ImPlot::SetupAxisLimits(ImAxis_X1, now - 60, now, ImGuiCond_Always);

        // draw lines
        for (size_t i = 0; i < _charts.size(); ++i) {
            const auto& chart = _charts[i];
            const auto* history = _server->watch_service.history(chart.subcategory, chart.name);
            const auto size = history->size();
            float now = history->back().x();
            size_t offset = history->array_one().first - history->array_two().first;

            if (chart.on_plot && size > 0) {
                ImPlot::SetAxis(chart.y_axis);
                // TODO ImPlot::SetNextLineStyle(dnd[k].Color);
                ImPlot::PlotLine(chart.label.c_str(), chart.p_time, chart.p_value, size, 0, offset, sizeof(boost::geometry::model::d2::point_xy<float>));
                // // allow legend item labels to be DND sources
                if (ImPlot::BeginDragDropSourceItem(chart.label.c_str())) {
                    ImGui::SetDragDropPayload("dnd_area", &i, sizeof(int));
                    //ImPlot::ItemIcon(dnd[k].Color); ImGui::SameLine();
                    ImGui::TextUnformatted(chart.label.c_str());
                    ImPlot::EndDragDropSource();
                }
            }
        }

        // allow the main plot area to be a DND target
        if (ImPlot::BeginDragDropTargetPlot()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("dnd_area")) {
                int i = *(int*)payload->Data;
                _charts[i].on_plot = true;
                _charts[i].y_axis = ImAxis_Y1;
            }
            ImPlot::EndDragDropTarget();
        }

        // allow each y-axis to be a DND target
        for (int y = ImAxis_Y1; y <= ImAxis_Y3; ++y) {
            if (ImPlot::BeginDragDropTargetAxis(y)) {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("dnd_area")) {
                    int i = *(int*)payload->Data;
                    _charts[i].on_plot = true;
                    _charts[i].y_axis = y;
                }
                ImPlot::EndDragDropTarget();
            }
        }

        // allow the legend to be a DND target
        if (ImPlot::BeginDragDropTargetLegend()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("dnd_area")) {
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
