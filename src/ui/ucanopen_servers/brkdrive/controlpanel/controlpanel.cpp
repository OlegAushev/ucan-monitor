#include "controlpanel.h"
#include "imgui.h"
#include <ui/util/togglebutton.h>
#include <ui/util/util.h>
#include <imguifiledialog/ImGuiFileDialog.h>


using namespace brkdrive;


namespace ui {
namespace brkdrive {


ControlPanel::ControlPanel(std::shared_ptr<::brkdrive::Server> server,
                           const std::string& menu_title,
                           const std::string& window_title,
                           bool open)
        : View(menu_title, window_title, open)
        , _server(server)
{}


void ControlPanel::_reset_refs() {
    _run = false;
    _calibrate = false;
    _ref_angle = 0;
    _ref_torque_pct = 0;
    _ref_speed = 0;
    _ref_dcurr_pu = 0;
    _openloop_ref_angle = 0;
    _track_speed = 0;

    _run_ref_control = ReferenceControl::manual;
    _angle_ref_control = ReferenceControl::manual;
}


void ControlPanel::_update_refs() {
    if (!_server->heartbeat_service.good()
            || _server->errors() != 0) {
        _run = false;
    }

    _server->set_opmode(_opmode);
    
    _server->toggle_wakeup(_wakeup);

    if (!(_run && _calibrate)) {
        if (_run) {
            _server->set_cmd_status(OperatingStatus::working);
        } else if (_calibrate) {
            _server->set_cmd_status(OperatingStatus::calibrating);
        } else {
            _server->set_cmd_status(OperatingStatus::inoperable);
        }
    }
    
    _server->set_ref_angle(_ref_angle);
    _server->set_track_speed(_track_speed);

    _ctlmode = static_cast<ControlMode>(_ctlmode_v);
    _ctlloop = static_cast<ControlLoop>(_ctlloop_v);
    _server->set_ctlmode(_ctlmode);
    _server->set_ctlloop(_ctlloop);

    _server->set_ref_torque(_ref_torque_pct / 100.0f);
    _server->set_ref_speed(_ref_speed);
    _server->set_ref_dcurr(_ref_dcurr_pu / 100.0f);
    _server->set_openloop_ref_angle(_openloop_ref_angle);
}


void ControlPanel::draw() {
    ImGui::Begin(_window_title.c_str(), &_opened);

    _read_keyboard();

    _draw_dash();
    _draw_normal_mode_controls();
    _draw_run_mode_controls();
    _draw_track_mode_controls();
    _draw_hwtest_mode_controls();

    _draw_actions();
    
    _update_refs();

    ImGui::End();
}


void ControlPanel::_draw_dash() {
    // Uptime
    ImGui::TextUnformatted(ICON_MDI_TIMER_OUTLINE);
    ImGui::SameLine();
    ImGui::PushItemWidth(120);
    auto uptime = _server->watch_service.string_value("sys", "uptime");
    ImGui::InputText("##uptime", uptime.data(), uptime.size(), ImGuiInputTextFlags_ReadOnly);
    ImGui::PopItemWidth();

    // Heartbeat indicator
    if (_server->heartbeat_service.good()) {
        ui::util::BlinkingText(ICON_MDI_NETWORK, std::chrono::milliseconds{750},
                               ui::colors::icon_green, ui::colors::icon_inactive);
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_red);
        ImGui::TextUnformatted(ICON_MDI_CLOSE_NETWORK);
        ImGui::PopStyleColor();
    }

    // Drive state indicator
    ImGui::SameLine();
    std::string state(drive_state_names.at(_server->drive_state()));
    ImGui::PushItemWidth(120);
    ImGui::InputText("##state", state.data(), state.size(), ImGuiInputTextFlags_ReadOnly);
    ImGui::PopItemWidth();

    // Operation mode selection
    ImGui::PushItemWidth(200);
    auto opmode_preview = opmode_names.at(_opmode).data();
    if (ImGui::BeginCombo("Operating Mode", opmode_preview)) {
        for (const auto& mode : opmode_names) {
            bool is_selected = (mode.first == _opmode);
            if (ImGui::Selectable(mode.second.data(), is_selected)) {
                _opmode = mode.first;
                _reset_refs();
            }
        }
        ImGui::EndCombo();
    }
    ImGui::PopItemWidth();

