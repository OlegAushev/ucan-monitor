#include "serversetuppanel.h"
#include <config_file/config_file.h>
#include <ui/util/style.h>
#include <ui/util/util.h>

#include <imguifiledialog/ImGuiFileDialog.h>

#include <algorithm>
#include <array>
#include <ctime>
#include <filesystem>
#include <map>
#include <set>
#include <string_view>
#include <utility>

namespace ui {

namespace {

std::string local_time(const char* format) {
    auto now = std::time(nullptr);
    std::tm tm{};
    localtime_r(&now, &tm);
    std::array<char, 64> buf;
    auto size = std::strftime(buf.data(), buf.size(), format, &tm);
    return std::string(buf.data(), size);
}

ucanopen::ConfigStep read_step(ucanopen::ODEntryIter entry) {
    return {.entry = entry};
}

std::string failure(const ucanopen::ConfigStep& step) {
    switch (step.status) {
    case ucanopen::ConfigStep::Status::refused: {
        auto message = ucanopen::sdo_abort_messages.find(step.abort_code);
        if (message == ucanopen::sdo_abort_messages.end()) {
            return "отказ";
        }
        return "отказ: " + message->second;
    }
    case ucanopen::ConfigStep::Status::timed_out:
        return "нет ответа";
    case ucanopen::ConfigStep::Status::cancelled:
        return "отменено";
    default:
        return {};
    }
}

std::string_view type_name(ucanopen::ODObjectDataType type) {
    constexpr std::array<std::string_view, 10> names = {
            "bool", "int8", "int16", "int32", "uint8",
            "uint16", "uint32", "float32", "exec", "string"};
    return names[type];
}

// The node's own ID and the COB-IDs it takes its RPDOs from: loaded from
// another device's file, they would move this device elsewhere on the bus, so
// they are loaded only when picked by hand.
constexpr std::string_view manual_subcategory = "ucanopen";

} // namespace

ServerSetupPanel::ServerSetupPanel(std::shared_ptr<ucanopen::Server> server,
                                   const std::string& menu_title,
                                   const std::string& window_title,
                                   bool open)
        : View(menu_title, window_title, open),
          _server(server),
          _save_dialog_key("config_save_" + server->name()),
          _load_dialog_key("config_load_" + server->name()) {}

bool ServerSetupPanel::Row::differs() const {
    if (!file_value.has_value()) {
        return false;
    }
    return !server_value.has_value() ||
           !config_file::same_value(entry->second, *server_value, *file_value);
}

void ServerSetupPanel::draw() {
    ImGui::Begin(_window_title.c_str(), &_opened);

    _take_transfer();
    _draw_about();
    _draw_setup();
    _draw_all_parameters();
    _draw_popups();
    _draw_dialogs();

    ImGui::End();
}

void ServerSetupPanel::_draw_about() {
    ImGui::SeparatorText("О сервере");
    if (ImGui::Button(ICON_MDI_REFRESH " Обновить##about")) {
        _device_name.clear();
        _hardware_version.clear();
        _software_version.clear();
        _software_commitdate.clear();
        _software_branch.clear();
        _device_sn.clear();
    }

    ImGui::TextDisabled("Устройство: ");
    ImGui::SameLine();
    if (_device_name.empty()) {
        _device_name = _server->read_string("info",
                                            "sys",
                                            "device_name",
                                            std::chrono::milliseconds(500))
                               .value_or("n/a");
    }
    ImGui::TextUnformatted(_device_name.c_str());

    ImGui::TextDisabled("Версия АО: ");
    ImGui::SameLine();
    if (_hardware_version.empty()) {
        _hardware_version = _server->read_string("info",
                                                 "sys",
                                                 "hardware_version",
                                                 std::chrono::milliseconds(500))
                                    .value_or("n/a");
    }
    ImGui::TextUnformatted(_hardware_version.c_str());

    ImGui::TextDisabled("Версия ПО: ");
    ImGui::SameLine();
    if (_software_version.empty()) {
        _software_version = _server->read_string("info",
                                                 "sys",
                                                 "firmware_version",
                                                 std::chrono::milliseconds(500))
                                    .value_or("n/a");
    }
    ImGui::TextUnformatted(_software_version.c_str());

    ImGui::TextDisabled("Дата ПО: ");
    ImGui::SameLine();
    if (_software_commitdate.empty()) {
        _software_commitdate =
                _server->read_string("info",
                                     "sys",
                                     "firmware_commitdate",
                                     std::chrono::milliseconds(500))
                        .value_or("n/a");
    }
    ImGui::TextUnformatted(_software_commitdate.c_str());

    ImGui::TextDisabled("Ветка ПО: ");
    ImGui::SameLine();
    if (_software_branch.empty()) {
        _software_branch = _server->read_string("info",
                                                "sys",
                                                "firmware_branch",
                                                std::chrono::milliseconds(500))
                                   .value_or("n/a");
    }
    ImGui::TextUnformatted(_software_branch.c_str());

    ImGui::TextDisabled("Номер Устройства: ");
    ImGui::SameLine();
    if (_device_sn.empty()) {
        _device_sn = _server->read_scalar("info",
                                          "sys",
                                          "serial_number",
                                          std::chrono::milliseconds(500))
                             .value_or("n/a");
    }
    ImGui::TextUnformatted(_device_sn.c_str());
}

void ServerSetupPanel::_draw_setup() {
    ImGui::SeparatorText("Настройка");

    const auto& objects = _server->config_service.objects();
    if (objects.empty()) {
        return;
    }

    // A transfer would overwrite a parameter set by hand meanwhile, and
    // «Применить» would store it half done.
    util::DisableGuard disabled(_busy());

    auto selected_category_iter = objects.find(_category);
    if (selected_category_iter == objects.end()) {
        selected_category_iter = objects.begin();
        _category = selected_category_iter->first;
    }

    if (ImGui::Button(ICON_MDI_REFRESH " Обновить##setup")) {
        _should_read = true;
    }

    if (ImGui::BeginCombo("Категория", selected_category_iter->first.data())) {
        for (auto iter = objects.begin(); iter != objects.end(); ++iter) {
            auto is_selected = (iter == selected_category_iter);
            if (ImGui::Selectable(iter->first.data(), is_selected)) {
                selected_category_iter = iter;
                _category = iter->first;
                _selected_object_idx = 0;
                _should_read = true;
            }
        }
        ImGui::EndCombo();
    }

    const std::string object_preview =
            selected_category_iter->second[_selected_object_idx]->name + "[" +
            selected_category_iter->second[_selected_object_idx]->unit + "]";

    if (ImGui::BeginCombo("Объект", object_preview.c_str())) {
        for (size_t i = 0; i < selected_category_iter->second.size(); ++i) {
            auto is_selected = (i == _selected_object_idx);
            const auto& obj = selected_category_iter->second[i];
            const std::string obj_ = obj->name + "[" + obj->unit + "]";
            if (ImGui::Selectable(obj_.c_str(), is_selected)) {
                _selected_object_idx = i;
                _should_read = true;
            }
        }
        ImGui::EndCombo();
    }

    if (_should_read) {
        _parameter_value = _server->read_expdata(
                _server->dictionary().config.config_category,
                selected_category_iter->first,
                selected_category_iter->second[_selected_object_idx]->name,
                std::chrono::milliseconds(500));
        _should_read = false;
    }

    if (!_parameter_value.has_value()) {
        std::string str = "н/д";
        ImGui::InputText("Значение",
                         str.data(),
                         3,
                         ImGuiInputTextFlags_ReadOnly);
    } else {

        switch (selected_category_iter->second[_selected_object_idx]
                        ->data_type) {
        case ucanopen::OD_BOOL:
        case ucanopen::OD_UINT8: {
            uint8_t value_u8 = _parameter_value.value().u8();
            if (ImGui::InputScalar("Значение",
                                   ImGuiDataType_U8,
                                   &value_u8,
                                   NULL,
                                   NULL,
                                   NULL,
                                   ImGuiInputTextFlags_EnterReturnsTrue)) {
                _server->write(
                        _server->dictionary().config.config_category,
                        selected_category_iter->first,
                        selected_category_iter->second[_selected_object_idx]
                                ->name,
                        ucanopen::ExpeditedSdoData(uint8_t(value_u8)));
                _should_read = true;
            }
            break;
        }

        case ucanopen::OD_UINT16: {
            uint16_t value_u16 = _parameter_value.value().u16();
            if (ImGui::InputScalar("Значение",
                                   ImGuiDataType_U16,
                                   &value_u16,
                                   NULL,
                                   NULL,
                                   NULL,
                                   ImGuiInputTextFlags_EnterReturnsTrue)) {
                _server->write(
                        _server->dictionary().config.config_category,
                        selected_category_iter->first,
                        selected_category_iter->second[_selected_object_idx]
                                ->name,
                        ucanopen::ExpeditedSdoData(uint16_t(value_u16)));
                _should_read = true;
            }
            break;
        }

        case ucanopen::OD_UINT32: {
            uint32_t value_u32 = _parameter_value.value().u32();
            char const* format{NULL};
            ImGuiInputTextFlags flags{ImGuiInputTextFlags_EnterReturnsTrue};
            if (selected_category_iter->second[_selected_object_idx]->unit ==
                "hex") {
                format = "%08X";
                flags |= ImGuiInputTextFlags_CharsHexadecimal;
            }
            if (ImGui::InputScalar("Значение",
                                   ImGuiDataType_U32,
                                   &value_u32,
                                   NULL,
                                   NULL,
                                   format,
                                   flags)) {
                _server->write(
                        _server->dictionary().config.config_category,
                        selected_category_iter->first,
                        selected_category_iter->second[_selected_object_idx]
                                ->name,
                        ucanopen::ExpeditedSdoData(uint32_t(value_u32)));
                _should_read = true;
            }
            break;
        }

        case ucanopen::OD_INT8: {
            int16_t value_i8 = _parameter_value.value().i8();
            if (ImGui::InputScalar("Значение",
                                   ImGuiDataType_S8,
                                   &value_i8,
                                   NULL,
                                   NULL,
                                   "%d",
                                   ImGuiInputTextFlags_EnterReturnsTrue)) {
                _server->write(
                        _server->dictionary().config.config_category,
                        selected_category_iter->first,
                        selected_category_iter->second[_selected_object_idx]
                                ->name,
                        ucanopen::ExpeditedSdoData(int8_t(value_i8)));
                _should_read = true;
            }
            break;
        }

        case ucanopen::OD_INT16: {
            int16_t value_i16 = _parameter_value.value().i16();
            if (ImGui::InputScalar("Значение",
                                   ImGuiDataType_S16,
                                   &value_i16,
                                   NULL,
                                   NULL,
                                   "%d",
                                   ImGuiInputTextFlags_EnterReturnsTrue)) {
                _server->write(
                        _server->dictionary().config.config_category,
                        selected_category_iter->first,
                        selected_category_iter->second[_selected_object_idx]
                                ->name,
                        ucanopen::ExpeditedSdoData(int16_t(value_i16)));
                _should_read = true;
            }
            break;
        }

        case ucanopen::OD_INT32: {
            int32_t value_i32 = _parameter_value.value().i32();
            if (ImGui::InputScalar("Значение",
                                   ImGuiDataType_S32,
                                   &value_i32,
                                   NULL,
                                   NULL,
                                   NULL,
                                   ImGuiInputTextFlags_EnterReturnsTrue)) {
                _server->write(
                        _server->dictionary().config.config_category,
                        selected_category_iter->first,
                        selected_category_iter->second[_selected_object_idx]
                                ->name,
                        ucanopen::ExpeditedSdoData(int32_t(value_i32)));
                _should_read = true;
            }
            break;
        }

        case ucanopen::OD_FLOAT32: {
            float value_f32 = _parameter_value.value().f32();
            if (ImGui::InputScalar("Значение",
                                   ImGuiDataType_Float,
                                   &value_f32,
                                   NULL,
                                   NULL,
                                   "%.6f",
                                   ImGuiInputTextFlags_EnterReturnsTrue)) {
                _server->write(
                        _server->dictionary().config.config_category,
                        selected_category_iter->first,
                        selected_category_iter->second[_selected_object_idx]
                                ->name,
                        ucanopen::ExpeditedSdoData(float(value_f32)));
                _should_read = true;
            }
            break;
        }

        default:
            break;
        }
    }

    ImGui::NewLine();

    if (ImGui::Button("Восстановить", ImVec2(-1.0f, 0))) {
        _server->sdo_service.restore_default_parameter(
                _server->dictionary().config.config_category,
                selected_category_iter->first,
                selected_category_iter->second[_selected_object_idx]->name);
        _should_read = true;
    }

    if (ImGui::Button("Восстановить Всё", ImVec2(-1.0f, 0))) {
        ImGui::OpenPopup("Внимание!##restore");
    }

    if (ImGui::Button("Применить", ImVec2(-1.0f, 0))) {
        ImGui::OpenPopup("Внимание!##apply");
    }

    if (ImGui::Button("Применить и Перезапустить", ImVec2(-1.0f, 0))) {
        ImGui::OpenPopup("Внимание!##apply_and_reset");
    }

    if (ImGui::Button("Очистить Память", ImVec2(-1.0f, 0))) {
        ImGui::OpenPopup("Внимание!##erase");
    }
}

void ServerSetupPanel::_draw_all_parameters() {
    auto& config = _server->config_service;
    if (config.entries().empty()) {
        return;
    }

    ImGui::SeparatorText("Все параметры");

    {
        util::DisableGuard disabled(_busy());
        if (ImGui::Button(ICON_MDI_TABLE_REFRESH " Прочитать всё",
                          ImVec2(-1.0f, 0))) {
            _read_all();
        }

        // Only a table of all the values makes a whole file.
        bool const nothing_to_save =
                (_table != Table::values) ||
                std::none_of(_rows.begin(), _rows.end(), [](const auto& row) {
                    return row.server_value.has_value();
                });
        {
            util::DisableGuard save_disabled(nothing_to_save);
            if (ImGui::Button(ICON_MDI_CONTENT_SAVE_OUTLINE
                              " Сохранить в файл...",
                              ImVec2(-1.0f, 0))) {
                _open_save_dialog();
            }
        }

        if (ImGui::Button(ICON_MDI_FOLDER_OPEN_OUTLINE " Загрузить из файла...",
                          ImVec2(-1.0f, 0))) {
            _open_load_dialog();
        }
    }

    if (_table == Table::file) {
        auto const selected = _selected_count();
        util::DisableGuard disabled(_busy() || selected == 0);
        auto label = std::string(ICON_MDI_UPLOAD " Записать выбранные (") +
                     std::to_string(selected) + ")###write_file";
        if (ImGui::Button(label.c_str(), ImVec2(-1.0f, 0))) {
            ImGui::OpenPopup("Внимание!##write_file");
        }
    }

    if (config.busy()) {
        auto [done, total] = config.progress();
        auto overlay = std::to_string(done) + " / " + std::to_string(total);
        const char* cancel_label = ICON_MDI_CLOSE " Отмена##transfer";
        float cancel_width = ImGui::CalcTextSize(cancel_label, nullptr, true).x +
                             2 * ImGui::GetStyle().FramePadding.x;
        ImGui::ProgressBar(
                total ? float(done) / float(total) : 0.0f,
                ImVec2(-(cancel_width + ImGui::GetStyle().ItemSpacing.x), 0),
                overlay.c_str());
        ImGui::SameLine();
        if (ImGui::Button(cancel_label)) {
            config.cancel();
        }
    }

    if (!_notes.empty()) {
        auto label = "Замечания к файлу " + _file_name + " (" +
                     std::to_string(_notes.size()) + ")###notes";
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

    constexpr ImGuiTableFlags flags =
            ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg |
            ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV |
            ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingFixedFit;
    float const min_height = 10 * ImGui::GetFrameHeightWithSpacing();
    ImVec2 const size(0.0f,
                      std::max(ImGui::GetContentRegionAvail().y, min_height));
    if (!ImGui::BeginTable("##parameters", 4, flags, size)) {
        return;
    }

    ImGui::TableSetupScrollFreeze(0, 1);
    ImGui::TableSetupColumn("Подкатегория");
    ImGui::TableSetupColumn("Параметр");
    ImGui::TableSetupColumn("Значение");
    ImGui::TableSetupColumn("Ед.", ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableHeadersRow();

    for (const auto& row : _rows) {
        const auto& object = row.entry->second;

        // Dimmed as they are commented out in the file: there to be seen,
        // not loaded.
        bool const read_only = !object.has_write_permission();
        if (read_only) {
            ImGui::PushStyleColor(ImGuiCol_Text,
                                  ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
        }

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(object.subcategory.c_str());
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(object.name.c_str());
        bool const name_hovered = ImGui::IsItemHovered();
        ImGui::TableNextColumn();
        if (row.server_value.has_value()) {
            auto value = config_file::format_value(object, *row.server_value);
            ImGui::TextUnformatted(value.c_str());
        } else if (!row.status.empty()) {
            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, row.status_color);
            ImGui::TextUnformatted(row.status.c_str());
        }
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(object.unit.c_str());

        if (read_only) {
            ImGui::PopStyleColor();
            if (name_hovered) {
                ImGui::SetTooltip("Только для чтения");
            }
        }
    }

    ImGui::EndTable();
}

void ServerSetupPanel::_draw_file_table() {
    if (_rows.empty()) {
        return;
    }

    constexpr ImGuiTableFlags flags =
            ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg |
            ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV |
            ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingFixedFit;
    float const min_height = 10 * ImGui::GetFrameHeightWithSpacing();
    ImVec2 const size(0.0f,
                      std::max(ImGui::GetContentRegionAvail().y, min_height));
    if (!ImGui::BeginTable("##file", 7, flags, size)) {
        return;
    }

    ImGui::TableSetupScrollFreeze(0, 1);
    ImGui::TableSetupColumn("##selected");
    ImGui::TableSetupColumn("Подкатегория");
    ImGui::TableSetupColumn("Параметр");
    ImGui::TableSetupColumn("В устройстве");
    ImGui::TableSetupColumn("В файле");
    ImGui::TableSetupColumn("Ед.");
    ImGui::TableSetupColumn("Статус", ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableHeadersRow();

    bool const busy = _busy();
    for (size_t i = 0; i < _rows.size(); ++i) {
        auto& row = _rows[i];
        const auto& object = row.entry->second;
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
        ImGui::TextUnformatted(object.subcategory.c_str());
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(object.name.c_str());
        ImGui::TableNextColumn();
        if (row.server_value.has_value()) {
            auto value = config_file::format_value(object, *row.server_value);
            ImGui::TextUnformatted(value.c_str());
        } else {
            ImGui::TextDisabled("н/д");
        }
        ImGui::TableNextColumn();
        if (differs && row.server_value.has_value()) {
            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg,
                                   colors::table_bg_yellow);
        }
        auto value = config_file::format_value(object, *row.file_value);
        ImGui::TextUnformatted(value.c_str());
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(object.unit.c_str());
        ImGui::TableNextColumn();
        if (row.status_color != 0) {
            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, row.status_color);
        }
        ImGui::TextUnformatted(row.status.c_str());
    }

    ImGui::EndTable();
}

void ServerSetupPanel::_draw_popups() {
    // Always center this window when appearing
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Внимание!##restore",
                               NULL,
                               ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Будут применены настройки по умолчанию. Продолжить?");
        ImGui::Separator();

        if (ImGui::Button(ICON_MDI_CANCEL " Нет", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button(ICON_MDI_CHECK " Да", ImVec2(120, 0))) {
            _server->exec("ctl", "sys", "restore_all_default_parameters");
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal("Внимание!##apply",
                               NULL,
                               ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Настройки будут записаны. Продолжить?");
        ImGui::Separator();

        if (ImGui::Button(ICON_MDI_CANCEL " Нет", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button(ICON_MDI_CHECK " Да", ImVec2(120, 0))) {
            _server->exec("ctl", "sys", "save_all_parameters");
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal("Внимание!##apply_and_reset",
                               NULL,
                               ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Настройки будут записаны, устройство перезапустится. "
                    "Продолжить?");
        ImGui::Separator();

        if (ImGui::Button(ICON_MDI_CANCEL " Нет", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button(ICON_MDI_CHECK " Да", ImVec2(120, 0))) {
            _server->exec("ctl", "sys", "save_all_parameters_and_reset");
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal("Внимание!##erase",
                               NULL,
                               ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Энергонезависимая память будет очищена. Продолжить?");
        ImGui::Separator();

        if (ImGui::Button(ICON_MDI_CANCEL " Нет", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button(ICON_MDI_CHECK " Да", ImVec2(120, 0))) {
            _server->exec("ctl", "sys", "erase_all_parameters");
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("Внимание!##write_file",
                               NULL,
                               ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Будет записано параметров из файла: %zu. Продолжить?",
                    _selected_count());
        if (!_file_server.empty() && _file_server != _server->name()) {
            ImGui::PushStyleColor(ImGuiCol_Text, colors::icon_yellow);
            ImGui::Text("Файл сохранён для %s, а не для %s.",
                        _file_server.c_str(),
                        _server->name().c_str());
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

bool ServerSetupPanel::_busy() const {
    return _transfer != Transfer::none || _server->config_service.busy();
}

size_t ServerSetupPanel::_selected_count() const {
    return std::count_if(_rows.begin(), _rows.end(), [](const auto& row) {
        return row.selected && row.differs();
    });
}

void ServerSetupPanel::_read_all() {
    std::vector<Row> rows;
    std::vector<ucanopen::ConfigStep> steps;
    for (auto entry : _server->config_service.entries()) {
        if (entry->second.has_read_permission()) {
            rows.push_back({.entry = entry});
            steps.push_back(read_step(entry));
        }
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
        bsclog::error("Не удалось загрузить настройки из {}: {}.",
                      path,
                      contents.error());
        return;
    }

    std::string file_server;
    std::string file_firmware;
    for (const auto& [key, value] : contents->header) {
        if (key == "server") {
            file_server = value;
        } else if (key == "firmware_version") {
            file_firmware = value;
        }
    }

    std::vector<std::string> notes;
    if (!file_server.empty() && file_server != _server->name()) {
        notes.push_back("файл сохранён для " + file_server +
                        ", а загружается в " + _server->name());
    }
    auto known = [](const std::string& text) {
        return !text.empty() && text != "n/a";
    };
    if (known(file_firmware) && known(_software_version) &&
        file_firmware != _software_version) {
        notes.push_back("файл сохранён с ПО " + file_firmware +
                        ", в устройстве " + _software_version);
    }
    for (auto& warning : contents->warnings) {
        notes.push_back(std::move(warning));
    }

    std::map<std::pair<std::string_view, std::string_view>,
             const config_file::Parameter*>
            in_file;
    for (const auto& parameter : contents->parameters) {
        in_file[{parameter.subcategory, parameter.name}] = &parameter;
    }
    auto where = [](const config_file::Parameter& parameter) {
        return "строка " + std::to_string(parameter.line) + ": [" +
               parameter.subcategory + "] " + parameter.name;
    };

    // The rows follow the dictionary, as the file does when it was saved here.
    std::vector<Row> rows;
    std::vector<ucanopen::ConfigStep> steps;
    std::set<const config_file::Parameter*> matched;
    for (auto entry : _server->config_service.entries()) {
        const auto& object = entry->second;
        auto found = in_file.find({object.subcategory, object.name});
        if (found == in_file.end()) {
            continue;
        }
        const auto& parameter = *found->second;
        matched.insert(&parameter);

        if (!object.has_write_permission() || !object.has_read_permission()) {
            notes.push_back(where(parameter) + " не записывается, пропущен");
            continue;
        }
        auto value = config_file::parse_value(object, parameter.value);
        if (!value.has_value()) {
            notes.push_back(where(parameter) + ": «" + parameter.value +
                            "» не " + std::string(type_name(object.data_type)) +
                            ", пропущен");
            continue;
        }
        rows.push_back({.entry = entry, .file_value = value});
        steps.push_back(read_step(entry));
    }
    for (const auto& parameter : contents->parameters) {
        if (!matched.contains(&parameter)) {
            notes.push_back(where(parameter) + " нет в словаре, пропущен");
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
        bsclog::warning("В файле {} нет параметров, которые можно записать в {}.",
                        _file_name,
                        _server->name());
        return;
    }
    _transfer = Transfer::compare;
    bsclog::info("Сравнение {} с файлом {}...", _server->name(), _file_name);
}

void ServerSetupPanel::_write_selected() {
    std::vector<ucanopen::ConfigStep> steps;
    std::vector<size_t> written_rows;
    for (size_t i = 0; i < _rows.size(); ++i) {
        const auto& row = _rows[i];
        if (!row.selected || !row.differs()) {
            continue;
        }
        steps.push_back({.entry = row.entry, .write_value = row.file_value});
        steps.push_back(read_step(row.entry)); // what the server made of it
        written_rows.push_back(i);
    }
    if (written_rows.empty()) {
        return;
    }

    // Whether what was taken needs a restart, where the server can tell.
    auto restart = _server->find_od_entry(
            _server->dictionary().config.config_category,
            "nvm",
            "restart_required");
    if (restart != _server->dictionary().entries.end() &&
        restart->second.has_read_permission()) {
        steps.push_back(read_step(restart));
    }

    if (!_server->config_service.start(std::move(steps))) {
        return;
    }
    _written_rows = std::move(written_rows);
    _hint.clear();
    _transfer = Transfer::write;
    bsclog::info("Запись параметров из файла {} в {}: {}...",
                 _file_name,
                 _server->name(),
                 _written_rows.size());
}

void ServerSetupPanel::_take_transfer() {
    if (_transfer == Transfer::none || _server->config_service.busy()) {
        return;
    }

    auto steps = _server->config_service.steps();
    auto outcome = _server->config_service.outcome();
    switch (std::exchange(_transfer, Transfer::none)) {
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

void ServerSetupPanel::_take_read_all(
        const std::vector<ucanopen::ConfigStep>& steps,
        ucanopen::ServerConfigService::Outcome outcome) {
    size_t read = 0;
    for (size_t i = 0; i < _rows.size() && i < steps.size(); ++i) {
        auto& row = _rows[i];
        if (steps[i].status == ucanopen::ConfigStep::Status::done) {
            row.server_value = steps[i].value;
            ++read;
        } else {
            row.status = failure(steps[i]);
            row.status_color = colors::table_bg_red;
        }
    }

    using Outcome = ucanopen::ServerConfigService::Outcome;
    auto const name = _server->name();
    if (outcome == Outcome::no_response) {
        bsclog::error("{} не отвечает, чтение прервано: прочитано {} из {}.",
                      name, read, steps.size());
    } else if (outcome == Outcome::cancelled) {
        bsclog::warning("Чтение параметров {} отменено: прочитано {} из {}.",
                        name, read, steps.size());
    } else if (read < steps.size()) {
        bsclog::warning("Параметры {} прочитаны не все: {} из {}.",
                        name, read, steps.size());
    } else {
        bsclog::success("Параметры {} прочитаны: {} из {}.",
                        name, read, steps.size());
    }
}

void ServerSetupPanel::_take_compare(
        const std::vector<ucanopen::ConfigStep>& steps,
        ucanopen::ServerConfigService::Outcome outcome) {
    size_t differ = 0;
    size_t unknown = 0;
    bool manual = false;
    for (size_t i = 0; i < _rows.size() && i < steps.size(); ++i) {
        auto& row = _rows[i];
        if (steps[i].status != ucanopen::ConfigStep::Status::done) {
            // Not picked: what kept the value from being read would likely
            // keep it from being written.
            row.status = "не прочитано (" + failure(steps[i]) + ")";
            row.status_color = colors::table_bg_red;
            ++unknown;
            continue;
        }

        row.server_value = steps[i].value;
        if (!row.differs()) {
            row.status = "совпадает";
            continue;
        }
        ++differ;
        if (row.entry->second.subcategory == manual_subcategory) {
            manual = true;
        } else {
            row.selected = true;
        }
    }

    if (manual) {
        _notes.push_back("параметры [" + std::string(manual_subcategory) +
                         "] отличаются, но не выбраны: это адрес узла и "
                         "RPDO, отметьте их вручную, если они нужны");
    }
    if (differ == 0 && unknown == 0) {
        _hint = "Настройки в устройстве совпадают с файлом.";
    }

    using Outcome = ucanopen::ServerConfigService::Outcome;
    auto const name = _server->name();
    if (outcome == Outcome::no_response) {
        bsclog::error("{} не отвечает, сравнение с файлом {} прервано.",
                      name, _file_name);
    } else if (outcome == Outcome::cancelled) {
        bsclog::warning("Сравнение {} с файлом {} отменено.", name, _file_name);
    }
    bsclog::info("{} и файл {}: отличаются {} из {}, не прочитано {}.",
                 name, _file_name, differ, _rows.size(), unknown);
}

void ServerSetupPanel::_take_write(
        const std::vector<ucanopen::ConfigStep>& steps,
        ucanopen::ServerConfigService::Outcome outcome) {
    using Status = ucanopen::ConfigStep::Status;
    size_t written = 0;
    size_t failed = 0;
    for (size_t k = 0; k < _written_rows.size() && 2 * k + 1 < steps.size(); ++k) {
        auto& row = _rows[_written_rows[k]];
        const auto& object = row.entry->second;
        const auto& write = steps[2 * k];
        const auto& check = steps[2 * k + 1];

        if (write.status != Status::done) {
            // Left picked, to be tried again.
            row.status = failure(write);
            if (write.status != Status::cancelled) {
                row.status_color = colors::table_bg_red;
                ++failed;
            }
            continue;
        }

        ++written;
        row.selected = false;
        if (check.status != Status::done) {
            row.status = "записано, не проверено";
            row.status_color = colors::table_bg_yellow;
            continue;
        }
        row.server_value = check.value;
        if (row.differs()) {
            // clamped, or kept in other units
            row.status = "записано, сервер вернул " +
                         config_file::format_value(object, check.value);
            row.status_color = colors::table_bg_yellow;
        } else {
            row.status = "записано";
            row.status_color = colors::table_bg_green;
        }
    }

    bool restart_required = false;
    if (steps.size() == 2 * _written_rows.size() + 1) {
        const auto& restart = steps.back();
        restart_required = (restart.status == Status::done) &&
                           (restart.value.u8() != 0);
    }
    auto const total = _written_rows.size();
    _written_rows.clear();

    if (written > 0) {
        _hint = "Записанное хранится в рабочей памяти устройства: чтобы "
                "сохранить его, нажмите «Применить».";
        if (restart_required) {
            _hint += " В силу изменения вступят после перезапуска устройства.";
        }
    }

    using Outcome = ucanopen::ServerConfigService::Outcome;
    auto const name = _server->name();
    if (outcome == Outcome::no_response) {
        bsclog::error("{} не отвечает, запись прервана: записано {} из {}.",
                      name, written, total);
    } else if (outcome == Outcome::cancelled) {
        bsclog::warning("Запись в {} отменена: записано {} из {}.",
                        name, written, total);
    } else if (failed > 0) {
        bsclog::warning("В {} записано параметров из файла {}: {} из {}.",
                        name, _file_name, written, total);
    } else {
        bsclog::success("В {} записаны параметры из файла {}: {} из {}.",
                        name, _file_name, written, total);
    }
}

void ServerSetupPanel::_open_save_dialog() {
    auto file_name = _server->name();
    if (!_device_sn.empty() && _device_sn != "n/a") {
        file_name += "_sn" + _device_sn;
    }
    file_name += "_" + local_time("%Y-%m-%d") + ".ini";

    IGFD::FileDialogConfig config;
    config.path = ".";
    config.fileName = file_name;
    config.flags = ImGuiFileDialogFlags_Default;
    ImGuiFileDialog::Instance()->OpenDialog(
            _save_dialog_key, "Сохранить настройки", ".ini", config);
}

void ServerSetupPanel::_open_load_dialog() {
    IGFD::FileDialogConfig config;
    config.path = ".";
    config.flags = ImGuiFileDialogFlags_Modal | ImGuiFileDialogFlags_HideColumnType;
    ImGuiFileDialog::Instance()->OpenDialog(
            _load_dialog_key, "Загрузить настройки", ".ini,.*", config);
}

void ServerSetupPanel::_save_file(const std::string& path) {
    config_file::Header header = {
            {"server", _server->name()},
            {"device_name", _device_name},
            {"hardware_version", _hardware_version},
            {"firmware_version", _software_version},
            {"firmware_commitdate", _software_commitdate},
            {"firmware_branch", _software_branch},
            {"serial_number", _device_sn},
            {"saved", local_time("%Y-%m-%d %H:%M:%S")}};

    std::vector<config_file::Value> values;
    for (const auto& row : _rows) {
        values.push_back({&row.entry->second, row.server_value});
    }

    if (auto result = config_file::write(path, header, values); !result) {
        bsclog::error("Не удалось сохранить настройки {} в {}: {}.",
                      _server->name(),
                      path,
                      result.error());
        return;
    }
    bsclog::success("Настройки {} сохранены в {}.", _server->name(), path);
}

} // namespace ui
