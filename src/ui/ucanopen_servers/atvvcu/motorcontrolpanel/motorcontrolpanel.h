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

    std::array<int, 4> _refmode{};
    std::array<int, 4> _refgear{};
    std::array<bool, 4> _refstatus{};
    std::array<bool, 4> _refdischarge{};
    std::array<bool, 4> _refrelay{};
    std::array<bool, 4> _reffootbrake{};
    std::array<bool, 4> _refhandbrake{};
    std::array<bool, 4> _reffaultreset{};

    std::array<int, 4> _refspeed{};
    std::array<int, 4> _reftorque{};

    int _refeltorque_max{0};
    int _refbraketorque_max{0};
};


} // namespace atvvcu
} // namespace ui