    // wakeup
    ToggleButton(ICON_MDI_POWER" On/Off", _wakeup, ImVec2{200, 0});
    ImGui::SameLine();
    ImGui::TextDisabled("(F3)");

    // start/stop
    ToggleButton(ICON_MDI_PLAY_CIRCLE_OUTLINE" Start/Stop " ICON_MDI_STOP_CIRCLE_OUTLINE, _run, ImVec2{200, 0});
    ImGui::SameLine();
    ImGui::TextDisabled("(F4)");
}


void ControlPanel::_read_keyboard() {
    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_F3))) {
        _wakeup = !_wakeup;
    }

    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_F4))) {
        _run = !_run;
    }
}


void ControlPanel::_draw_normal_mode_controls() {
    bool enabled = _server->opmode() == ::brkdrive::OperatingMode::normal;
    if (enabled) {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_green);
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_red);
    }
    ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED); 
    ImGui::PopStyleColor();

    bool selected = _opmode == OperatingMode::normal;
    ui::util::Switchable normal_mode_header(selected, [](){
        ImGui::SameLine();
        ImGui::SeparatorText("Normal Mode");
    });

    if (selected) {
        ToggleButton(ICON_MDI_MATH_COMPASS" Calibrate", _calibrate, ImVec2{200, 0});

        _draw_calibration_results();

        static int angle_mode = 0;
        float ref_angle;
        if (angle_mode == 0) {
            ref_angle = _ref_angle;
        } else {
            ref_angle = (_ref_angle / std::numbers::pi) * 180.f;
        }

        ImGui::PushItemWidth(200);
        ImGui::InputFloat("Angle", &ref_angle, 1.0f, 100.0f, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue);
        ImGui::PopItemWidth();

        if (angle_mode == 0) {
            _ref_angle = ref_angle;
        } else {
            _ref_angle = (ref_angle / 180.f) * std::numbers::pi;
        }    
        
        ImGui::SameLine();
        ImGui::RadioButton("[rad]", &angle_mode, 0);
        ImGui::SameLine();
        ImGui::RadioButton("[deg]", &angle_mode, 1);

        ImGui::PushItemWidth(200);
        if (ImGui::InputScalar("Track Speed [rpm]", ImGuiDataType_U16, &_track_speed, NULL, NULL, NULL, ImGuiInputTextFlags_EnterReturnsTrue)) {
            _track_speed = std::clamp(_track_speed, uint16_t(0), uint16_t(3000));
        }
        ImGui::PopItemWidth();
    }
}


