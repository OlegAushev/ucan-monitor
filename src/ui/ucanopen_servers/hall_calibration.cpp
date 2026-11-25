#include "hall_calibration.hpp"

#include <icons/IconsMaterialDesignIcons.h>
#include <ucanopen/server/server.h>
#include <ucanopen_servers/hall_calibration.hpp>
#include <ui/util/style.h>

namespace ui {

namespace {

using Result = inverter::HallCalibration::Result;

const char* failure_message(Result result, const ucanopen::ServerStatusService& statuses) {
    // The diagnostic can arrive in an MPDO after the drive leaves calibrating.
    if (result == Result::interrupted || statuses.active("hall_calibration_terminated")) {
        return "Калибровка прервана, углы датчика остались прежними.";
    }
    if (statuses.active("hall_invalid_calibration")) {
        return "Калибровка дала некорректные углы. Углы датчика остались прежними.";
    }
    if (result == Result::fault) {
        return "Во время калибровки привод перешёл в аварию.";
    }
    return "Результат калибровки неизвестен.";
}

} // namespace

void draw_hall_calibration(ucanopen::Server& server,
                           inverter::HallCalibration& calibration) {
    auto const result = calibration.result(server.node_id().get());
    if (result == Result::none) {
        return;
    }

    ImGui::SeparatorText("Калибровка датчика Холла");
    if (result == Result::completed) {
        ImGui::TextWrapped("Калибровка закончена: двенадцать углов датчика "
                           "запрошены. Примените их, пока привод стоит с "
                           "выключенным мостом, затем сохраните в «Настройке».");
        if (ImGui::Button(ICON_MDI_CHECK_ALL " Применить всё")) {
            server.command("settings.apply_all");
        }
        ImGui::SameLine();
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, colors::icon_red);
        ImGui::TextWrapped("%s", failure_message(result, server.status_service));
        ImGui::PopStyleColor();
    }
    if (ImGui::Button(ICON_MDI_CLOSE " Скрыть##calibration")) {
        calibration.dismiss(result);
    }
}

} // namespace ui
