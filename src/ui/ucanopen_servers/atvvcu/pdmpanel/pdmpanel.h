#pragma once


#include <imgui.h>
#include <ui/view/view.h>
#include <ucanopen_servers/atvvcu/atvvcu_server.h>
#include <memory>


namespace ui {
namespace atvvcu {


class PdmPanel : public View {
private:
    std::shared_ptr<::atvvcu::Server> _server;
public:
    PdmPanel(std::shared_ptr<::atvvcu::Server> server,
                    const std::string& menu_title,
                    const std::string& window_title,
                    bool show_by_default);
    virtual void draw(bool& open) override;
private:
    void _draw_contactor_states();
private:
    std::array<bool, ::atvvcu::pdm_contactor_count> _contactor_refstates{};
    std::array<bool, ::atvvcu::pdm_contactor_count> _contactor_states{};
};


} // namespace atvvcu
} // namespace ui
