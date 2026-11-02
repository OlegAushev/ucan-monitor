#include "serversetuppanel.h"
#include <config_file/config_file.h>
#include <ucanopen/od/value.h>
#include <ui/util/style.h>
#include <ui/util/util.h>

#include <imguifiledialog/ImGuiFileDialog.h>

#include <algorithm>
#include <array>
#include <ctime>
#include <filesystem>
#include <format>
#include <map>
#include <set>
#include <utility>

namespace ui {

namespace {

namespace od = ucanopen::od;
using ucanopen::ExpeditedSdoData;
using ucanopen::SdoStep;

// The members of the record of a parameter (emblib,
// docs/canopen-od-algorithm.ru.md, section 3.5).
constexpr uint8_t requested = 0x01;
constexpr uint8_t applied = 0x02;
constexpr uint8_t stored = 0x03;
constexpr uint8_t default_value = 0x04;
constexpr uint8_t min_value = 0x05;
constexpr uint8_t max_value = 0x06;
constexpr uint8_t policy = 0x07;
constexpr uint8_t apply = 0x08;
constexpr uint8_t discard = 0x09;
constexpr uint8_t actual = 0x0A;

// The parameters of this group a file loads only when picked by hand: the
// node's own ID, loaded from another device's file, would move this device
// elsewhere on the bus.
constexpr std::string_view manual_group = "canopen";

std::string local_time(const char* format) {
    auto now = std::time(nullptr);
    std::tm tm{};
    localtime_r(&now, &tm);
    std::array<char, 64> buf;
    auto size = std::strftime(buf.data(), buf.size(), format, &tm);
    return std::string(buf.data(), size);
}

const std::string& label_of(const od::Text& label, const std::string& fallback) {
    if (!label.ru.empty()) {
        return label.ru;
    }
    return label.en.empty() ? fallback : label.en;
}

const std::string& unit_of(const od::Text& unit) {
    return unit.ru.empty() ? unit.en : unit.ru;
}

std::string parameter_label(const od::Record& record) {
    auto text = label_of(record.label, record.name);
    if (const auto& unit = unit_of(record.unit); !unit.empty()) {
        text += " [" + unit + "]";
    }
    return text;
}

bool has_member(const od::Description& description, const od::Record& record, uint8_t subindex) {
    return description.layouts()[record.layout].find(subindex) != nullptr;
}

// An input of a value of the item's type; true once a person enters one.
bool input_value(const char* label, const od::Item& item, ExpeditedSdoData& data) {
    constexpr ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue;
    switch (item.datatype) {
    case od::DataType::boolean: {
        // a choice, as of a named value, rather than a box that a click
        // alone writes
        bool const value = data.u8() != 0;
        bool chosen = false;
        if (ImGui::BeginCombo(label, value ? "да" : "нет")) {
            for (bool option : {false, true}) {
                if (ImGui::Selectable(option ? "да" : "нет", option == value)) {
                    data = ExpeditedSdoData(option);
                    chosen = true;
                }
            }
            ImGui::EndCombo();
        }
        return chosen;
    }
    case od::DataType::int8: {
        int8_t value = data.i8();
        if (ImGui::InputScalar(label, ImGuiDataType_S8, &value, NULL, NULL, "%d", flags)) {
            data = ExpeditedSdoData(value);
            return true;
        }
        return false;
    }
    case od::DataType::int16: {
        int16_t value = data.i16();
        if (ImGui::InputScalar(label, ImGuiDataType_S16, &value, NULL, NULL, "%d", flags)) {
            data = ExpeditedSdoData(value);
            return true;
        }
        return false;
    }
    case od::DataType::int32: {
        int32_t value = data.i32();
        if (ImGui::InputScalar(label, ImGuiDataType_S32, &value, NULL, NULL, "%d", flags)) {
            data = ExpeditedSdoData(value);
            return true;
        }
        return false;
    }
    case od::DataType::uint8: {
        uint8_t value = data.u8();
        if (ImGui::InputScalar(label, ImGuiDataType_U8, &value, NULL, NULL, "%u", flags)) {
            data = ExpeditedSdoData(value);
            return true;
        }
        return false;
    }
    case od::DataType::uint16: {
        uint16_t value = data.u16();
        if (ImGui::InputScalar(label, ImGuiDataType_U16, &value, NULL, NULL, "%u", flags)) {
            data = ExpeditedSdoData(value);
            return true;
        }
        return false;
    }
    case od::DataType::uint32: {
        uint32_t value = data.u32();
        if (ImGui::InputScalar(label, ImGuiDataType_U32, &value, NULL, NULL, "%u", flags)) {
            data = ExpeditedSdoData(value);
            return true;
        }
        return false;
    }
    case od::DataType::float32: {
        float value = data.f32();
        if (ImGui::InputScalar(label, ImGuiDataType_Float, &value, NULL, NULL, "%.6g", flags)) {
            data = ExpeditedSdoData(value);
            return true;
        }
        return false;
    }
    default:
        ImGui::TextDisabled("тип не поддерживается");
        return false;
    }
}

} // namespace

ServerSetupPanel::ServerSetupPanel(std::shared_ptr<ucanopen::Server> server,
                                   const std::string& menu_title,
                                   const std::string& window_title,
                                   bool open)
        : View(menu_title, window_title, open),
          _server(server),
          _save_dialog_key("config_save_" + server->name()),
          _load_dialog_key("config_load_" + server->name()) {
    const auto& description = *_server->description();
    for (const auto& [index, record] : description.records()) {
        if (record.cls != od::Class::parameters) {
            continue;
        }
        auto group = std::find_if(_groups.begin(), _groups.end(), [&record](const Group& g) {
            return g.group->name == record.group;
        });
        if (group == _groups.end()) {
            _groups.push_back({description.group(record.group), {}});
            group = std::prev(_groups.end());
        }
        group->records.push_back(&record);
    }
}

bool ServerSetupPanel::Row::differs() const {
    if (!file_value.has_value()) {
        return false;
    }
    return !stored.has_value() || !config_file::same_value(item, *stored, *file_value);
}

void ServerSetupPanel::draw() {
    ImGui::Begin(_window_title.c_str(), &_opened);

    // What the panel learned of the parameters holds for one connection.
    if (auto const epoch = _server->heartbeat_service.epoch(); epoch != _epoch) {
        _epoch = epoch;
        _constants.clear();
        _shown.clear();
        _should_read = true;
    }

    _take_transfer();
    _draw_about();
    _draw_state();
    _draw_parameter();
    _draw_actions();
    _draw_all_parameters();
    _draw_popups();
    _draw_dialogs();

    ImGui::End();
}

void ServerSetupPanel::_draw_about() {
    ImGui::SeparatorText("О сервере");
    if (ImGui::Button(ICON_MDI_REFRESH " Обновить##about")) {
        _server->info_service.refresh();
    }

    auto const info = _server->info_service.info();
    auto const file_fingerprint = _server->description()->fingerprint();
    for (const auto& entry : info.entries) {
        ImGui::TextDisabled("%s: ", label_of(entry.item.label, entry.item.path).c_str());
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
            ImGui::SetTooltip("%s", entry.item.path.c_str());
        }
        ImGui::SameLine();
        if (entry.item.path == "dictionary.fingerprint" && info.fingerprint.has_value()) {
            ImGui::Text("0x%08X", *info.fingerprint);
        } else {
            ImGui::TextUnformatted(entry.value.has_value() ? entry.value->c_str() : "н/д");
        }
    }