void ControlPanel::_draw_run_mode_controls() {
    bool enabled = _server->opmode() == OperatingMode::run;
    if (enabled) {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_green);
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_red);
    }
    ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED); 
    ImGui::PopStyleColor();

    bool selected = _opmode == OperatingMode::run;
    ui::util::Switchable run_mode_header(selected, []() {
        ImGui::SameLine();
        ImGui::SeparatorText("Run Mode");
    });


    if (selected) {
        int ref_control = std::to_underlying(_run_ref_control);
        ImGui::PushItemWidth(200);
        if (ImGui::Combo("Ref Control##run_ref_control", &ref_control, "manual\0program\0\0")) {
            _run = false;
            _ref_torque_manager.reset();
            _ref_speed_manager.reset();
            _ref_torque_pct = 0;
            _ref_speed = 0;
        }
        ImGui::PopItemWidth();
        _run_ref_control = static_cast<ReferenceControl>(ref_control);
        
        if (_run_ref_control == ReferenceControl::program) {
            ReferenceManager* ref_manager = (_ctlmode == ControlMode::torque) ?    
                                            &_ref_torque_manager : &_ref_speed_manager;

            ImGui::PushItemWidth(200);
            std::string test_name = ref_manager->label();
            ImGui::InputText("Test Program", test_name.data(), test_name.size(), ImGuiInputTextFlags_ReadOnly);
            ImGui::PopItemWidth();

            util::Switchable load_test_button(!_run, [](){
                if (ImGui::Button("Load Test Program", ImVec2{200, 0})) {
                    IGFD::FileDialogConfig filedialog_config;
                    filedialog_config.path = ".";
                    ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".csv", filedialog_config);
                }
            });

            ImGui::SameLine();
            static bool repeat = false;
            ToggleButton(ICON_MDI_REPEAT, repeat);
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNone)) {
                ImGui::SetTooltip("Repeat test program");
            }

            // display
            if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey")) {
                if (ImGuiFileDialog::Instance()->IsOk()) { // action if OK
                std::string file_path = ImGuiFileDialog::Instance()->GetFilePathName();
                    ref_manager->read_file(file_path);
                }
                
                // close
                ImGuiFileDialog::Instance()->Close();
            }
            
            if (ref_manager->empty()) {
                _run = false;
            }

            if (_run && !ref_manager->empty()) {
                ImGui::ProgressBar(ref_manager->progress());
                auto ref = ref_manager->get();

                switch (_ctlmode) {
                case ControlMode::torque:
                    _ref_torque_pct = 100.f * ref.value_or(0);
                    break;
                case ControlMode::speed:
                    _ref_speed = ref.value_or(0);
                    break;
                default:
                    break;
                }

                if (!ref.has_value()) {
                    ref_manager->restart();
                    if (!repeat) {
                        _run = false;
                    }
                }
            }
        }

        // torque input
        ImGui::RadioButton("##torque_ctlmode", &_ctlmode_v, std::to_underlying(::brkdrive::ControlMode::torque));
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNone)) {
            ImGui::SetTooltip("Torque mode");
        }
        ImGui::SameLine();

        util::Switchable torque_input(_run_ref_control == ReferenceControl::manual, [this](){
            ImGui::PushItemWidth(200);
            if (ImGui::InputFloat("Torque [%]", &_ref_torque_pct, 1.0f, 100.0f, "%.2f", ImGuiInputTextFlags_EnterReturnsTrue)) {
                _ref_torque_pct = std::clamp(_ref_torque_pct, -100.0f, 100.0f);
            }
            if (_ctlmode != ControlMode::torque) {
                _ref_torque_pct = 0;
            }
            ImGui::PopItemWidth();
        });

        // speed input
        ImGui::RadioButton("##speed_ctlmode", &_ctlmode_v, std::to_underlying(::brkdrive::ControlMode::speed));
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNone)) {
            ImGui::SetTooltip("Speed mode");
        }
        ImGui::SameLine();

        util::Switchable speed_input(_run_ref_control == ReferenceControl::manual, [this](){
            ImGui::PushItemWidth(200);
            if (ImGui::InputScalar("Speed [rpm]", ImGuiDataType_S16,  &_ref_speed, NULL, NULL, NULL, ImGuiInputTextFlags_EnterReturnsTrue)) {
                _ref_speed = std::clamp(_ref_speed, int16_t(-5000), int16_t(5000));
            }
            if (_ctlmode != ControlMode::speed) {
                _ref_speed = 0;
            }
            ImGui::PopItemWidth();
        });

        // control loop
        if (ImGui::TreeNode("Control Loop")) {
            ImGui::RadioButton("Closed Loop", &_ctlloop_v, std::to_underlying(::brkdrive::ControlLoop::closed));
            if (ImGui::RadioButton("Open Loop", &_ctlloop_v, std::to_underlying(::brkdrive::ControlLoop::open))) {
                _ref_dcurr_pu = std::clamp(_ref_dcurr_pu, 0.0f, 100.0f);
            }
            ImGui::RadioButton("Semi-Closed Loop", &_ctlloop_v, std::to_underlying(::brkdrive::ControlLoop::semiclosed));
            ImGui::PushItemWidth(200);

            switch (_ctlloop) {
            case ControlLoop::closed:
                // DO NOTHING
                break;
            case ControlLoop::open:
                if (ImGui::InputFloat("D-Current [%]", &_ref_dcurr_pu, 0.1f, 100.0f, "%.1f", ImGuiInputTextFlags_EnterReturnsTrue)) {
                    _ref_dcurr_pu = std::clamp(_ref_dcurr_pu, 0.0f, 100.0f);
                }

                if (_ref_speed == 0) {
                    if (ImGui::InputInt("Angle [deg]", &_openloop_ref_angle, 1, 100, ImGuiInputTextFlags_EnterReturnsTrue)) {
                        // _openloop_ref_angle = std::clamp(_openloop_ref_angle, 0, 360);
                    }
                }

                break;
            case ControlLoop::semiclosed:
                if (ImGui::InputFloat("D-Current [%]", &_ref_dcurr_pu, 0.1f, 100.0f, "%.1f", ImGuiInputTextFlags_EnterReturnsTrue)) {
                    _ref_dcurr_pu = std::clamp(_ref_dcurr_pu, -100.0f, 100.0f);
                }           
                break;
            }

            ImGui::PopItemWidth();
            ImGui::TreePop();
        }
    }
}


