#pragma once


#include <imgui.h>
#include <ui/view/view.h>
#include <ucanopen_servers/pdu/pdu_server.hpp>

#include <array>
#include <memory>


namespace ui {
namespace pdu {

class ControlPanel : public View {
private:
    std::shared_ptr<::pdu::Server> _server;

    ::pdu::Mode _mode{::pdu::Mode::idle};
    int _mode_v{std::to_underlying(::pdu::Mode::idle)};

    bool _inverter_request{false};

    // Ручные уровни контакторов: PDU слушает их только в продувке.
    bool _fuelcell_main{false};
    bool _fuelcell_precharge{false};
    bool _inverter_main{false};
    bool _inverter_precharge{false};

    // Замещающие напряжения батареи, ветви ЭХГ и ветви инвертора в том виде, в
    // каком их последний раз отправили: PDU держит их в RAM и берёт только для
    // выведенных из работы датчиков.
    std::array<float, 3> _substitutes{};
public:
    ControlPanel(std::shared_ptr<::pdu::Server> server,
                 const std::string& menu_title,
                 const std::string& window_title,
                 bool open);
    virtual void draw() override;
private:
    void _draw_dash();
    void _draw_controls();
    void _draw_substitutes();
    void _draw_actions();

    void _draw_popups();
    void _read_keyboard();

    void _reset_refs();
    void _update_refs();

    void _draw_branch_state(const char* title,
                            ::pdu::BranchState state,
                            bool connected,
                            ::pdu::ContactorPosition main_feedback,
                            ::pdu::ContactorPosition precharge_feedback,
                            float voltage,
                            float current);
};


} // namespace pdu
} // namespace ui