    ImGui::TextDisabled("Описание словаря: ");
    ImGui::SameLine();
    ImGui::Text("%s, отпечаток 0x%08X", _server->description()->device().c_str(), file_fingerprint);
    if (_server->info_service.fingerprint_matches() == false) {
        ImGui::PushStyleColor(ImGuiCol_Text, colors::icon_yellow);
        ImGui::TextWrapped("Описание словаря не от этой прошивки: в устройстве 0x%08X, в файле 0x%08X.",
                           info.fingerprint.value_or(0),
                           file_fingerprint);
        ImGui::PopStyleColor();
    }
}

// What waits in the settings, as the statuses that the device sends say.
void ServerSetupPanel::_draw_state() {
    struct Flag {
        std::string_view status;
        const char* text;
    };
    static constexpr std::array<Flag, 3> flags = {{
            {"unapplied_changes", ICON_MDI_ALERT_OUTLINE " есть неприменённое"},
            {"unsaved_changes", ICON_MDI_CONTENT_SAVE_ALERT_OUTLINE " есть несохранённое"},
            {"restart_required", ICON_MDI_RESTART_ALERT " требуется перезапуск"}}};

    bool shown = false;
    for (const auto& flag : flags) {
        if (!_status_active(flag.status)) {
            continue;
        }
        if (shown) {
            ImGui::SameLine();
        }
        ImGui::PushStyleColor(ImGuiCol_Text, colors::icon_yellow);
        ImGui::TextUnformatted(flag.text);
        ImGui::PopStyleColor();
        shown = true;
    }
}

void ServerSetupPanel::_draw_parameter() {
    ImGui::SeparatorText("Параметр");
    if (_groups.empty()) {
        ImGui::TextDisabled("Описание словаря не называет параметров.");
        return;
    }

    // A transfer would overwrite a parameter set by hand meanwhile.
    util::DisableGuard disabled(_busy());

    if (ImGui::Button(ICON_MDI_REFRESH " Обновить##parameter")) {
        _should_read = true;
    }

    _group_idx = std::min(_group_idx, _groups.size() - 1);
    const auto& group = _groups[_group_idx];
    if (ImGui::BeginCombo("Группа", label_of(group.group->label, group.group->name).c_str())) {
        for (size_t i = 0; i < _groups.size(); ++i) {
            const auto& g = *_groups[i].group;
            if (ImGui::Selectable(label_of(g.label, g.name).c_str(), i == _group_idx)) {
                _group_idx = i;
                _record_idx = 0;
                _shown.clear();
                _should_read = true;
            }
        }
        ImGui::EndCombo();
    }

    const auto& records = _groups[_group_idx].records;
    _record_idx = std::min(_record_idx, records.size() - 1);
    if (ImGui::BeginCombo("Параметр", parameter_label(*records[_record_idx]).c_str())) {
        for (size_t i = 0; i < records.size(); ++i) {
            if (ImGui::Selectable(parameter_label(*records[i]).c_str(), i == _record_idx)) {
                _record_idx = i;
                _shown.clear();
                _should_read = true;
            }
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
                ImGui::SetTooltip("%s.%s", records[i]->group.c_str(), records[i]->name.c_str());
            }
        }
        ImGui::EndCombo();
    }

    if (_should_read && !_busy()) {
        _read_parameter();
    }

    const auto& description = *_server->description();
    const auto& record = *records[_record_idx];
    auto const value_of = [&](uint8_t subindex) -> std::string {
        auto value = _shown.find(subindex);
        if (subindex >= default_value && subindex <= policy) {
            auto constant = _constant(record, subindex);
            if (!constant.has_value()) {
                return "н/д";
            }
            auto item = description.resolve({record.index, subindex});
            return item.has_value() ? od::format_value(*item, *constant) : "н/д";
        }
        if (value == _shown.end()) {
            return "н/д";
        }
        auto item = description.resolve({record.index, subindex});
        return item.has_value() ? od::format_value(*item, value->second) : "н/д";
    };

    // The value requested is the one to set; the others are what it led to.
    auto item = description.resolve({record.index, requested});
    auto shown = _shown.find(requested);
    ImGui::PushItemWidth(240);
    if (!item.has_value() || shown == _shown.end()) {
        std::string unknown = "н/д";
        ImGui::InputText("Запрошено", unknown.data(), unknown.size() + 1, ImGuiInputTextFlags_ReadOnly);
    } else if (item->values != nullptr) {
        // a value of a table, within the bounds if they are known
        auto const current = static_cast<int64_t>(od::to_double(*item, shown->second));
        auto const min = _constant(record, min_value);
        auto const max = _constant(record, max_value);
        if (ImGui::BeginCombo("Запрошено", od::format_value(*item, shown->second).c_str())) {
            for (const auto& value : item->values->values) {
                if ((min.has_value() && value.value < static_cast<int64_t>(od::to_double(*item, *min))) ||
                    (max.has_value() && value.value > static_cast<int64_t>(od::to_double(*item, *max)))) {
                    continue;
                }
                if (ImGui::Selectable(label_of(value.label, value.name).c_str(), value.value == current)) {
                    if (auto data = od::parse_value(*item, value.name); data.has_value()) {
                        _write_requested(*data);
                    }
                }
            }
            ImGui::EndCombo();
        }
    } else {
        auto data = shown->second;
        if (input_value("Запрошено", *item, data)) {
            _write_requested(data);
        }
    }
    ImGui::PopItemWidth();

    ImGui::TextDisabled("Применено:");
    ImGui::SameLine();
    ImGui::TextUnformatted(value_of(applied).c_str());
    ImGui::TextDisabled("Сохранено:");
    ImGui::SameLine();
    ImGui::TextUnformatted(value_of(stored).c_str());
    if (has_member(description, record, actual)) {
        ImGui::TextDisabled("Фактическое:");
        ImGui::SameLine();
        ImGui::TextUnformatted(value_of(actual).c_str());
    }
    ImGui::TextDisabled("По умолчанию:");
    ImGui::SameLine();
    ImGui::TextUnformatted(value_of(default_value).c_str());
    ImGui::TextDisabled("Границы:");
    ImGui::SameLine();
    ImGui::Text("%s … %s", value_of(min_value).c_str(), value_of(max_value).c_str());
    ImGui::TextDisabled("Применяется:");
    ImGui::SameLine();
    ImGui::TextUnformatted(value_of(policy).c_str());

    if (ImGui::Button(ICON_MDI_CHECK " Применить##parameter")) {
        _command_member(apply);
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_MDI_UNDO " Отбросить##parameter")) {
        _command_member(discard);
    }
    ImGui::SameLine();
    {
        auto const default_data = _constant(record, default_value);
        util::DisableGuard unknown(!default_data.has_value());
        if (ImGui::Button(ICON_MDI_RESTORE " По умолчанию##parameter") && default_data.has_value()) {
            _write_requested(*default_data);
        }
    }
}

