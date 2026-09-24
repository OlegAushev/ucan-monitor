#pragma once


#include <imgui.h>
#include <ui/view/view.h>
#include <ucanopen/server/server.h>

#include <optional>
#include <string_view>


namespace ui {


class ServerSetupPanel : public View {
private:
    std::shared_ptr<ucanopen::Server> _server;

    std::string _device_name;
    std::string _hardware_version;
    std::string _software_version;
    std::string _software_commitdate;
    std::string _software_branch;
    std::string _device_sn;

    // The parameter the setup section shows. It belongs to this panel: with a
    // panel per server, a shared one would point into another server's
    // dictionary. An empty category stands for the first one.
    std::string_view _category;
    size_t _selected_object_idx{0};
    bool _should_read{true};
    std::optional<ucanopen::ExpeditedSdoData> _parameter_value;
public:
    ServerSetupPanel(std::shared_ptr<ucanopen::Server> server,
                const std::string& menu_title,
                const std::string& window_title,
                bool open);
    virtual void draw() override;
private:
    void _draw_about();
    void _draw_setup();
    void _draw_popups();
};


} // namespace ui
