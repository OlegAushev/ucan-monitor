#pragma once

namespace inverter { class HallCalibration; }
namespace ucanopen { class Server; }

namespace ui {

void draw_hall_calibration(ucanopen::Server& server,
                           inverter::HallCalibration& calibration);

} // namespace ui
