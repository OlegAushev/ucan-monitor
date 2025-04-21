#include "throttlecontroller.hpp"
#include <ui/util/util.h>

namespace ui {
namespace shm80 {

ThrottleController::ThrottleController(std::shared_ptr<::shm80::Server> server,
                                       const std::string& menu_title,
                                       const std::string& window_title,
                                       bool open)
        : View(menu_title, window_title, open),
          server_(server),
          send_timepoint_(std::chrono::steady_clock::now()) {}

void ThrottleController::draw() {
    ImGui::Begin(_window_title.c_str(), &_opened);

    ImGui::Checkbox("Вкл/Выкл", &enabled_);

    ImGui::RadioButton("R", &gear_v_, std::to_underlying(MkfJs38Gear::reverse));
    ImGui::SameLine();
    ImGui::RadioButton("R*",
                       &gear_v_,
                       std::to_underlying(MkfJs38Gear::reverse_pre));
    ImGui::SameLine();
    ImGui::RadioButton("N", &gear_v_, std::to_underlying(MkfJs38Gear::neutral));
    ImGui::SameLine();
    ImGui::RadioButton("D*",
                       &gear_v_,
                       std::to_underlying(MkfJs38Gear::drive_pre));
    ImGui::SameLine();
    ImGui::RadioButton("D", &gear_v_, std::to_underlying(MkfJs38Gear::drive));

    gear_ = static_cast<MkfJs38Gear>(gear_v_);

    bool const can_throttle =
            gear_v_ == std::to_underlying(MkfJs38Gear::drive) ||
            gear_v_ == std::to_underlying(MkfJs38Gear::reverse);

    if (!can_throttle) {
        throttle_v_ = 0;
    }

    ui::util::Switchable(can_throttle, [this]() {
        ImGui::SliderInt("Газ [%]", &throttle_v_, 0, 100);
    });

    throttle_ = std::clamp(throttle_v_, 0, 100);

    ImGui::End();

    send_frame();
}

void ThrottleController::send_frame() {
    if (!enabled_) {
        return;
    }

    MkfJs38Message msg{};
    msg.gear = gear_;
    msg.throttle = throttle_;
    if (std::chrono::steady_clock::now() - send_timepoint_ > send_period_) {
        can_frame frame;
        frame.can_id = can_id | (uint32_t(1) << 31);
        frame.can_dlc = 8;
        memcpy(frame.data, &msg, 8);
        server_->send(frame);
    }
}

} // namespace shm80
} // namespace ui
