#pragma once


#include <imgui.h>
#include <ui/view/view.h>
#include <ucanopen_servers/atvvcu/atvvcu_server.h>
#include <memory>


namespace ui {
namespace atvvcu {


class Dashboard : public View {
private:
    std::shared_ptr<::atvvcu::Server> _server;

    bool _dash_dbg{false};
    bool _pdm_dbg{false};
    bool _drive_dbg{false};
    bool _gear_dbg{false};
    bool _accl_dbg{false};
    bool _steer_dbg{false};
    bool _brake_dbg{false};
    bool _esp_dbg{false};

    bool _emergency{false};
    bool _power_enabled{false};
    bool _run_enabled{false};
    bool _fault_reset{false};

    bool _turn_left{false};
    bool _turn_right{false};

    int _charging_mode{0};

    int _ref_gear{0};
    float _accl{0};

    bool _brake_left{false};
    bool _brake_right{false};

    bool _tcs_enable{true};
public:
    Dashboard(std::shared_ptr<::atvvcu::Server> server,
                const std::string& menu_title,
                const std::string& window_title,
                bool open);
    virtual void draw() override;
private:
    void _draw_debug_controls();
    void _draw_misc_controls();
    void _draw_status();
    void _read_keyboard();

    void _draw_dash();
    void _draw_gear();
    void _draw_accl_brakes();
};


} // namespace atvvcu
} // namespace ui
