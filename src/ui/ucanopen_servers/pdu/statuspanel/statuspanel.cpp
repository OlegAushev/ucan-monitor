#include "statuspanel.hpp"

#include <ui/util/style.h>

namespace ui {
namespace pdu {

StatusPanel::StatusPanel(std::shared_ptr<::pdu::Server> server,
                         const std::string& menu_title,
                         const std::string& window_title,
                         bool open)
        : View(menu_title, window_title, open), _server(server) {}

void StatusPanel::draw() {
    ImGui::Begin(_window_title.c_str(), &_opened);

    static ImGuiTableFlags flags =
            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
    if (ImGui::BeginTable("status_table", 2, flags)) {
        ImGui::TableSetupColumn("Статус");
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, ImGui::GetTextLineHeight());
        ImGui::TableHeadersRow();

        auto const& status = _server->status();
        for (auto bit = 0uz; bit < ::pdu::status::status_count; ++bit) {
            // find highest active level for this status
            bool active = false;
            ::pdu::trouble::level active_level{};
            for (auto level = ::pdu::trouble::level_count; level-- > 0;) {
                if (status[level].test(bit)) {
                    active = true;
                    active_level = static_cast<::pdu::trouble::level>(level);
                    break;
                }
            }

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            auto const& name = ::pdu::status::names_ru[bit];
            ImGui::TextUnformatted(name.data(), name.data() + name.size());

            if (active) {
                auto bg = [&]() {
                    switch (active_level) {
                    case ::pdu::trouble::level::emergency:
                    case ::pdu::trouble::level::critical:
                    case ::pdu::trouble::level::error:
                        return ui::colors::table_bg_red;
                    case ::pdu::trouble::level::warning:
                        return ui::colors::table_bg_yellow;
                    default:
                        return ui::colors::table_bg_blue;
                    }
                }();
                ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, bg);

                ImGui::TableSetColumnIndex(1);
                if (active_level == ::pdu::trouble::level::critical
                    || active_level == ::pdu::trouble::level::emergency) {
                    ImGui::TextUnformatted(ICON_MDI_ALERT);
                }
                ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, bg);
            }
        }
        ImGui::EndTable();
    }

    ImGui::End();
}

} // namespace pdu
} // namespace ui