void ControlPanel::_draw_track_mode_controls() {
    bool enabled = _server->opmode() == ::brkdrive::OperatingMode::track;
    if (enabled) {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_green);
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_red);
    }
    ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED); 
    ImGui::PopStyleColor();

    bool selected = _opmode == ::brkdrive::OperatingMode::track;
    ui::util::Switchable track_mode_header(selected, [](){
        ImGui::SameLine();  
        ImGui::SeparatorText("Track Mode");
    });

    if (selected) {
        int ref_control = std::to_underlying(_angle_ref_control);
        ImGui::PushItemWidth(200);
        if (ImGui::Combo("Ref Control##angle_ref_control", &ref_control, "manual\0program\0\0")) {
            _run = false;
            _ref_angle_manager.reset();
            _ref_angle = 0;
        }
        ImGui::PopItemWidth();
        _angle_ref_control = static_cast<ReferenceControl>(ref_control);

        static int angle_mode = 0;
        float ref_angle;

        if (_angle_ref_control == ReferenceControl::program) {
            ImGui::PushItemWidth(200);
            std::string test_name = _ref_angle_manager.label();
            ImGui::InputText("Test Program", test_name.data(), test_name.size(), ImGuiInputTextFlags_ReadOnly);
            ImGui::PopItemWidth();

            util::Switchable load_test_button(!_run, [](){
                if (ImGui::Button("Load Test Program", ImVec2{200, 0})) {
                    IGFD::FileDialogConfig filedialog_config;
                    filedialog_config.path = ".";
                    ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".csv", filedialog_config);
                }
            });

            ImGui::SameLine();
            static bool repeat = false;
            ToggleButton(ICON_MDI_REPEAT, repeat);
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNone)) {
                ImGui::SetTooltip("Repeat test program");
            }

            // display
            if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey")) {
                if (ImGuiFileDialog::Instance()->IsOk()) { // action if OK
                std::string file_path = ImGuiFileDialog::Instance()->GetFilePathName();
                    _ref_angle_manager.read_file(file_path);
                }
                
                // close
                ImGuiFileDialog::Instance()->Close();
            }
            
            if (_ref_angle_manager.empty()) {
                _run = false;
            }

            if (_run && !_ref_angle_manager.empty()) {
                ImGui::ProgressBar(_ref_angle_manager.progress());
                auto ref = _ref_angle_manager.get();
                ref_angle = ref.value_or(0);

                if (!ref.has_value()) {
                    _ref_angle_manager.restart();
                    if (!repeat) {
                        _run = false;
                    }
                }
            }
        } else {
            if (angle_mode == 0) {
                ref_angle = _ref_angle;
            } else {
                ref_angle = (_ref_angle / std::numbers::pi) * 180.f;
            }
        }

        ImGui::PushItemWidth(200);
        ImGui::InputFloat("Angle", &ref_angle, 1.0f, 100.0f, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue);
        ImGui::PopItemWidth();

        if (angle_mode == 0) {
            _ref_angle = ref_angle;
        } else {
            _ref_angle = (ref_angle / 180.f) * std::numbers::pi;
        }    
        
        ImGui::SameLine();
        ImGui::RadioButton("[rad]", &angle_mode, 0);
        ImGui::SameLine();
        ImGui::RadioButton("[deg]", &angle_mode, 1);

        ImGui::PushItemWidth(200);
        if (ImGui::InputScalar("Track Speed [rpm]", ImGuiDataType_U16, &_track_speed, NULL, NULL, NULL, ImGuiInputTextFlags_EnterReturnsTrue)) {
            _track_speed = std::clamp(_track_speed, uint16_t(0), uint16_t(3000));
        }
        ImGui::PopItemWidth();
    }
}