void ServerSetupPanel::_draw_actions() {
    ImGui::SeparatorText("Все параметры устройства");
    {
        util::DisableGuard disabled(_busy());
        ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2{0.f, 0.5f});
        if (ImGui::Button(ICON_MDI_CHECK_ALL " Применить всё", ImVec2(-1.0f, 0))) {
            _command("settings.apply_all");
        }
        if (ImGui::Button(ICON_MDI_UNDO_VARIANT " Отбросить всё", ImVec2(-1.0f, 0))) {
            _command("settings.discard_all");
        }
        if (ImGui::Button(ICON_MDI_CONTENT_SAVE " Сохранить", ImVec2(-1.0f, 0))) {
            ImGui::OpenPopup("Внимание!##save");
        }
        if (ImGui::Button(ICON_MDI_CONTENT_SAVE_MOVE " Сохранить и перезапустить", ImVec2(-1.0f, 0))) {
            ImGui::OpenPopup("Внимание!##save_and_reset");
        }
        if (ImGui::Button(ICON_MDI_RESTORE " Восстановить умолчания", ImVec2(-1.0f, 0))) {
            ImGui::OpenPopup("Внимание!##restore");
        }
        if (ImGui::Button(ICON_MDI_ERASER " Очистить память", ImVec2(-1.0f, 0))) {
            ImGui::OpenPopup("Внимание!##erase");
        }
        ImGui::PopStyleVar();
    }

    if (!_result.empty()) {
        if (_result_color != 0) {
            ImGui::PushStyleColor(ImGuiCol_Text, _result_color);
        }
        ImGui::TextWrapped("%s", _result.c_str());
        if (_result_color != 0) {
            ImGui::PopStyleColor();
        }
    }
}

void ServerSetupPanel::_draw_all_parameters() {
    ImGui::SeparatorText("Все параметры");

    {
        util::DisableGuard disabled(_busy());
        if (ImGui::Button(ICON_MDI_TABLE_REFRESH " Прочитать всё", ImVec2(-1.0f, 0))) {
            _read_all();
        }

        // Only a table of all the values makes a whole file.
        bool const nothing_to_save =
                (_table != Table::values) || std::none_of(_rows.begin(), _rows.end(), [](const auto& row) {
                    return row.stored.has_value() || row.applied.has_value();
                });
        {
            util::DisableGuard save_disabled(nothing_to_save);
            if (ImGui::Button(ICON_MDI_CONTENT_SAVE_OUTLINE " Сохранить в файл...", ImVec2(-1.0f, 0))) {
                _open_save_dialog();
            }
        }

        if (ImGui::Button(ICON_MDI_FOLDER_OPEN_OUTLINE " Загрузить из файла...", ImVec2(-1.0f, 0))) {
            _open_load_dialog();
        }
    }

    if (_table == Table::file) {
        auto const selected = _selected_count();
        util::DisableGuard disabled(_busy() || selected == 0);
        auto label = std::string(ICON_MDI_UPLOAD " Записать и применить выбранные (") + std::to_string(selected) +
                     ")###write_file";
        if (ImGui::Button(label.c_str(), ImVec2(-1.0f, 0))) {
            ImGui::OpenPopup("Внимание!##write_file");
        }
    }

    auto& config = _server->config_service;
    if (config.busy() && _transfer != Transfer::parameter && _transfer != Transfer::command) {
        auto [done, total] = config.progress();
        auto overlay = std::to_string(done) + " / " + std::to_string(total);
        const char* cancel_label = ICON_MDI_CLOSE " Отмена##transfer";
        float cancel_width =
                ImGui::CalcTextSize(cancel_label, nullptr, true).x + 2 * ImGui::GetStyle().FramePadding.x;
        ImGui::ProgressBar(total ? float(done) / float(total) : 0.0f,
                           ImVec2(-(cancel_width + ImGui::GetStyle().ItemSpacing.x), 0),
                           overlay.c_str());
        ImGui::SameLine();
        if (ImGui::Button(cancel_label)) {
            config.cancel();
        }
    }

    if (!_notes.empty()) {
        auto label = "Замечания к файлу " + _file_name + " (" + std::to_string(_notes.size()) + ")###notes";
        ImGui::SetNextItemOpen(true, ImGuiCond_Appearing);
        if (ImGui::TreeNode(label.c_str())) {
            for (const auto& note : _notes) {
                ImGui::Bullet();
                ImGui::TextWrapped("%s", note.c_str());
            }
            ImGui::TreePop();
        }
    }

    if (!_hint.empty()) {
        ImGui::TextWrapped("%s", _hint.c_str());
    }

    if (_table == Table::values) {
        _draw_values_table();
    } else {
        _draw_file_table();
    }
}

