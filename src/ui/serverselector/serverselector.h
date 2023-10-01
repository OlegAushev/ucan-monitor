#pragma once


#include <imgui.h>
#include <string>
#include <vector>


namespace ui {


class ServerSelector {
private:
    ServerSelector() = default;
    int _server_selected_id = 0;
    bool _server_is_selected = false;
public:
    static ServerSelector& instance() {
        static ServerSelector s;
        return s;
    }

    void show(const std::vector<std::string>& server_names);
    bool server_is_selected() { return _server_is_selected; }
};


} // namespace ui
