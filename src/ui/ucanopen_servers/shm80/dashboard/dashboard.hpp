#pragma once

#include <imgui.h>
#include <memory>
#include <ucanopen_servers/shm80/shm80_server.h>
#include <ui/util/style.h>
#include <ui/view/view.h>

namespace ui {
namespace shm80 {

class Dashboard : public ui::View {
private:
    static constexpr float converter_warn_temp = 70.f;
    static constexpr float converter_err_temp = 80.f;
    static constexpr float motor_warn_temp = 70.f;
    static constexpr float motor_err_temp = 80.f;
    static constexpr float reduction_gear_warn_temp = 70.f;
    static constexpr float reduction_gear_err_temp = 80.f;
    static constexpr float motor_speed_max = 8000;
    void color_cell(float val, float warn_val, float err_val) {
        if (val > err_val) {
            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg,
                                   ui::colors::table_bg_red);
        } else if (val > warn_val) {
            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg,
                                   ui::colors::table_bg_yellow);
        }
    }

private:
    std::shared_ptr<::shm80::Server> server_;
public:
    Dashboard(std::shared_ptr<::shm80::Server> server,
              const std::string& menu_title,
              const std::string& window_title,
              bool open);
    virtual void draw() override;
private:
    void draw_converter();
    void draw_motor();
    void draw_discretes();
    void draw_pdu();
    void draw_throttle();
    void draw_errors();
private:
    std::chrono::time_point<std::chrono::steady_clock> error_update_timepoint_;
    std::chrono::milliseconds error_update_period_{1000};
    static constexpr float table_val_width = 140.f;
};

} // namespace shm80
} // namespace ui