void ServerSetupPanel::_draw_values_table() {
    if (_rows.empty()) {
        return;
    }

    constexpr ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg |
                                      ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV |
                                      ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingFixedFit;
    float const min_height = 10 * ImGui::GetFrameHeightWithSpacing();
    ImVec2 const size(0.0f, std::max(ImGui::GetContentRegionAvail().y, min_height));
    if (!ImGui::BeginTable("##parameters", 6, flags, size)) {
        return;
    }

    ImGui::TableSetupScrollFreeze(0, 1);
    ImGui::TableSetupColumn("Группа");
    ImGui::TableSetupColumn("Параметр");
    ImGui::TableSetupColumn("Запрошено");
    ImGui::TableSetupColumn("Применено");
    ImGui::TableSetupColumn("Сохранено");
    ImGui::TableSetupColumn("Ед.", ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableHeadersRow();

    const auto& description = *_server->description();
    auto cell = [&](const Row& row, const std::optional<ExpeditedSdoData>& value) {
        if (value.has_value()) {
            ImGui::TextUnformatted(od::format_value(row.item, *value).c_str());
        } else if (!row.status.empty()) {
            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, row.status_color);
            ImGui::TextUnformatted(row.status.c_str());
        }
    };

    for (const auto& row : _rows) {
        // Dimmed as they are commented out in the file: there to be seen,
        // not loaded.
        bool const read_only = (row.record == nullptr);
        if (read_only) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
        }

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        const auto* group = description.group(row.group);
        ImGui::TextUnformatted(group != nullptr ? label_of(group->label, group->name).c_str() : row.group.c_str());
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(row.record != nullptr ? label_of(row.record->label, row.name).c_str()
                                                     : label_of(row.item.label, row.name).c_str());
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
            ImGui::SetTooltip("%s.%s%s", row.group.c_str(), row.name.c_str(), read_only ? ", только для чтения" : "");
        }

        ImGui::TableNextColumn();
        if (row.record != nullptr) {
            bool const unapplied = row.requested.has_value() && row.applied.has_value() &&
                                   !config_file::same_value(row.item, *row.requested, *row.applied);
            if (unapplied) {
                ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, colors::table_bg_yellow);
            }
            cell(row, row.requested);
        }
        ImGui::TableNextColumn();
        cell(row, row.applied);
        ImGui::TableNextColumn();
        if (row.record != nullptr) {
            bool const unsaved = row.applied.has_value() && row.stored.has_value() &&
                                 !config_file::same_value(row.item, *row.applied, *row.stored);
            if (unsaved) {
                ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, colors::table_bg_blue);
            }
            cell(row, row.stored);
        }
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(unit_of(row.item.unit).c_str());

        if (read_only) {
            ImGui::PopStyleColor();
        }
    }

    ImGui::EndTable();
}

void ServerSetupPanel::_draw_file_table() {
    if (_rows.empty()) {
        return;
    }

    constexpr ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg |
                                      ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV |
                                      ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingFixedFit;
    float const min_height = 10 * ImGui::GetFrameHeightWithSpacing();
    ImVec2 const size(0.0f, std::max(ImGui::GetContentRegionAvail().y, min_height));
    if (!ImGui::BeginTable("##file", 7, flags, size)) {
        return;
    }

    ImGui::TableSetupScrollFreeze(0, 1);
    ImGui::TableSetupColumn("##selected");
    ImGui::TableSetupColumn("Группа");
    ImGui::TableSetupColumn("Параметр");
    ImGui::TableSetupColumn("Сохранено");
    ImGui::TableSetupColumn("В файле");
    ImGui::TableSetupColumn("Ед.");
    ImGui::TableSetupColumn("Статус", ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableHeadersRow();

    const auto& description = *_server->description();
    bool const busy = _busy();
    for (size_t i = 0; i < _rows.size(); ++i) {
        auto& row = _rows[i];
        bool const differs = row.differs();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        if (differs) {
            util::DisableGuard disabled(busy);
            ImGui::PushID(static_cast<int>(i));
            ImGui::Checkbox("##selected", &row.selected);
            ImGui::PopID();
        }
        ImGui::TableNextColumn();
        const auto* group = description.group(row.group);
        ImGui::TextUnformatted(group != nullptr ? label_of(group->label, group->name).c_str() : row.group.c_str());
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(label_of(row.record->label, row.name).c_str());
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
            ImGui::SetTooltip("%s.%s", row.group.c_str(), row.name.c_str());
        }
        ImGui::TableNextColumn();
        if (row.stored.has_value()) {
            ImGui::TextUnformatted(od::format_value(row.item, *row.stored).c_str());
        } else {
            ImGui::TextDisabled("н/д");
        }
        ImGui::TableNextColumn();
        if (differs && row.stored.has_value()) {
            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, colors::table_bg_yellow);
        }
        ImGui::TextUnformatted(od::format_value(row.item, *row.file_value).c_str());
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(unit_of(row.item.unit).c_str());
        ImGui::TableNextColumn();
        if (row.status_color != 0) {
            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, row.status_color);
        }
        ImGui::TextUnformatted(row.status.c_str());
    }

    ImGui::EndTable();
}

