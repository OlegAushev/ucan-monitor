#include "options.h"
#include <icons/IconsMaterialDesignIcons.h>

namespace ui {

Options::Options(std::shared_ptr<can::Socket> socket,
                 std::shared_ptr<ucanopen::Client> ucanopen_client) {
    _socket = socket;
    _client = ucanopen_client;

    _client->set_sync_period(std::chrono::milliseconds(_client_sync_period));
    for (const auto& server : _client->server_names()) {
        _client->set_watch_period_on_server(
                server,
                std::chrono::milliseconds(_server_watch_period));
    }
}

void Options::draw(bool& open) {
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;

    if (!ImGui::Begin("Настройка##options", &open, window_flags)) {
        ImGui::End();
        return;
    }

    if (!_socket) {
        ImGui::End();
        return;
    }

    if (ImGui::BeginTabBar("##TabBar")) {
        if (ImGui::BeginTabItem("SocketCAN")) {
            _draw_socketcan_tab();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("CANopen")) {
            _draw_ucanopen_tab();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Внешний Вид")) {
            _draw_appearance_tab();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
    ImGui::End();
}

void Options::_draw_socketcan_tab() {
    auto interface_preview =
            can::detail::interface_list[_selected_interface_idx].c_str();
    if (ImGui::BeginCombo("Интерфейс", interface_preview)) {
        for (size_t i = 0; i < can::detail::interface_list.size(); ++i) {
            auto is_selected = (i == _selected_interface_idx);
            if (ImGui::Selectable(can::detail::interface_list[i].c_str(),
                                  is_selected)) {
                _selected_interface_idx = i;
            }
        }
        ImGui::EndCombo();
    }

    auto bitrate_preview =
            can::detail::bitrate_list[_selected_bitrate_idx].c_str();
    if (ImGui::BeginCombo("Битрейт", bitrate_preview)) {
        for (size_t i = 0; i < can::detail::bitrate_list.size(); ++i) {
            auto is_selected = (i == _selected_bitrate_idx);
            if (ImGui::Selectable(can::detail::bitrate_list[i].c_str(),
                                  is_selected)) {
                _selected_bitrate_idx = i;
            }
        }
        ImGui::EndCombo();
    }

    ImGui::NewLine();

    if (ImGui::Button(ICON_MDI_NETWORK_OFF_OUTLINE " Отключить",
                      ImVec2(160, 0))) {
        _can_status = _socket->disconnect();
        if (_can_status == can::Status::ok) {
            ImGui::OpenPopup("CAN Bus setup success");
        } else {
            ImGui::OpenPopup("CAN Bus setup fail");
        }
    }

    ImGui::SameLine();

    if (ImGui::Button(ICON_MDI_NETWORK_OUTLINE " Подключить", ImVec2(160, 0))) {
        _can_status = _socket->connect(
                can::detail::interface_list[_selected_interface_idx],
                can::detail::bitrate_list[_selected_bitrate_idx]);
        if (_can_status == can::Status::ok) {
            ImGui::OpenPopup("CAN Bus setup success");
        } else {
            ImGui::OpenPopup("CAN Bus setup fail");
        }
    }

    if (ImGui::BeginPopup("CAN Bus setup success")) {
        ImGui::Text("Успех!");
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopup("CAN Bus setup fail")) {
        ImGui::Text("Ошибка. Код: %d", std::to_underlying(_can_status));
        ImGui::EndPopup();
    }
}

void Options::_draw_ucanopen_tab() {
    ImGui::SeparatorText("Клиент");

    static int client_id = _client->node_id().get();
    if (ImGui::InputInt("ID Клиента",
                        &client_id,
                        1,
                        100,
                        ImGuiInputTextFlags_EnterReturnsTrue)) {
        client_id = std::clamp(client_id, 1, 127);
        if (_client->set_node_id(ucanopen::NodeId(client_id)) !=
            ucanopen::SetupStatus::success) {
            client_id = _client->node_id().get();
        }
    }

    static bool client_sync_enabled = true;
    if (ImGui::Checkbox("SYNC-сообщения", &client_sync_enabled)) {
        if (client_sync_enabled) {
            _client->enable_sync();
        } else {
            _client->disable_sync();
        }
    }

    if (ImGui::InputInt("Период SYNC-сообщений",
                        &_client_sync_period,
                        1,
                        100,
                        ImGuiInputTextFlags_EnterReturnsTrue)) {
        _client_sync_period = std::clamp(_client_sync_period, 1, 10000);
        _client->set_sync_period(
                std::chrono::milliseconds(_client_sync_period));
    }

    static bool client_tpdo_enabled = true;
    if (ImGui::Checkbox("TPDO-сообщения", &client_tpdo_enabled)) {
        if (client_tpdo_enabled) {
            _client->enable_tpdo();
        } else {
            _client->disable_tpdo();
        }
    }

    for (const auto& server : _client->server_names()) {
        _draw_server_settings(server);
    }
}

void Options::_draw_server_settings(const std::string& name) {
    auto server = _client->server(name);
    if (!server) {
        return;
    }

    ImGui::NewLine();
    std::string separator_text = "Сервер: " + name;
    ImGui::SeparatorText(separator_text.c_str());

    // Each server draws the same widgets: its own ID scope keeps them apart,
    // and each shows that server's setting, read from the server itself.
    ImGui::PushID(name.c_str());

    int server_id = server->node_id().get();
    if (ImGui::InputInt("ID Сервера",
                        &server_id,
                        1,
                        100,
                        ImGuiInputTextFlags_EnterReturnsTrue)) {
        server_id = std::clamp(server_id, 1, 127);
        _client->set_server_node_id(name, ucanopen::NodeId(server_id));
    }

    bool rpdo_enabled = server->rpdo_service.enabled();
    if (ImGui::Checkbox("RPDO-сообщения", &rpdo_enabled)) {
        if (rpdo_enabled) {
            _client->enable_rpdo_on_server(name);
        } else {
            _client->disable_rpdo_on_server(name);
        }
    }

    bool watch_enabled = server->watch_service.enabled();
    if (ImGui::Checkbox("Watch-сообщения", &watch_enabled)) {
        if (watch_enabled) {
            _client->enable_watch_on_server(name);
        } else {
            _client->disable_watch_on_server(name);
        }
    }

    int watch_period = static_cast<int>(server->watch_service.period().count());
    if (ImGui::InputInt("Период Watch-сообщений",
                        &watch_period,
                        1,
                        100,
                        ImGuiInputTextFlags_EnterReturnsTrue)) {
        watch_period = std::clamp(watch_period, 1, 10000);
        _client->set_watch_period_on_server(
                name,
                std::chrono::milliseconds(watch_period));
    }

    int log_capacity = static_cast<int>(server->log_service.log_capacity());
    if (ImGui::InputInt("Размер Лога",
                        &log_capacity,
                        1,
                        100,
                        ImGuiInputTextFlags_EnterReturnsTrue)) {
        log_capacity = std::clamp(log_capacity, 10, 1000000);
        _client->set_log_capacity_on_server(name, log_capacity);
    }

    ImGui::PopID();
}

void Options::_draw_appearance_tab() {
    ImGui::ShowStyleSelector("Стиль ImGui");
    ImPlot::ShowStyleSelector("Стиль ImPlot");
    ImPlot::ShowColormapSelector("Цвета ImPlot");
}

} // namespace ui
