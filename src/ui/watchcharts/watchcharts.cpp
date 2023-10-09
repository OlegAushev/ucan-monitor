#include "watchcharts.h"


namespace ui {


void WatchCharts::draw(bool& open) {
    ImGui::Begin("Watch Charts", &open);

    static ImPlotAxisFlags flags = ImPlotAxisFlags_NoTickLabels;

    const auto& history = _server->watch_service.history("sys", "uptime");

    const float* time = history->time.array_two().first;
    const float* value = history->value.array_two().first;
    const auto size = history->time.size();
    float now = history->time.back();

    size_t offset = history->time.array_one().first - history->time.array_two().first;
    
    if (ImPlot::BeginPlot("##Scrolling", ImVec2(-1,150))) {
        ImPlot::SetupAxes(nullptr, nullptr, flags, flags);
        ImPlot::SetupAxisLimits(ImAxis_X1,now - 60, now, ImGuiCond_Always);
        ImPlot::SetupAxisLimits(ImAxis_Y1,0,1);
        ImPlot::SetNextFillStyle(IMPLOT_AUTO_COL,0.5f);
        //ImPlot::PlotShaded("Mouse X", &sdata1.Data[0].x, &sdata1.Data[0].y, sdata1.Data.size(), -INFINITY, 0, sdata1.Offset, 2 * sizeof(float));
        ImPlot::PlotLine("uptime", time, value, size, 0, offset, 2*sizeof(float));
        ImPlot::EndPlot();
    }

    ImGui::End();
}


} // namespace ui
