#pragma once


#include <imgui.h>
#include <ui/view/view.h>
#include <ucanopen/server/server.h>

#include <optional>
#include <string>
#include <string_view>
#include <vector>


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

    // All the parameters at once. The table holds what the last transfer
    // left; the panel stays busy until it has taken the transfer's result.
    enum class Transfer { none, read_all, compare, write };
    Transfer _transfer{Transfer::none};

    // What the rows hold: every value of the server, or the values of a file
    // set against the server's.
    enum class Table { values, file };
    Table _table{Table::values};

    struct Row {
        ucanopen::ODEntryIter entry{};
        std::optional<ucanopen::ExpeditedSdoData> server_value{};
        std::optional<ucanopen::ExpeditedSdoData> file_value{};
        bool selected{false};
        std::string status{};
        ImU32 status_color{0}; // the background of the status, none if 0

        // The file's value is not the server's, or the server's is unknown.
        bool differs() const;
    };
    std::vector<Row> _rows;
    std::vector<size_t> _written_rows; // a write and a read-back each

    std::string _file_name;
    std::string _file_server;
    std::vector<std::string> _notes; // on the file loaded
    std::string _hint;               // what the operator has to do next

    // ImGuiFileDialog is one for the whole application: each panel opens it
    // under its own key, so that it takes back only its own files.
    std::string _save_dialog_key;
    std::string _load_dialog_key;
public:
    ServerSetupPanel(std::shared_ptr<ucanopen::Server> server,
                const std::string& menu_title,
                const std::string& window_title,
                bool open);
    virtual void draw() override;
private:
    void _draw_about();
    void _draw_setup();
    void _draw_all_parameters();
    void _draw_values_table();
    void _draw_file_table();
    void _draw_popups();
    void _draw_dialogs();

    bool _busy() const;
    size_t _selected_count() const;
    void _read_all();
    void _load_file(const std::string& path);
    void _write_selected();
    void _take_transfer();
    void _take_read_all(const std::vector<ucanopen::ConfigStep>& steps,
                        ucanopen::ServerConfigService::Outcome outcome);
    void _take_compare(const std::vector<ucanopen::ConfigStep>& steps,
                       ucanopen::ServerConfigService::Outcome outcome);
    void _take_write(const std::vector<ucanopen::ConfigStep>& steps,
                     ucanopen::ServerConfigService::Outcome outcome);
    void _open_save_dialog();
    void _open_load_dialog();
    void _save_file(const std::string& path);
};


} // namespace ui
