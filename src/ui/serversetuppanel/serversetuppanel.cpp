#include "serversetuppanel.h"

namespace ui {

ServerSetupPanel::ServerSetupPanel(std::shared_ptr<ucanopen::Server> server,
                                   const std::string& menu_title,
                                   const std::string& window_title,
                                   bool open)
        : View(menu_title, window_title, open), _server(server) {}

void ServerSetupPanel::draw() {
    ImGui::Begin(_window_title.c_str(), &_opened);

    _draw_about();
    _draw_setup();
    _draw_popups();

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

    static auto selected_category_iter = objects.begin();
    static size_t selected_object_idx = 0;
    static bool should_read = true;

    if (ImGui::Button(ICON_MDI_REFRESH " Обновить##setup")) {
        should_read = true;
    }

    if (ImGui::BeginCombo("Категория", selected_category_iter->first.data())) {
        for (auto iter = objects.begin(); iter != objects.end(); ++iter) {
            auto is_selected = (iter == selected_category_iter);
            if (ImGui::Selectable(iter->first.data(), is_selected)) {
                selected_category_iter = iter;
                selected_object_idx = 0;
                should_read = true;
            }
        }
        ImGui::EndCombo();
    }

    const std::string object_preview =
            selected_category_iter->second[selected_object_idx]->name + "[" +
            selected_category_iter->second[selected_object_idx]->unit + "]";

    if (ImGui::BeginCombo("Объект", object_preview.c_str())) {
        for (size_t i = 0; i < selected_category_iter->second.size(); ++i) {
            auto is_selected = (i == selected_object_idx);
            const auto& obj = selected_category_iter->second[i];
            const std::string obj_ = obj->name + "[" + obj->unit + "]";
            if (ImGui::Selectable(obj_.c_str(), is_selected)) {
                selected_object_idx = i;
                should_read = true;
            }
        }
        ImGui::EndCombo();
    }

    static std::optional<ucanopen::ExpeditedSdoData> parameter_value;
    if (should_read) {
        parameter_value = _server->read_expdata(
                _server->dictionary().config.config_category,
                selected_category_iter->first,
                selected_category_iter->second[selected_object_idx]->name,
                std::chrono::milliseconds(500));
        should_read = false;
    }

    if (!parameter_value.has_value()) {
        std::string str = "н/д";
        ImGui::InputText("Значение",
                         str.data(),
                         3,
                         ImGuiInputTextFlags_ReadOnly);
    } else {

        switch (selected_category_iter->second[selected_object_idx]
                        ->data_type) {
        case ucanopen::OD_BOOL:
        case ucanopen::OD_UINT8: {
            uint8_t value_u8 = parameter_value.value().u8();
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
                        selected_category_iter->second[selected_object_idx]
                                ->name,
                        ucanopen::ExpeditedSdoData(uint8_t(value_u8)));
                should_read = true;
            }
            break;
        }

        case ucanopen::OD_UINT16: {
            uint16_t value_u16 = parameter_value.value().u16();
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
                        selected_category_iter->second[selected_object_idx]
                                ->name,
                        ucanopen::ExpeditedSdoData(uint16_t(value_u16)));
                should_read = true;
            }
            break;
        }

        case ucanopen::OD_UINT32: {
            uint32_t value_u32 = parameter_value.value().u32();
            char const* format{NULL};
            ImGuiInputTextFlags flags{ImGuiInputTextFlags_EnterReturnsTrue};
            if (selected_category_iter->second[selected_object_idx]->unit ==
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
                        selected_category_iter->second[selected_object_idx]
                                ->name,
                        ucanopen::ExpeditedSdoData(uint32_t(value_u32)));
                should_read = true;
            }
            break;
        }

        case ucanopen::OD_INT8: {
            int16_t value_i8 = parameter_value.value().i8();
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
                        selected_category_iter->second[selected_object_idx]
                                ->name,
                        ucanopen::ExpeditedSdoData(int8_t(value_i8)));
                should_read = true;
            }
            break;
        }

        case ucanopen::OD_INT16: {
            int16_t value_i16 = parameter_value.value().i16();
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
                        selected_category_iter->second[selected_object_idx]
                                ->name,
                        ucanopen::ExpeditedSdoData(int16_t(value_i16)));
                should_read = true;
            }
            break;
        }

        case ucanopen::OD_INT32: {
            int32_t value_i32 = parameter_value.value().i32();
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
                        selected_category_iter->second[selected_object_idx]
                                ->name,
                        ucanopen::ExpeditedSdoData(int32_t(value_i32)));
                should_read = true;
            }
            break;
        }

        case ucanopen::OD_FLOAT32: {
            float value_f32 = parameter_value.value().f32();
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
                        selected_category_iter->second[selected_object_idx]
                                ->name,
                        ucanopen::ExpeditedSdoData(float(value_f32)));
                should_read = true;
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
                selected_category_iter->second[selected_object_idx]->name);
        should_read = true;
    }

    if (ImGui::Button("Восстановить Всё", ImVec2(-1.0f, 0))) {
        ImGui::OpenPopup("Внимание!##restore");
    }

    if (ImGui::Button("Применить", ImVec2(-1.0f, 0))) {
        ImGui::OpenPopup("Внимание!##apply");
    }
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

} // namespace ui
