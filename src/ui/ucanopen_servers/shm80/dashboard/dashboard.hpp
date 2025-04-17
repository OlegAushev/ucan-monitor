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
    void draw_discretes();
    void draw_pdu();
};

} // namespace shm80
} // namespace ui
