#pragma once


#include <array>
#include <imgui.h>
#include <ui/view/view.h>
#include <ucanopen_servers/atvvcu/atvvcu_server.h>
#include <memory>


namespace ui {
namespace atvvcu {


class MotorControlPanel : public View {
private:
    std::shared_ptr<::atvvcu::Server> _server;
public:
    MotorControlPanel(std::shared_ptr<::atvvcu::Server> server,
                      const std::string& menu_title,
                      const std::string& window_title,
                      bool show_by_default);
    virtual void draw(bool& open) override;
private:
    std::array<std::string, 4> _window_titles = {"Motor LF##ctl", "Motor LB##ctl", "Motor RF##ctl", "Motor RB##ctl"};

    std::array<int, 4> _ref_ctlmode{};
    std::array<int, 4> _ref_gear{};
    std::array<bool, 4> _ref_enable{};
    std::array<bool, 4> _ref_discharge{};
    std::array<bool, 4> _ref_mainrelay{};
    std::array<bool, 4> _ref_footbrake{};
    std::array<bool, 4> _ref_handbrake{};
    std::array<bool, 4> _ref_faultreset{};

    std::array<int, 4> _ref_speed{};
    std::array<int, 4> _ref_torque{};

    int _ref_eltorque_max{0};
    int _ref_braketorque_max{0};
};


} // namespace atvvcu
} // namespace ui
