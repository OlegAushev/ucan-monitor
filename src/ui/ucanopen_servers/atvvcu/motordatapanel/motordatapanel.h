#pragma once


#include <imgui.h>
#include <string_view>
#include <ui/view/view.h>
#include <ucanopen_servers/atvvcu/atvvcu_server.h>


namespace ui {
namespace atvvcu {


class MotorDataPanel : public ui::View {
private:
    std::shared_ptr<::atvvcu::Server> _server;
public:
    MotorDataPanel(std::shared_ptr<::atvvcu::Server> server,
                   const std::string& menu_title,
                   const std::string& window_title,
                   bool open);
    virtual void draw() override;
private:
    std::array<std::string, 4> _window_titles = {"Motor LF Data##data",
                                                 "Motor LB Data##data",
                                                 "Motor RF Data##data",
                                                 "Motor RB Data##data"};

    std::array<std::string_view, 4> _vdc_watchobj = {"vdc_lf", "vdc_lb", "vdc_rf", "vdc_rb"};
    std::array<std::string_view, 4> _iac_watchobj = {"iac_lf", "iac_lb", "iac_rf", "iac_rb"};
    std::array<std::string_view, 4> _torque_watchobj = {"torque_lf", "torque_lb", "torque_rf", "torque_rb"};
    std::array<std::string_view, 4> _speed_watchobj = {"speed_lf", "speed_lb", "speed_rf", "speed_rb"};
    std::array<std::string_view, 4> _motortemp_watchobj = {"motortemp_lf", "motortemp_lb", "motortemp_rf", "motortemp_rb"};
    std::array<std::string_view, 4> _moduletemp_watchobj = {"moduletemp_lf", "moduletemp_lb", "moduletemp_rf", "moduletemp_rb"};
};


} // namespace atvvcu
} // namespace ui
