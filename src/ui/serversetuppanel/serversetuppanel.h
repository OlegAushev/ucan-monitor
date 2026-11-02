#pragma once


#include <imgui.h>
#include <ui/view/view.h>
#include <ucanopen/server/server.h>

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>


namespace ui {


// The setup of a server by the description of its dictionary: what the
// device says of itself, its parameters one by one and all at once, and files
// of them. A parameter is a record: its value requested, applied and stored,
// its default, bounds and policy. A write only requests a value, which a
// command applies, of the parameter or of all of them
// (docs/canopen-rework-plan.ru.md, section 3.8).
class ServerSetupPanel : public View {
private:
    std::shared_ptr<ucanopen::Server> _server;

    // The records of the class parameters by group, as the description orders
    // them.
    struct Group {
        const ucanopen::od::Group* group;
        std::vector<const ucanopen::od::Record*> records;
    };
    std::vector<Group> _groups;
    size_t _group_idx{0};
    size_t _record_idx{0};

    // The members of the parameter shown, by subindex, as last read.
    std::map<uint8_t, ucanopen::ExpeditedSdoData> _shown;
    bool _should_read{true};

    // The default, the bounds and the policy of each parameter, which do not
    // change while the device runs: read once a connection.
    unsigned _epoch{0};
    std::map<uint16_t, std::map<uint8_t, ucanopen::ExpeditedSdoData>> _constants;

    // The transfer running; the panel stays busy until it has taken its
    // result.
    enum class Transfer { none, parameter, command, read_all, compare, write };
    Transfer _transfer{Transfer::none};
    std::string _result;       // how the last command went
    ImU32 _result_color{0};

    // What the rows hold: every value of the device, or the values of a file
    // set against the device's.
    enum class Table { values, file };
    Table _table{Table::values};

    struct Row {
        const ucanopen::od::Record* record{nullptr}; // null for an object, read only
        ucanopen::od::Item item;                     // the requested member of the record, or the object
        std::string group;                           // the section and the name in a file
        std::string name;
        std::optional<ucanopen::ExpeditedSdoData> requested{};
        std::optional<ucanopen::ExpeditedSdoData> applied{}; // the value of an object
        std::optional<ucanopen::ExpeditedSdoData> stored{};
        std::optional<ucanopen::ExpeditedSdoData> file_value{};
        bool selected{false};
        std::string status{};
        ImU32 status_color{0}; // the background of the status, none if 0

        // The file's value is not the stored one, or the stored one is
        // unknown.
        bool differs() const;
    };
    std::vector<Row> _rows;
    std::vector<size_t> _written_rows;

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
    void _draw_state();
    void _draw_parameter();
    void _draw_actions();
    void _draw_all_parameters();
    void _draw_values_table();
    void _draw_file_table();
    void _draw_popups();
    void _draw_dialogs();

    bool _busy() const;
    size_t _selected_count() const;
    const ucanopen::od::Record* _record() const;
    std::optional<ucanopen::ExpeditedSdoData> _constant(const ucanopen::od::Record& record, uint8_t subindex) const;
    std::string _refusal_text(ucanopen::SdoAbortCode code) const;
    std::string _failure(const ucanopen::SdoStep& step) const;
    bool _status_active(std::string_view name) const;

    void _read_parameter(std::vector<ucanopen::SdoStep> first = {});
    void _command(std::string_view path);
    void _command_member(uint8_t subindex);
    void _write_requested(ucanopen::ExpeditedSdoData value);
    void _read_all();
    void _load_file(const std::string& path);
    void _write_selected();
    void _take_transfer();
    void _take_parameter(const std::vector<ucanopen::SdoStep>& steps);
    void _take_read_all(const std::vector<ucanopen::SdoStep>& steps, ucanopen::SdoSequence::Outcome outcome);
    void _take_compare(const std::vector<ucanopen::SdoStep>& steps, ucanopen::SdoSequence::Outcome outcome);
    void _take_write(const std::vector<ucanopen::SdoStep>& steps, ucanopen::SdoSequence::Outcome outcome);
    void _open_save_dialog();
    void _open_load_dialog();
    void _save_file(const std::string& path);
};


} // namespace ui
