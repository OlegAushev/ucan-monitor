#include "statuspanel.hpp"

#include <ui/util/style.h>

using namespace sevpress;

namespace ui {
namespace sevpress {

StatusPanel::StatusPanel(std::shared_ptr<::sevpress::Server> server,
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
        for (auto bit = 0uz; bit < sys::status::status_count; ++bit) {
            // find highest active level for this status
            bool active = false;
            sys::diag::level active_level{};
            for (auto level = sys::diag::level_count; level-- > 0;) {
                if (status[level].test(bit)) {
                    active = true;
                    active_level = static_cast<sys::diag::level>(level);
                    break;
                }
            }

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted(
                    sys::status::names_ru[bit].data(),
                    sys::status::names_ru[bit].data()
                            + sys::status::names_ru[bit].size());

            if (active) {
                auto bg = [&]() {
                    switch (active_level) {
                    case sys::diag::level::critical:
                    case sys::diag::level::error:
                        return ui::colors::table_bg_red;
                    case sys::diag::level::warning:
                        return ui::colors::table_bg_yellow;
                    default:
                        return ui::colors::table_bg_blue;
                    }
                }();
                ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, bg);

                ImGui::TableSetColumnIndex(1);
                if (active_level == sys::diag::level::critical) {
                    ImGui::TextUnformatted(ICON_MDI_ALERT);
                }
                ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, bg);
            }
        }
        ImGui::EndTable();
    }

    ImGui::End();
}

} // namespace sevpress
} // namespace ui
