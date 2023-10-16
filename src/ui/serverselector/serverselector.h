#pragma once


#include <imgui.h>
#include <string>
#include <vector>


namespace ui {


class ServerSelector {
private:
    ServerSelector() = default;
    int _selected_server_idx{0};
    bool _server_is_selected{false};
    std::string _selected_server;
public:
    static ServerSelector& instance() {
        static ServerSelector s;
        return s;
    }

    void show(const std::vector<std::string>& server_names);
    bool server_is_selected() { return _server_is_selected; }
    std::string selected_server() const { return _selected_server; }
};


} // namespace ui
