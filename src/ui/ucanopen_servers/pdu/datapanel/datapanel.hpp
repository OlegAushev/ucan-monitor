#pragma once

#include <imgui.h>
#include <ui/view/view.h>
#include <ucanopen_servers/pdu/pdu_server.hpp>
#include <memory>

namespace ui {
namespace pdu {

class DataPanel : public View {
private:
    std::shared_ptr<::pdu::Server> server_;
public:
    DataPanel(std::shared_ptr<::pdu::Server> server,
              const std::string& menu_title,
              const std::string& window_title,
              bool open);
    virtual void draw() override;
private:
    void draw_tpdo1_table();
    void draw_tpdo2_table();
    void draw_tpdo3_table();
    void draw_tpdo4_table();

    void draw_pdo_header(ucanopen::CobTpdo tpdo, const char* title);
    void draw_payload_row(ucanopen::CobTpdo tpdo);
    void draw_branch_rows(const char* branch_name,
                          float voltage,
                          float current,
                          ::pdu::ContactorPosition main_command,
                          ::pdu::ContactorPosition main_feedback,
                          ::pdu::ContactorPosition precharge_command,
                          ::pdu::ContactorPosition precharge_feedback,
                          ::pdu::BranchState state,
                          bool connected);
};

} // namespace pdu
} // namespace ui
