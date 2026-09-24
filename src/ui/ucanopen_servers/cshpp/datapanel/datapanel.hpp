#pragma once

#include <imgui.h>
#include <ui/view/view.h>
#include <ucanopen_servers/cshpp/cshpp_server.hpp>
#include <memory>

namespace ui {
namespace cshpp {

class DataPanel : public View {
private:
    std::shared_ptr<::cshpp::Server> server_;
public:
    DataPanel(std::shared_ptr<::cshpp::Server> server,
              const std::string& menu_title,
              const std::string& window_title,
              bool open);
    virtual void draw() override;
private:
    void draw_tpdo1_table();
    void draw_tpdo2_table();
    void draw_tpdo3_table();

    void draw_pdo_header(ucanopen::CobTpdo tpdo, const char* title);
    void draw_payload_row(ucanopen::CobTpdo tpdo);
    void draw_value_row(const char* title, float value);
    void draw_valve_row(const char* title, ::cshpp::ValvePosition position);
};

} // namespace cshpp
} // namespace ui