void ControlPanel::_draw_hwtest_mode_controls() {
    bool enabled = _server->opmode() == ::brkdrive::OperatingMode::hwtest;
    if (enabled) {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_green);
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ui::colors::icon_red);
    }
    ImGui::TextUnformatted(ICON_MDI_SQUARE_ROUNDED); 
    ImGui::PopStyleColor();

    bool selected = _opmode == ::brkdrive::OperatingMode::hwtest;
    ui::util::Switchable hwtest_mode_header(selected, [](){
        ImGui::SameLine();
        ImGui::SeparatorText("Hardware Test Mode");
    });
}


void ControlPanel::_draw_actions() {
    ImGui::SeparatorText("");

    ImGui::TextUnformatted(ICON_MDI_CAR_WRENCH);
    ImGui::SameLine();
    if (ImGui::TreeNode("Service Actions")) {
        if (ImGui::Button(ICON_MDI_SHIELD_REFRESH" Clear Errors          ")) {
            _server->exec("ctl", "sys", "clear_errors");
        }

        if (ImGui::Button(ICON_MDI_RESTART" Reset MCU             ")) {
            _server->exec("ctl", "sys", "reset_device");
        }

        if (ImGui::Button(ICON_MDI_COMPASS_OUTLINE" Calibrate Angle Sensor")) {
            _server->exec("ctl", "angsens", "calibrate");
        }

        ImGui::TreePop();
    }
}


void ControlPanel::_draw_popups() {
    // Always center this window when appearing
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Warning!##calibration", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Calibration procedure is about to begin.");
        ImGui::Separator();

        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Ok", ImVec2(120, 0))) {
            _server->exec("ctl", "drive", "calibrate");
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal("Warning!##direction", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Default rotation direction will be changed.");
        ImGui::Separator();

        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Ok", ImVec2(120, 0))) {
            _server->exec("ctl", "drive", "invert_rotdir");
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal("Warning!##device_reset", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Device will be reset.");
        ImGui::Separator();

        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Ok", ImVec2(120, 0))) {
            _server->exec("ctl", "sys", "reset_device");
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}


void ControlPanel::_draw_calibration_results() {
    static std::string phi_1;
    static std::string phi_2;
    static std::string phi_3;
    
    ImGui::TextUnformatted(ICON_MDI_MATH_COMPASS);
    ImGui::SameLine();
    if (ImGui::TreeNode("Calibration Results")) {
        if (phi_1.empty()) {
            phi_1 = _server->read_scalar("ctl", "drive", "phi_1", std::chrono::milliseconds(100)).value_or("n/a");
        }
        
        if (phi_2.empty()) {
            phi_2 = _server->read_scalar("ctl", "drive", "phi_2", std::chrono::milliseconds(100)).value_or("n/a");
        }
        
        if (phi_3.empty()) {
            phi_3 = _server->read_scalar("ctl", "drive", "phi_3", std::chrono::milliseconds(100)).value_or("n/a");
        }

        static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
        if (ImGui::BeginTable("phi_table", 3, flags)) {
            ImGui::TableSetupColumn("Phi_1 [rad]");
            ImGui::TableSetupColumn("Phi_2 [rad]");
            ImGui::TableSetupColumn("Phi_3 [rad]");
            ImGui::TableHeadersRow();

            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted(phi_1.c_str());
            
            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted(phi_2.c_str());
            
            ImGui::TableSetColumnIndex(2);
            ImGui::TextUnformatted(phi_3.c_str());

            ImGui::EndTable();
        }

        if (ImGui::Button(ICON_MDI_REFRESH " Refresh##calibration")) {
            phi_1.clear();
            phi_2.clear();
            phi_3.clear();
        }

        ImGui::TreePop();
    }
}


} // namespace srmdrive
} // namespace ui
