#include "serversetup.h"


namespace ui {


ServerSetup::ServerSetup(std::shared_ptr<ucanopen::Server> server,
                         const std::string& menu_title,
                         const std::string& window_title,
                         bool show_by_default)
        : View(menu_title, window_title, show_by_default)
        , _server(server)
{}


void ServerSetup::draw(bool& open) {
    ImGui::Begin(_window_title.c_str(), &open);

    _draw_about();
    _draw_setup();
    _draw_popups();

    ImGui::End();
}


void ServerSetup::_draw_about() {
    ImGui::SeparatorText("About");
    if (ImGui::Button(ICON_MDI_REFRESH " Refresh##about")) {
        _device_name.clear();
        _hardware_version.clear();
        _software_version.clear();
        _device_sn.clear();
    }

    ImGui::TextDisabled("Device Name: ");
    ImGui::SameLine();
    if (_device_name.empty()) {
        _device_name = _server->read_string("info", "sys", "device_name", std::chrono::milliseconds(500));
    }
    ImGui::TextUnformatted(_device_name.c_str());

    ImGui::TextDisabled("Hardware Version: ");
    ImGui::SameLine();
    if (_hardware_version.empty()) {
        _hardware_version = _server->read_string("info", "sys", "hardware_version", std::chrono::milliseconds(500));
    }
    ImGui::TextUnformatted(_hardware_version.c_str());

    ImGui::TextDisabled("Software Version: ");
    ImGui::SameLine();
    if (_software_version.empty()) {
        _software_version = _server->read_string("info", "sys", "firmware_version", std::chrono::milliseconds(500));
    }
    ImGui::TextUnformatted(_software_version.c_str());

    ImGui::TextDisabled("Device S/N: ");
    ImGui::SameLine();
    if (_device_sn.empty()) {
        _device_sn = _server->read_numval("info", "sys", "serial_number", std::chrono::milliseconds(500));
    }
    ImGui::TextUnformatted(_device_sn.c_str()); 
}


void ServerSetup::_draw_setup() {
    ImGui::SeparatorText("Setup");

    const auto& objects = _server->config_service.objects();
    if (objects.empty()) { return; }

    static auto selected_category_iter = objects.begin();
    static size_t selected_object_idx = 0;
    static bool should_read = true;

    if (ImGui::Button(ICON_MDI_REFRESH " Refresh##setup")) {
        should_read = true;
    }

    auto category_preview = selected_category_iter->first;

    if (ImGui::BeginCombo("Category", category_preview.data())) {
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

    auto object_preview = selected_category_iter->second[selected_object_idx]->name;

    if (ImGui::BeginCombo("Object", object_preview.c_str())) {
        for (size_t i = 0; i < selected_category_iter->second.size(); ++i) {
            auto is_selected = (i == selected_object_idx);
            if (ImGui::Selectable(selected_category_iter->second[i]->name.c_str(), is_selected)) {
                selected_object_idx = i;
                should_read = true;
            }
        }
        ImGui::EndCombo();
    }

    static std::optional<ucanopen::ExpeditedSdoData> parameter_value;
    if (should_read) {
        parameter_value = _server->read_expdata(_server->dictionary().config.config_category,
                                                selected_category_iter->first,
                                                selected_category_iter->second[selected_object_idx]->name,
                                                std::chrono::milliseconds(500));
        should_read = false;
    }

    if (!parameter_value.has_value()) {
        std::string str = "n/a";
        ImGui::InputText("Value", str.data(), 3, ImGuiInputTextFlags_ReadOnly);
    } else {

        switch (selected_category_iter->second[selected_object_idx]->data_type) {
        case ucanopen::OD_BOOL:
        case ucanopen::OD_UINT8: {
            uint8_t value_u8 = parameter_value.value().u8();
            if (ImGui::InputScalar("Value", ImGuiDataType_U8, &value_u8, NULL, NULL, NULL, ImGuiInputTextFlags_EnterReturnsTrue)) {
                _server->write(_server->dictionary().config.config_category,
                            selected_category_iter->first,
                            selected_category_iter->second[selected_object_idx]->name,
                            ucanopen::ExpeditedSdoData(uint8_t(value_u8)));
                should_read = true;
            }
            break;
        }

        case ucanopen::OD_UINT16: {
            uint16_t value_u16 = parameter_value.value().u16();
            if (ImGui::InputScalar("Value", ImGuiDataType_U16, &value_u16, NULL, NULL, NULL, ImGuiInputTextFlags_EnterReturnsTrue)) {
                _server->write(_server->dictionary().config.config_category,
                            selected_category_iter->first,
                            selected_category_iter->second[selected_object_idx]->name,
                            ucanopen::ExpeditedSdoData(uint16_t(value_u16)));
                should_read = true;
            }
            break;
        }

        case ucanopen::OD_UINT32: {
            uint32_t value_u32 = parameter_value.value().u32();
            if (ImGui::InputScalar("Value", ImGuiDataType_U32, &value_u32, NULL, NULL, NULL, ImGuiInputTextFlags_EnterReturnsTrue)) {
                _server->write(_server->dictionary().config.config_category,
                            selected_category_iter->first,
                            selected_category_iter->second[selected_object_idx]->name,
                            ucanopen::ExpeditedSdoData(uint32_t(value_u32)));
                should_read = true;
            }
            break;
        }

        case ucanopen::OD_INT8: {
            int16_t value_i8 = parameter_value.value().i8();
            if (ImGui::InputScalar("Value", ImGuiDataType_S8, &value_i8, NULL, NULL, "%d", ImGuiInputTextFlags_EnterReturnsTrue)) {
                _server->write(_server->dictionary().config.config_category,
                            selected_category_iter->first,
                            selected_category_iter->second[selected_object_idx]->name,
                            ucanopen::ExpeditedSdoData(int8_t(value_i8)));
                should_read = true;
            }
            break;
        }

        case ucanopen::OD_INT16: {
            int16_t value_i16 = parameter_value.value().i16();
            if (ImGui::InputScalar("Value", ImGuiDataType_S16, &value_i16, NULL, NULL, "%d", ImGuiInputTextFlags_EnterReturnsTrue)) {
                _server->write(_server->dictionary().config.config_category,
                            selected_category_iter->first,
                            selected_category_iter->second[selected_object_idx]->name,
                            ucanopen::ExpeditedSdoData(int16_t(value_i16)));
                should_read = true;
            }
            break;
        }

        case ucanopen::OD_INT32: {
            int32_t value_i32 = parameter_value.value().i32();
            if (ImGui::InputScalar("Value", ImGuiDataType_S32, &value_i32, NULL, NULL, NULL, ImGuiInputTextFlags_EnterReturnsTrue)) {
                _server->write(_server->dictionary().config.config_category,
                            selected_category_iter->first,
                            selected_category_iter->second[selected_object_idx]->name,
                            ucanopen::ExpeditedSdoData(int32_t(value_i32)));
                should_read = true;
            }
            break;
        }

        case ucanopen::OD_FLOAT32: {
            float value_f32 = parameter_value.value().f32();
            if (ImGui::InputScalar("Value", ImGuiDataType_Float, &value_f32, NULL, NULL, "%.6f", ImGuiInputTextFlags_EnterReturnsTrue)) {
                _server->write(_server->dictionary().config.config_category,
                            selected_category_iter->first,
                            selected_category_iter->second[selected_object_idx]->name,
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
    
    if (ImGui::Button("Restore", ImVec2(120, 0))) {
        _server->sdo_service.restore_default_parameter(_server->dictionary().config.config_category,
                                                       selected_category_iter->first,
                                                       selected_category_iter->second[selected_object_idx]->name);
        should_read = true;
    }

    ImGui::SameLine();

    if (ImGui::Button("Restore All", ImVec2(120, 0))) {
        ImGui::OpenPopup("Warning!##restore");
    }

    if (ImGui::Button("Apply", ImVec2(248, 0))) {
        ImGui::OpenPopup("Warning!##apply");
    }
}


void ServerSetup::_draw_popups() {
    // Always center this window when appearing
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Warning!##restore", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Default configuration parameters will be restored.");
        ImGui::Separator();

        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Ok", ImVec2(120, 0))) {
            _server->exec("ctl", "sys", "restore_all_default_parameters");
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal("Warning!##apply", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("All configuration parameters will be applied.");
        ImGui::Separator();

        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Ok", ImVec2(120, 0))) {
            _server->exec("ctl", "sys", "save_all_parameters");
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}


} // namespace ui
