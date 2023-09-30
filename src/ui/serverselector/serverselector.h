#pragma once


#include <imgui.h>
#include <string>
#include <vector>


namespace ui {


class ServerSelector {
private:
    static inline int _server_selected_id = 0;
    static inline bool _is_selected = false;
public:
    static void show_server_selector_window(const std::vector<std::string>& server_names);
    static bool is_selected() { return _is_selected; }
};


} // namespace ui
