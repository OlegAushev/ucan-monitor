#include "serversetuppanel.h"
#include <config_file/config_file.h>
#include <ui/util/style.h>
#include <ui/util/util.h>

#include <imguifiledialog/ImGuiFileDialog.h>

#include <algorithm>
#include <array>
#include <ctime>
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

} // namespace

ServerSetupPanel::ServerSetupPanel(std::shared_ptr<ucanopen::Server> server,
                                   const std::string& menu_title,
                                   const std::string& window_title,
                                   bool open)
        : View(menu_title, window_title, open),
          _server(server),
          _save_dialog_key("config_save_" + server->name()) {}

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

        bool const nothing_read =
                std::none_of(_rows.begin(), _rows.end(), [](const auto& row) {
                    return row.server_value.has_value();
                });
        util::DisableGuard nothing_to_save(nothing_read);
        if (ImGui::Button(ICON_MDI_CONTENT_SAVE_OUTLINE " Сохранить в файл...",
                          ImVec2(-1.0f, 0))) {
            _open_save_dialog();
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

    _draw_table();
}

void ServerSetupPanel::_draw_table() {
    if (_rows.empty()) {
        return;
    }

    auto tone_color = [](Tone tone) -> ImU32 {
        switch (tone) {
        case Tone::good:
            return colors::table_bg_green;
        case Tone::warning:
            return colors::table_bg_yellow;
        case Tone::bad:
            return colors::table_bg_red;
        default:
            return 0;
        }
    };

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
            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg,
                                   tone_color(row.tone));
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
}

bool ServerSetupPanel::_busy() const {
    return _transfer != Transfer::none || _server->config_service.busy();
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
    _transfer = Transfer::read_all;
    bsclog::info("Чтение всех параметров {}...", _server->name());
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
            row.tone = Tone::bad;
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
