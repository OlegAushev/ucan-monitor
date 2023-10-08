#include "options.h"


namespace ui {


Options::Options(std::shared_ptr<can::Socket> socket, std::shared_ptr<ucanopen::Client> ucanopen_client) {
    _socket = socket;
    _client = ucanopen_client;

    _client->enable_sync();
    _client->set_sync_period(std::chrono::milliseconds(200));
    _client->enable_tpdo();
    for (const auto& server : _client->server_names()) {
        _client->enable_rpdo_on_server(server);
        _client->enable_watch_on_server(server);
        _client->set_watch_period_on_server(server, std::chrono::milliseconds(10));
    }
}


void Options::draw(bool& open) {
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;

    if (!ImGui::Begin("Options", &open, window_flags))
    {
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
        ImGui::EndTabBar();
    }
    ImGui::End();
}


void Options::_draw_socketcan_tab() {
    auto interface_preview = can::detail::interface_list[_selected_interface_idx].c_str();
    if (ImGui::BeginCombo("Interface", interface_preview)) {
        for (auto i = 0; i < can::detail::interface_list.size(); ++i) {
            auto is_selected = (i == _selected_interface_idx);
            if (ImGui::Selectable(can::detail::interface_list[i].c_str(), is_selected)) {
                _selected_interface_idx = i;
            }
        }
        ImGui::EndCombo();
    }

    auto bitrate_preview = can::detail::bitrate_list[_selected_bitrate_idx].c_str();
    if (ImGui::BeginCombo("Bitrate", bitrate_preview)) {
        for (auto i = 0; i < can::detail::bitrate_list.size(); ++i) {
            auto is_selected = (i == _selected_bitrate_idx);
            if (ImGui::Selectable(can::detail::bitrate_list[i].c_str(), is_selected)) {
                _selected_bitrate_idx = i;
            }
        }
        ImGui::EndCombo();
    }

    ImGui::NewLine();

    if (ImGui::Button("Disconnect", ImVec2(120, 0))) {
        _error = _socket->disconnect();
        if (_error == can::Error::none) {
            ImGui::OpenPopup("CAN Bus setup success");
        } else {
            ImGui::OpenPopup("CAN Bus setup fail");
        }
    }

    ImGui::SameLine();

    if (ImGui::Button("Connect", ImVec2(120, 0))) {
        _error = _socket->connect(can::detail::interface_list[_selected_interface_idx], can::detail::bitrate_list[_selected_bitrate_idx]);
        if (_error == can::Error::none) {
            ImGui::OpenPopup("CAN Bus setup success");
        } else {
            ImGui::OpenPopup("CAN Bus setup fail");
        }
    }

    if (ImGui::BeginPopup("CAN Bus setup success"))
    {
        ImGui::Text("Success!");
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopup("CAN Bus setup fail"))
    {
        ImGui::Text("Fail. Error code: %d", std::to_underlying(_error));
        ImGui::EndPopup();
    }
}


void Options::_draw_ucanopen_tab() {
    ImGui::SeparatorText("Client");

    static int client_id = _client->node_id().get();
    if (ImGui::InputInt("Client ID", &client_id, 1, 100, ImGuiInputTextFlags_EnterReturnsTrue)) {
        if (client_id < 1) {
            client_id = 1;
        } else if (client_id > 127) {
            client_id = 127;
        }

        if (_client->set_node_id(ucanopen::NodeId(client_id)) != ucanopen::SetupStatus::success) {
            client_id = _client->node_id().get();
        }
    }

    static bool client_sync_enabled = true;
    if (ImGui::Checkbox("SYNC Messages", &client_sync_enabled)) {
        if (client_sync_enabled) {
            _client->enable_sync();
        } else {
            _client->disable_sync();
        }
    }

    static int client_sync_period = 200;
    if (ImGui::InputInt("SYNC Period", &client_sync_period, 1, 100, ImGuiInputTextFlags_EnterReturnsTrue)) {
        if (client_sync_period < 1) {
            client_sync_period = 1;
        } else if (client_sync_period > 10000) {
            client_sync_period = 10000;
        }
        _client->set_sync_period(std::chrono::milliseconds(client_sync_period));
    }

    static bool client_tpdo_enabled = true;
    if (ImGui::Checkbox("TPDO Messages", &client_tpdo_enabled)) {
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
    

void Options::_draw_server_settings(const std::string& server) {
    ImGui::NewLine();
    std::string separator_text = "Server: " + server;
    ImGui::SeparatorText(separator_text.c_str());

    static int server_id = _client->server(server)->node_id().get();
    if (ImGui::InputInt("Server ID", &server_id, 1, 100, ImGuiInputTextFlags_EnterReturnsTrue)) {
        if (server_id < 1) {
            server_id = 1;
        } else if (server_id > 127) {
            server_id = 127;
        }

        if (_client->set_server_node_id(server, ucanopen::NodeId(server_id)) != ucanopen::SetupStatus::success) {
            server_id = _client->server(server)->node_id().get();
        }
    }

    static bool server_rpdo_enabled = true;
    if (ImGui::Checkbox("RPDO messages", &server_rpdo_enabled)) {
        if (server_rpdo_enabled) {
            _client->enable_rpdo_on_server(server);
        } else {
            _client->disable_rpdo_on_server(server);
        }
    }

    static bool server_watch_enabled = true;
    if (ImGui::Checkbox("Watch messages", &server_watch_enabled)) {
        if (server_watch_enabled) {
            _client->enable_watch_on_server(server);
        } else {
            _client->disable_watch_on_server(server);
        }
    }

    static int server_watch_period = 10;
    if (ImGui::InputInt("Watch Period", &server_watch_period, 1, 100, ImGuiInputTextFlags_EnterReturnsTrue)) {
        if (server_watch_period < 1) {
            server_watch_period = 1;
        } else if (server_watch_period > 10000) {
            server_watch_period = 10000;
        }
        _client->set_watch_period_on_server(server, std::chrono::milliseconds(server_watch_period));
    }
}


} // namespace ui