void ServerSetupPanel::_draw_popups() {
    // A confirmation of a command on all the parameters: its question, what
    // it runs.
    struct Confirmation {
        const char* id;
        const char* question;
        const char* command;
    };
    static constexpr std::array<Confirmation, 4> confirmations = {{
            {"Внимание!##save", "Применённые настройки будут сохранены. Продолжить?", "settings.save_all_parameters"},
            {"Внимание!##save_and_reset",
             "Применённые настройки будут сохранены, устройство перезапустится. Продолжить?",
             "settings.save_all_parameters_and_reset"},
            {"Внимание!##restore",
             "Будут запрошены настройки по умолчанию; в силу их введёт «Применить всё». Продолжить?",
             "settings.restore_all_default_parameters"},
            {"Внимание!##erase", "Энергонезависимая память будет очищена. Продолжить?", "settings.erase_all_parameters"}}};

    // Always center this window when appearing
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    for (const auto& confirmation : confirmations) {
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        if (ImGui::BeginPopupModal(confirmation.id, NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::TextUnformatted(confirmation.question);
            ImGui::Separator();

            if (ImGui::Button(ICON_MDI_CANCEL " Нет", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::SetItemDefaultFocus();
            ImGui::SameLine();
            if (ImGui::Button(ICON_MDI_CHECK " Да", ImVec2(120, 0))) {
                _command(confirmation.command);
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }

    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("Внимание!##write_file", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Будет записано и применено параметров из файла: %zu. Продолжить?", _selected_count());
        if (!_file_server.empty() && _file_server != _server->name()) {
            ImGui::PushStyleColor(ImGuiCol_Text, colors::icon_yellow);
            ImGui::Text("Файл сохранён для %s, а не для %s.", _file_server.c_str(), _server->name().c_str());
            ImGui::PopStyleColor();
        }
        if (_status_active("unapplied_changes")) {
            ImGui::PushStyleColor(ImGuiCol_Text, colors::icon_yellow);
            ImGui::TextUnformatted("В устройстве есть неприменённые изменения: они применятся вместе с файлом.");
            ImGui::PopStyleColor();
        }
        ImGui::Separator();

        if (ImGui::Button(ICON_MDI_CANCEL " Нет", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button(ICON_MDI_CHECK " Да", ImVec2(120, 0))) {
            _write_selected();
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void ServerSetupPanel::_draw_dialogs() {
    auto* dialog = ImGuiFileDialog::Instance();
    ImVec2 const min_size(640.0f, 400.0f);

    if (dialog->Display(_save_dialog_key, ImGuiWindowFlags_NoCollapse, min_size)) {
        if (dialog->IsOk()) {
            _save_file(dialog->GetFilePathName());
        }
        dialog->Close();
    }

    if (dialog->Display(_load_dialog_key, ImGuiWindowFlags_NoCollapse, min_size)) {
        if (dialog->IsOk()) {
            _load_file(dialog->GetFilePathName(IGFD_ResultMode_KeepInputFile));
        }
        dialog->Close();
    }
}

//----------------------------------------------------------------------------------------------------------------------
bool ServerSetupPanel::_busy() const {
    return _transfer != Transfer::none || _server->config_service.busy();
}

size_t ServerSetupPanel::_selected_count() const {
    return std::count_if(_rows.begin(), _rows.end(), [](const auto& row) { return row.selected && row.differs(); });
}

const ucanopen::od::Record* ServerSetupPanel::_record() const {
    if (_groups.empty()) {
        return nullptr;
    }
    const auto& records = _groups[std::min(_group_idx, _groups.size() - 1)].records;
    return records[std::min(_record_idx, records.size() - 1)];
}

std::optional<ExpeditedSdoData> ServerSetupPanel::_constant(const ucanopen::od::Record& record,
                                                            uint8_t subindex) const {
    auto members = _constants.find(record.index);
    if (members == _constants.end()) {
        return std::nullopt;
    }
    auto value = members->second.find(subindex);
    if (value == members->second.end()) {
        return std::nullopt;
    }
    return value->second;
}

// The firmware names the refusals of the settings itself, in the table of
// settings.last_refusal; the others go by the texts of CiA 301.
std::string ServerSetupPanel::_refusal_text(ucanopen::SdoAbortCode code) const {
    const auto& description = *_server->description();
    if (auto key = description.find("settings.last_refusal"); key.has_value()) {
        if (auto item = description.resolve(*key); item.has_value() && item->values != nullptr) {
            if (const auto* value = item->values->find(static_cast<int64_t>(code)); value != nullptr) {
                return label_of(value->label, value->name);
            }
        }
    }
    if (auto message = ucanopen::sdo_abort_messages.find(code); message != ucanopen::sdo_abort_messages.end()) {
        return message->second;
    }
    return std::format("код 0x{:08X}", static_cast<uint32_t>(code));
}

std::string ServerSetupPanel::_failure(const SdoStep& step) const {
    switch (step.status) {
    case SdoStep::Status::refused:
        return "отказ: " + _refusal_text(step.abort_code);
    case SdoStep::Status::timed_out:
        return "нет ответа";
    case SdoStep::Status::cancelled:
        return "отменено";
    default:
        return {};
    }
}

bool ServerSetupPanel::_status_active(std::string_view name) const {
    return _server->status_service.active(name);
}

//----------------------------------------------------------------------------------------------------------------------
// Reads the members of the parameter shown, after `first`: a command or a
// write, whose effect the reading shows. The constants, once a connection.
void ServerSetupPanel::_read_parameter(std::vector<SdoStep> first) {
    const auto* record = _record();
    if (record == nullptr) {
        return;
    }
    const auto& description = *_server->description();

    bool const command = !first.empty();
    auto steps = std::move(first);
    for (uint8_t subindex : {requested, applied, stored, actual}) {
        if (has_member(description, *record, subindex)) {
            steps.push_back(SdoStep::read({record->index, subindex}));
        }
    }
    if (!_constants.contains(record->index)) {
        for (uint8_t subindex : {default_value, min_value, max_value, policy}) {
            if (has_member(description, *record, subindex)) {
                steps.push_back(SdoStep::read({record->index, subindex}));
            }
        }
    }

    if (!_server->config_service.start(std::move(steps))) {
        return;
    }
    _should_read = false;
    _transfer = command ? Transfer::command : Transfer::parameter;
}

void ServerSetupPanel::_command(std::string_view path) {
    auto key = _server->description()->find(path);
    if (!key.has_value()) {
        _result = "Описание словаря не называет команды " + std::string(path) + ".";
        _result_color = colors::icon_red;
        return;
    }
    _read_parameter({SdoStep::command(*key)});
}

void ServerSetupPanel::_command_member(uint8_t subindex) {
    const auto* record = _record();
    if (record == nullptr || !has_member(*_server->description(), *record, subindex)) {
        return;
    }
    _read_parameter({SdoStep::command({record->index, subindex})});
}

void ServerSetupPanel::_write_requested(ExpeditedSdoData value) {
    const auto* record = _record();
    if (record == nullptr) {
        return;
    }
    _read_parameter({SdoStep::write({record->index, requested}, value)});
}

void ServerSetupPanel::_read_all() {
    const auto& description = *_server->description();
    std::vector<Row> rows;
    std::vector<SdoStep> steps;

    // the parameters, then the objects of their class that only show the
    // state of the storage and the constants of the sensors
    for (const auto& group : _groups) {
        for (const auto* record : group.records) {
            auto item = description.resolve({record->index, requested});
            if (!item.has_value()) {
                continue;
            }
            rows.push_back({.record = record, .item = *item, .group = record->group, .name = record->name});
            for (uint8_t subindex : {requested, applied, stored}) {
                steps.push_back(SdoStep::read({record->index, subindex}));
            }
        }
    }
    for (const auto& [key, object] : description.objects()) {
        if (object.cls != od::Class::parameters) {
            continue;
        }
        auto item = description.resolve(key);
        if (!item.has_value() || !item->available || item->access == od::Access::wo) {
            continue;
        }
        rows.push_back({.record = nullptr, .item = *item, .group = object.group, .name = object.name});
        steps.push_back(SdoStep::read(key));
    }

    if (!_server->config_service.start(std::move(steps))) {
        return;
    }
    _rows = std::move(rows);
    _table = Table::values;
    _notes.clear();
    _hint.clear();
    _file_name.clear();
    _file_server.clear();
    _transfer = Transfer::read_all;
    bsclog::info("Чтение всех параметров {}...", _server->name());
}

void ServerSetupPanel::_load_file(const std::string& path) {
    auto contents = config_file::read(path);
    if (!contents.has_value()) {
        bsclog::error("Не удалось загрузить настройки из {}: {}.", path, contents.error());
        return;
    }

    std::string file_server;
    std::string file_firmware;
    for (const auto& [key, value] : contents->header) {
        if (key == "server") {
            file_server = value;
        } else if (key == "device.firmware_version") {
            file_firmware = value;
        }
    }

    std::vector<std::string> notes;
    if (!file_server.empty() && file_server != _server->name()) {
        notes.push_back("файл сохранён для " + file_server + ", а загружается в " + _server->name());
    }
    std::string device_firmware;
    for (const auto& entry : _server->info_service.info().entries) {
        if (entry.item.path == "device.firmware_version" && entry.value.has_value()) {
            device_firmware = *entry.value;
        }
    }
    if (!file_firmware.empty() && !device_firmware.empty() && file_firmware != device_firmware) {
        notes.push_back("файл сохранён с ПО " + file_firmware + ", в устройстве " + device_firmware);
    }
    for (auto& warning : contents->warnings) {
        notes.push_back(std::move(warning));
    }

    std::map<std::pair<std::string_view, std::string_view>, const config_file::Parameter*> in_file;
    for (const auto& parameter : contents->parameters) {
        in_file[{parameter.group, parameter.name}] = &parameter;
    }
    auto where = [](const config_file::Parameter& parameter) {
        return "строка " + std::to_string(parameter.line) + ": [" + parameter.group + "] " + parameter.name;
    };

    // The rows follow the description, as the file does when it was saved
    // here.
    const auto& description = *_server->description();
    std::vector<Row> rows;
    std::vector<SdoStep> steps;
    std::set<const config_file::Parameter*> matched;
    for (const auto& group : _groups) {
        for (const auto* record : group.records) {
            auto found = in_file.find({record->group, record->name});
            if (found == in_file.end()) {
                continue;
            }
            const auto& parameter = *found->second;
            matched.insert(&parameter);

            auto item = description.resolve({record->index, requested});
            if (!item.has_value() || item->access != od::Access::rw) {
                notes.push_back(where(parameter) + " не записывается, пропущен");
                continue;
            }
            auto value = config_file::parse_value(*item, parameter.value);
            if (!value.has_value()) {
                notes.push_back(where(parameter) + ": «" + parameter.value + "» — не значение параметра, пропущен");
                continue;
            }
            rows.push_back({.record = record,
                            .item = *item,
                            .group = record->group,
                            .name = record->name,
                            .file_value = value});
            steps.push_back(SdoStep::read({record->index, stored}));
        }
    }
    for (const auto& parameter : contents->parameters) {
        if (!matched.contains(&parameter)) {
            notes.push_back(where(parameter) + " нет среди параметров устройства, пропущен");
        }
    }

    if (!steps.empty() && !_server->config_service.start(std::move(steps))) {
        return;
    }
    _rows = std::move(rows);
    _table = Table::file;
    _notes = std::move(notes);
    _hint.clear();
    _file_name = std::filesystem::path(path).filename().string();
    _file_server = file_server;

    for (const auto& note : _notes) {
        bsclog::warning("{}: {}.", _file_name, note);
    }
    if (_rows.empty()) {
        bsclog::warning("В файле {} нет параметров, которые можно записать в {}.", _file_name, _server->name());
        return;
    }
    _transfer = Transfer::compare;
    bsclog::info("Сравнение {} с файлом {}...", _server->name(), _file_name);
}

// Writes the values picked as requested, applies them all together, as the
// rules that relate parameters ask, and reads back what was applied.
void ServerSetupPanel::_write_selected() {
    std::vector<SdoStep> steps;
    std::vector<size_t> written_rows;
    for (size_t i = 0; i < _rows.size(); ++i) {
        const auto& row = _rows[i];
        if (!row.selected || !row.differs()) {
            continue;
        }
        steps.push_back(SdoStep::write({row.record->index, requested}, *row.file_value));
        written_rows.push_back(i);
    }
    if (written_rows.empty()) {
        return;
    }

    if (auto apply_all = _server->description()->find("settings.apply_all"); apply_all.has_value()) {
        steps.push_back(SdoStep::command(*apply_all));
    }
    for (auto i : written_rows) {
        for (uint8_t subindex : {applied, policy}) {
            steps.push_back(SdoStep::read({_rows[i].record->index, subindex}));
        }
    }

    if (!_server->config_service.start(std::move(steps))) {
        return;
    }
    _written_rows = std::move(written_rows);
    _hint.clear();
    _transfer = Transfer::write;
    bsclog::info("Запись параметров из файла {} в {}: {}...", _file_name, _server->name(), _written_rows.size());
}

void ServerSetupPanel::_take_transfer() {
    if (_transfer == Transfer::none || _server->config_service.busy()) {
        return;
    }

    auto steps = _server->config_service.steps();
    auto outcome = _server->config_service.outcome();
    switch (std::exchange(_transfer, Transfer::none)) {
    case Transfer::parameter:
    case Transfer::command:
        _take_parameter(steps);
        break;
    case Transfer::read_all:
        _take_read_all(steps, outcome);
        break;
    case Transfer::compare:
        _take_compare(steps, outcome);
        break;
    case Transfer::write:
        _take_write(steps, outcome);
        break;
    case Transfer::none:
        break;
    }
}

void ServerSetupPanel::_take_parameter(const std::vector<SdoStep>& steps) {
    const auto& description = *_server->description();
    for (const auto& step : steps) {
        if (step.kind != SdoStep::Kind::read) {
            // the command or the write that went first
            auto item = description.resolve(step.key);
            auto const name = item.has_value() ? item->path : std::string("команда");
            if (step.status == SdoStep::Status::done) {
                _result = "Выполнено: " + name + ".";
                _result_color = colors::icon_green;
            } else {
                _result = "Не выполнено: " + name + ", " + _failure(step) + ".";
                _result_color = colors::icon_red;
            }
            continue;
        }
        bool const constant = step.key.subindex >= default_value && step.key.subindex <= policy;
        if (step.status == SdoStep::Status::done) {
            if (constant) {
                _constants[step.key.index][step.key.subindex] = step.value;
            } else if (const auto* record = _record(); record != nullptr && record->index == step.key.index) {
                _shown[step.key.subindex] = step.value;
            }
        } else if (!constant) {
            _shown.erase(step.key.subindex);
        }
    }
}

void ServerSetupPanel::_take_read_all(const std::vector<SdoStep>& steps, ucanopen::SdoSequence::Outcome outcome) {
    std::map<ucanopen::od::Key, const SdoStep*> by_key;
    for (const auto& step : steps) {
        by_key[step.key] = &step;
    }

    size_t read = 0;
    for (auto& row : _rows) {
        auto take = [&](ucanopen::od::Key key, std::optional<ExpeditedSdoData>& value) {
            auto step = by_key.find(key);
            if (step == by_key.end()) {
                return;
            }
            if (step->second->status == SdoStep::Status::done) {
                value = step->second->value;
            } else if (row.status.empty()) {
                row.status = _failure(*step->second);
                row.status_color = colors::table_bg_red;
            }
        };
        if (row.record != nullptr) {
            take({row.record->index, requested}, row.requested);
            take({row.record->index, applied}, row.applied);
            take({row.record->index, stored}, row.stored);
        } else {
            take(row.item.key, row.applied);
        }
        if (row.status.empty()) {
            ++read;
        }
    }

    using Outcome = ucanopen::SdoSequence::Outcome;
    auto const name = _server->name();
    if (outcome == Outcome::no_response) {
        bsclog::error("{} не отвечает, чтение прервано: прочитано {} из {}.", name, read, _rows.size());
    } else if (outcome == Outcome::cancelled) {
        bsclog::warning("Чтение параметров {} отменено: прочитано {} из {}.", name, read, _rows.size());
    } else if (read < _rows.size()) {
        bsclog::warning("Параметры {} прочитаны не все: {} из {}.", name, read, _rows.size());
    } else {
        bsclog::success("Параметры {} прочитаны: {} из {}.", name, read, _rows.size());
    }
}

void ServerSetupPanel::_take_compare(const std::vector<SdoStep>& steps, ucanopen::SdoSequence::Outcome outcome) {
    size_t differ = 0;
    size_t unknown = 0;
    bool manual = false;
    for (size_t i = 0; i < _rows.size() && i < steps.size(); ++i) {
        auto& row = _rows[i];
        if (steps[i].status != SdoStep::Status::done) {
            // Not picked: what kept the value from being read would likely
            // keep it from being written.
            row.status = "не прочитано (" + _failure(steps[i]) + ")";
            row.status_color = colors::table_bg_red;
            ++unknown;
            continue;
        }

        row.stored = steps[i].value;
        if (!row.differs()) {
            row.status = "совпадает";
            continue;
        }
        ++differ;
        if (row.group == manual_group) {
            manual = true;
        } else {
            row.selected = true;
        }
    }

    if (manual) {
        _notes.push_back("параметры [" + std::string(manual_group) +
                         "] отличаются, но не выбраны: это адрес узла, отметьте их вручную, если он нужен");
    }
    if (differ == 0 && unknown == 0) {
        _hint = "Сохранённые настройки устройства совпадают с файлом.";
    }

    using Outcome = ucanopen::SdoSequence::Outcome;
    auto const name = _server->name();
    if (outcome == Outcome::no_response) {
        bsclog::error("{} не отвечает, сравнение с файлом {} прервано.", name, _file_name);
    } else if (outcome == Outcome::cancelled) {
        bsclog::warning("Сравнение {} с файлом {} отменено.", name, _file_name);
    }
    bsclog::info("{} и файл {}: отличаются {} из {}, не прочитано {}.", name, _file_name, differ, _rows.size(), unknown);
}

void ServerSetupPanel::_take_write(const std::vector<SdoStep>& steps, ucanopen::SdoSequence::Outcome outcome) {
    std::map<std::pair<ucanopen::od::Key, SdoStep::Kind>, const SdoStep*> by_key;
    const SdoStep* apply_step = nullptr;
    for (const auto& step : steps) {
        if (step.kind == SdoStep::Kind::command) {
            apply_step = &step;
        } else {
            by_key[{step.key, step.kind}] = &step;
        }
    }
    bool const applied_all = (apply_step != nullptr) && (apply_step->status == SdoStep::Status::done);

    const auto& description = *_server->description();
    size_t written = 0;
    size_t failed = 0;
    bool restart = false;
    for (auto i : _written_rows) {
        auto& row = _rows[i];
        auto const index = row.record->index;
        auto write = by_key.find({{index, requested}, SdoStep::Kind::write});
        if (write == by_key.end() || write->second->status != SdoStep::Status::done) {
            // Left picked, to be tried again.
            if (write != by_key.end()) {
                row.status = _failure(*write->second);
                if (write->second->status != SdoStep::Status::cancelled) {
                    row.status_color = colors::table_bg_red;
                    ++failed;
                }
            }
            continue;
        }

        ++written;
        row.selected = false;
        auto read = [&](uint8_t subindex) -> std::optional<ExpeditedSdoData> {
            auto step = by_key.find({{index, subindex}, SdoStep::Kind::read});
            if (step == by_key.end() || step->second->status != SdoStep::Status::done) {
                return std::nullopt;
            }
            return step->second->value;
        };
        auto const applied_value = read(applied);
        if (auto const policy_value = read(policy); policy_value.has_value()) {
            _constants[index][policy] = *policy_value;
        }
        bool on_restart = false;
        auto const policy_item = description.resolve({index, policy});
        auto const policy_value = _constant(*row.record, policy);
        if (policy_item.has_value() && policy_item->values != nullptr && policy_value.has_value()) {
            const auto* named =
                    policy_item->values->find(static_cast<int64_t>(od::to_double(*policy_item, *policy_value)));
            on_restart = (named != nullptr) && (named->name == "on_restart");
        }

        if (on_restart) {
            row.status = "записано, вступит в силу после сохранения и перезапуска";
            row.status_color = colors::table_bg_yellow;
            restart = true;
        } else if (!applied_all) {
            row.status = "записано, не применено" +
                         (apply_step != nullptr ? ": " + _failure(*apply_step) : std::string{});
            row.status_color = colors::table_bg_yellow;
        } else if (applied_value.has_value() && config_file::same_value(row.item, *applied_value, *row.file_value)) {
            row.status = "применено";
            row.status_color = colors::table_bg_green;
        } else {
            row.status = "применено, сервер вернул " +
                         (applied_value.has_value() ? od::format_value(row.item, *applied_value) : std::string("н/д"));
            row.status_color = colors::table_bg_yellow;
        }
    }
    auto const total = _written_rows.size();
    _written_rows.clear();

    if (written > 0) {
        _hint = applied_all ? "Применённое хранится в рабочей памяти устройства: чтобы сохранить его, нажмите "
                              "«Сохранить»."
                            : "Записанное не применено: исправьте отклонённое или нажмите «Отбросить всё».";
        if (restart) {
            _hint += " Параметры «после перезапуска» вступят в силу после сохранения и перезапуска устройства.";
        }
    }

    using Outcome = ucanopen::SdoSequence::Outcome;
    auto const name = _server->name();
    if (outcome == Outcome::no_response) {
        bsclog::error("{} не отвечает, запись прервана: записано {} из {}.", name, written, total);
    } else if (outcome == Outcome::cancelled) {
        bsclog::warning("Запись в {} отменена: записано {} из {}.", name, written, total);
    } else if (failed > 0) {
        bsclog::warning("В {} записано параметров из файла {}: {} из {}.", name, _file_name, written, total);
    } else {
        bsclog::success("В {} записаны параметры из файла {}: {} из {}.", name, _file_name, written, total);
    }
}

void ServerSetupPanel::_open_save_dialog() {
    auto file_name = _server->name();
    for (const auto& entry : _server->info_service.info().entries) {
        if (entry.item.path == "device.mcu_uid" && entry.value.has_value()) {
            file_name += "_uid" + *entry.value;
        }
    }
    file_name += "_" + local_time("%Y-%m-%d") + ".ini";

    IGFD::FileDialogConfig config;
    config.path = ".";
    config.fileName = file_name;
    config.flags = ImGuiFileDialogFlags_Default;
    ImGuiFileDialog::Instance()->OpenDialog(_save_dialog_key, "Сохранить настройки", ".ini", config);
}

void ServerSetupPanel::_open_load_dialog() {
    IGFD::FileDialogConfig config;
    config.path = ".";
    config.flags = ImGuiFileDialogFlags_Modal | ImGuiFileDialogFlags_HideColumnType;
    ImGuiFileDialog::Instance()->OpenDialog(_load_dialog_key, "Загрузить настройки", ".ini,.*", config);
}

// A file holds the stored values: what the device starts with.
void ServerSetupPanel::_save_file(const std::string& path) {
    config_file::Header header = {{"server", _server->name()}};
    auto const info = _server->info_service.info();
    for (const auto& entry : info.entries) {
        if (entry.item.path == "dictionary.fingerprint" && info.fingerprint.has_value()) {
            header.emplace_back(entry.item.path, std::format("0x{:08X}", *info.fingerprint));
        } else {
            header.emplace_back(entry.item.path, entry.value.value_or("n/a"));
        }
    }
    header.emplace_back("saved", local_time("%Y-%m-%d %H:%M:%S"));

    std::vector<config_file::Value> values;
    for (const auto& row : _rows) {
        if (row.record != nullptr) {
            values.push_back({row.group, row.name, row.item, true, row.stored});
        } else {
            values.push_back({row.group, row.name, row.item, false, row.applied});
        }
    }

    if (auto result = config_file::write(path, header, values); !result) {
        bsclog::error("Не удалось сохранить настройки {} в {}: {}.", _server->name(), path, result.error());
        return;
    }
    bsclog::success("Настройки {} сохранены в {}.", _server->name(), path);
}

} // namespace ui
