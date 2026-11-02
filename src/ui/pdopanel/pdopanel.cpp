#include "pdopanel.h"
#include <ucanopen/od/pdo_codec.h>
#include <ucanopen/od/value.h>
#include <ui/util/style.h>
#include <ui/util/util.h>

#include <chrono>
#include <format>
#include <map>
#include <optional>


namespace ui {


namespace {


const std::string& label_of(const ucanopen::od::Text& label, const std::string& name) {
    if (!label.ru.empty()) {
        return label.ru;
    }
    return label.en.empty() ? name : label.en;
}


// A signal as people read it: the label of a named value, or the number and
// its unit.
std::string text_of(const ucanopen::od::SignalValue& value) {
    const auto& signal = *value.signal;
    if (signal.role == ucanopen::od::Role::counter) {
        return std::to_string(value.raw);
    }
    if (signal.value_table.has_value()) {
        if (value.named == nullptr) {
            return std::to_string(value.raw) + "?";
        }
        return label_of(value.named->label, value.named->name);
    }
    auto text = ucanopen::od::physical_text(signal, value.raw);
    const auto& unit = signal.unit.ru.empty() ? signal.unit.en : signal.unit.ru;
    if (!unit.empty()) {
        text += " " + unit;
    }
    return text;
}


void row(const std::string& name, const std::string& value) {
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::TextUnformatted(name.c_str());
    ImGui::TableSetColumnIndex(1);
    ImGui::TextUnformatted(value.c_str());
}


} // namespace


PdoPanel::PdoPanel(std::shared_ptr<ucanopen::Server> server,
                   const std::string& menu_title,
                   const std::string& window_title,
                   bool open)
        : View(menu_title, window_title, open)
        , _server(server) {}


void PdoPanel::draw() {
    ImGui::Begin(_window_title.c_str(), &_opened);

    const auto& description = _server->description();
    if (!description) {
        ImGui::TextDisabled("Нет описания словаря.");
        ImGui::End();
        return;
    }

    bool first = true;
    for (const auto& pdo : description->pdos()) {
        if (!first) {
            ImGui::NewLine();
        }
        first = false;
        _draw_pdo(*description, pdo);
    }

    ImGui::End();
}


void PdoPanel::_draw_pdo(const ucanopen::od::Description& description, const ucanopen::od::Pdo& pdo) {
    namespace od = ucanopen::od;
    bool const tx = (pdo.direction == od::Direction::tx);

    // what the PDO carried last: a TPDO while it comes, an RPDO as it went
    std::optional<ucanopen::can_payload> payload;
    bool alive = false;
    if (tx) {
        if (auto tpdo = od::tpdo_of(pdo); tpdo.has_value()) {
            alive = _server->tpdo_service.good(*tpdo);
            if (alive) {
                payload = _server->tpdo_service.data(*tpdo);
            }
        }
    } else if (auto rpdo = od::rpdo_of(pdo); rpdo.has_value()) {
        payload = _server->rpdo_service.data(*rpdo);
        alive = payload.has_value() && _server->rpdo_service.enabled() && _server->rpdo_service.enabled(*rpdo);
    }

    if (alive) {
        ui::util::BlinkingText(ICON_MDI_NETWORK,
                               std::chrono::milliseconds{750},
                               ui::colors::icon_green,
                               ui::colors::icon_inactive);
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_red);
        ImGui::TextUnformatted(ICON_MDI_CLOSE_NETWORK);
        ImGui::PopStyleColor();
    }
    ImGui::SameLine();
    auto const title = std::format("{}{} · {}", tx ? "TPDO" : "RPDO", pdo.number, label_of(pdo.label, pdo.name));
    ImGui::SeparatorText(title.c_str());

    static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
    if (!ImGui::BeginTable(pdo.name.c_str(), 2, flags)) {
        return;
    }
    ImGui::TableSetupColumn("Сигнал");
    ImGui::TableSetupColumn("Значение");
    ImGui::TableHeadersRow();

    // A frame of the MPDO brings one status, and they go round all of them
    // every 10 ms: the frames would only flicker, while the statuses they
    // bring stand still.
    if (pdo.mapping == od::Mapping::sam) {
        _draw_statuses(description, pdo, alive);
        ImGui::EndTable();
        return;
    }

    if (!payload.has_value()) {
        row("Кадр [hex]", "н/д");
    } else {
        const auto& data = *payload;
        row("Кадр [hex]",
            std::format("{:02X} {:02X} {:02X} {:02X} {:02X} {:02X} {:02X} {:02X}",
                        data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]));
    }

    if (auto decoded = payload.has_value() ? od::decode_pdo(description, pdo, *payload) : std::nullopt;
        decoded.has_value()) {
        for (const auto& value : decoded->signals) {
            auto const name = (value.signal->role == od::Role::counter) ? std::string("Счётчик")
                                                                         : label_of(value.signal->label,
                                                                                    value.signal->name);
            row(name, text_of(value));
        }
    } else {
        for (const auto& signal : pdo.signals) {
            auto const name = (signal.role == od::Role::counter) ? std::string("Счётчик")
                                                                  : label_of(signal.label, signal.name);
            row(name, "н/д");
        }
    }

    ImGui::EndTable();
}


// How many statuses the frames brought and how many stand at each level; the
// panel of statuses names them.
void PdoPanel::_draw_statuses(const ucanopen::od::Description& description, const ucanopen::od::Pdo& pdo,
                              bool alive) {
    const auto& service = _server->status_service;
    if (service.pdo() != &pdo || !alive) {
        row("Статусов получено", "н/д");
        return;
    }

    auto const statuses = service.statuses();
    size_t received = 0;
    std::map<uint8_t, size_t> at_level; // by severity
    for (const auto& status : statuses) {
        if (!status.severity.has_value()) {
            continue;
        }
        ++received;
        if (*status.severity != 0) {
            ++at_level[*status.severity];
        }
    }
    row("Статусов получено", std::format("{} из {}", received, statuses.size()));
    if (at_level.empty()) {
        row("На уровне", "нет");
        return;
    }

    const auto* severities = description.value_table(statuses.front().field->value_table);
    for (const auto& [severity, count] : at_level) {
        const auto* value = (severities == nullptr) ? nullptr : severities->find(severity);
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted(value != nullptr ? label_of(value->label, value->name).c_str()
                                                : std::format("{}?", severity).c_str());
        ImGui::TableSetColumnIndex(1);
        if (value != nullptr) {
            bool const red = value->name == "error" || value->name == "critical" || value->name == "emergency";
            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg,
                                   red ? ui::colors::table_bg_red : ui::colors::table_bg_yellow);
        }
        ImGui::Text("%zu", count);
    }
}


} // namespace ui
