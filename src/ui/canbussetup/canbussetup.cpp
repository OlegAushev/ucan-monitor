#include "canbussetup.h"


namespace ui {


void CanBusSetup::init(std::shared_ptr<can::Socket> socket) {
    _socket = socket;
}


void CanBusSetup::show(bool* p_open) {
    if (!ImGui::Begin("CAN Bus Setup", p_open))
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
            _show_socketcan_tab();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("CANopen")) {
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}


void CanBusSetup::_show_socketcan_tab() {
    auto interface_preview = can::detail::interface_list[_selected_interface_idx].c_str();
    if (ImGui::BeginCombo("Interface", interface_preview)) {
        for (auto i = 0; i < can::detail::interface_list.size(); ++i) {
            auto is_selected = (_selected_interface_idx == i);
            if (ImGui::Selectable(can::detail::interface_list[i].c_str(), is_selected)) {
                _selected_interface_idx = i;
            }
        }
        ImGui::EndCombo();
    }

    auto bitrate_preview = can::detail::bitrate_list[_selected_bitrate_idx].c_str();
    if (ImGui::BeginCombo("Bitrate", bitrate_preview)) {
        for (auto i = 0; i < can::detail::bitrate_list.size(); ++i) {
            auto is_selected = (_selected_bitrate_idx == i);
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


} // namespace ui
