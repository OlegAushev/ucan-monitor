#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <implot.h>

//#include <icons_font_awesome/IconsFontAwesome6.h>
#include <icons/IconsMaterialDesignIcons.h>

#include <ui/util/style.h>

#include <memory>
#include <ucanopen/client/client.h>

#include <ui/mainview/mainview.h>
#include <ui/log/log.h>
#include <ui/serverselector/serverselector.h>
#include <ui/serversetuppanel/serversetuppanel.h>
#include <ui/watchpanel/watchpanel.h>
#include <ui/watchplot/watchplot.h>

#include <ui/ucanopen_servers/moyka/panel/panel.h>

#include <ui/ucanopen_servers/srmdrive/controlpanel/controlpanel.h>
#include <ui/ucanopen_servers/srmdrive/datapanel/datapanel.h>
#include <ui/ucanopen_servers/srmdrive/statuspanel/statuspanel.h>

#include <ui/ucanopen_servers/atvvcu/dashboard/dashboard.h>
#include <ui/ucanopen_servers/atvvcu/bmspanel/bmspanel.h>
#include <ui/ucanopen_servers/atvvcu/pdmpanel/pdmpanel.h>
#include <ui/ucanopen_servers/atvvcu/motorcontrolpanel/motorcontrolpanel.h>
#include <ui/ucanopen_servers/atvvcu/motordatapanel/motordatapanel.h>

#include <ui/ucanopen_servers/brkdrive/controlpanel/controlpanel.h>
#include <ui/ucanopen_servers/brkdrive/datapanel/datapanel.h>
#include <ui/ucanopen_servers/brkdrive/statuspanel/statuspanel.h>

#include <ui/ucanopen_servers/loco/controlpanel/controlpanel.h>
#include <ui/ucanopen_servers/loco/datapanel/datapanel.h>
#include <ui/ucanopen_servers/loco/statuspanel/statuspanel.h>

#include <iostream>
#include <fstream>

#include <csv_writer/csv_writer.h>
#include <reference_manager/reference_manager.h>


const std::vector<std::string> server_names = {"project-moyka", "srmdrive", "atv-vcu", "brake-drive", "loco-drive"};


static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}


int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv) {
    bsclog::add_sink(std::shared_ptr<std::ostream>(&std::cout, [](void*){}));
    auto logfile = std::make_shared<std::ofstream>("log.txt");
    bsclog::add_sink(logfile);
    auto gui_log = std::make_shared<ui::Log>(); 
    bsclog::add_sink(gui_log->stream());
    bsclog::success("Initialized bsclog. Sink count: {}", bsclog::sink_count());  
    
    glfwSetErrorCallback(glfw_error_callback);
    if(!glfwInit()){
        return 1;
    }

    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1280, 720, "uCAN Monitor", nullptr, nullptr);
    if (window == nullptr) {
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;   // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;    // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;       // Enable Docking

    // Style
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 0.0f;
    style.WindowBorderSize = 1.0f;
    style.FrameRounding = 7.0f;
    style.FrameBorderSize = 1.0f;

    // Fonts
    float base_fontsize = 20.0f;
    float icon_fontsize = base_fontsize;
    io.Fonts->AddFontFromFileTTF("../assets/fonts/SourceCodePro-Regular.otf", base_fontsize, NULL, io.Fonts->GetGlyphRangesDefault());

        // merge in icons from Font Awesome
    static const ImWchar icons_ranges[] = {ICON_MIN_MDI, ICON_MAX_MDI, 0};
    ImFontConfig icons_config; 
    icons_config.MergeMode = true; 
    icons_config.PixelSnapH = true; 
    icons_config.GlyphMinAdvanceX = icon_fontsize;
    icons_config.GlyphOffset = {0, 2};
    io.Fonts->AddFontFromFileTTF("../assets/fonts/" FONT_ICON_FILE_NAME_MDI, icon_fontsize, &icons_config, icons_ranges);

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

#ifndef SINGLE_SERVER
    // Server Selection Loop
    while (!glfwWindowShouldClose(window) && !ui::ServerSelector::instance().server_is_selected()) {
        // Poll and handle events (inputs, window resize, etc.)
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ui::ServerSelector::instance().show(server_names, ImVec2{400, 260});

        // Rendering
        // (Your code clears your framebuffer, renders your other stuff etc.)
        ImGui::Render();


        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }
#endif

    // Server Creation
    auto can_socket = std::make_shared<can::Socket>();
    auto ucanopen_client = std::make_shared<ucanopen::Client>(ucanopen::NodeId(127), can_socket);

    std::vector<std::shared_ptr<ui::View>> views;
    std::vector<std::shared_ptr<ui::View>> tools;
    views.push_back(gui_log);

    std::shared_ptr<ui::ServerSetupPanel> serversetuppanel;
    std::shared_ptr<ui::WatchPanel> watchpanel;
    std::vector<std::shared_ptr<ui::WatchPlot>> watchplots;

#ifndef SINGLE_SERVER
    std::string server_name = ui::ServerSelector::instance().selected_server();
#else
    std::string  server_name = SINGLE_SERVER;
#endif
    if (server_name == "project-moyka") {
        glfwMaximizeWindow(window);
        glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_FALSE);

        auto moyka_server = std::make_shared<moyka::Server>(can_socket, ucanopen::NodeId(0x01), server_name);
        ucanopen_client->register_server(moyka_server);
        
        auto panel = std::make_shared<ui::moyka::Panel>(moyka_server, ICON_MDI_GAUGE" Panel", ICON_MDI_GAUGE, "true");
        watchpanel = std::make_shared<ui::WatchPanel>(moyka_server, ICON_MDI_TABLE_EYE" Watch SDO", "Watch SDO", false);
        serversetuppanel = std::make_shared<ui::ServerSetupPanel>(moyka_server, ICON_MDI_TOOLS" Setup", "Setup", false);
        gui_log->toggle(false);
        
        views.push_back(panel);
        views.push_back(watchpanel);
        views.push_back(serversetuppanel);
    } if (server_name == "srmdrive") {
        auto srmdrive_server = std::make_shared<srmdrive::Server>(can_socket, ucanopen::NodeId(0x01), server_name);
        ucanopen_client->register_server(srmdrive_server);
        
        auto controlpanel = std::make_shared<ui::srmdrive::ControlPanel>(srmdrive_server, ICON_MDI_GAMEPAD_OUTLINE" Control", "Control", true);
        watchpanel = std::make_shared<ui::WatchPanel>(srmdrive_server, ICON_MDI_TABLE_EYE" Watch SDO", "Watch SDO", true);
        auto datapanel = std::make_shared<ui::srmdrive::DataPanel>(srmdrive_server, ICON_MDI_TABLE" TPDO Data", "TPDO Data", true);
        auto statuspanel = std::make_shared<ui::srmdrive::StatusPanel>(srmdrive_server, ICON_MDI_INFORMATION_OUTLINE" Status", "Status", true);
        serversetuppanel = std::make_shared<ui::ServerSetupPanel>(srmdrive_server, ICON_MDI_TOOLS" Setup", "Setup", false);
        
        views.push_back(controlpanel);
        views.push_back(watchpanel);
        views.push_back(datapanel);
        views.push_back(statuspanel);
        views.push_back(serversetuppanel);

        watchplots.push_back(std::make_shared<ui::WatchPlot>(srmdrive_server, "Plot 1", "Watch Plot 1", true));
        watchplots.push_back(std::make_shared<ui::WatchPlot>(srmdrive_server, "Plot 2", "Watch Plot 2", false));
        watchplots.push_back(std::make_shared<ui::WatchPlot>(srmdrive_server, "Plot 3", "Watch Plot 3", false));
        watchplots.push_back(std::make_shared<ui::WatchPlot>(srmdrive_server, "Plot 4", "Watch Plot 4", false));

    } else if (server_name == "atv-vcu") {
        auto atvvcu_server = std::make_shared<atvvcu::Server>(can_socket, ucanopen::NodeId(0x0A), server_name);
        ucanopen_client->register_server(atvvcu_server);

        watchpanel = std::make_shared<ui::WatchPanel>(atvvcu_server, ICON_MDI_TABLE_EYE" Watch SDO", "Watch SDO", true);
        auto systempanel = std::make_shared<ui::atvvcu::Dashboard>(atvvcu_server, ICON_MDI_GAUGE" Dashboard", "Dashboard", true);
        auto bmspanel = std::make_shared<ui::atvvcu::BmsPanel>(atvvcu_server, ICON_MDI_CAR_BATTERY" BMS", "BMS", true);
        auto pdmpanel = std::make_shared<ui::atvvcu::PdmPanel>(atvvcu_server, ICON_MDI_CAR_ELECTRIC_OUTLINE" PDM", "PDM", true);
        auto motordatapanel = std::make_shared<ui::atvvcu::MotorDataPanel>(atvvcu_server, ICON_MDI_TABLE" Motor Data", "Motor Data", true);
        auto motorcontrolpanel = std::make_shared<ui::atvvcu::MotorControlPanel>(atvvcu_server, ICON_MDI_GAMEPAD_OUTLINE" Motor Control", "Motor Control", false);
        serversetuppanel = std::make_shared<ui::ServerSetupPanel>(atvvcu_server, ICON_MDI_TOOLS" Setup", "Setup", false);

        views.push_back(watchpanel);
        views.push_back(systempanel);
        views.push_back(bmspanel);
        views.push_back(pdmpanel);
        views.push_back(motordatapanel);
        views.push_back(motorcontrolpanel);
        views.push_back(serversetuppanel);

        watchplots.push_back(std::make_shared<ui::WatchPlot>(atvvcu_server, "Plot 1", "Watch Plot 1", true));
        watchplots.push_back(std::make_shared<ui::WatchPlot>(atvvcu_server, "Plot 2", "Watch Plot 2", false));
        watchplots.push_back(std::make_shared<ui::WatchPlot>(atvvcu_server, "Plot 3", "Watch Plot 3", false));
        watchplots.push_back(std::make_shared<ui::WatchPlot>(atvvcu_server, "Plot 4", "Watch Plot 4", false));
    
    } else if (server_name == "brake-drive") {
        auto brkdrive_server = std::make_shared<brkdrive::Server>(can_socket, ucanopen::NodeId(0x01), server_name);
        ucanopen_client->register_server(brkdrive_server);

        auto controlpanel = std::make_shared<ui::brkdrive::ControlPanel>(brkdrive_server, ICON_MDI_GAMEPAD_OUTLINE" Control", "Control", true);
        watchpanel = std::make_shared<ui::WatchPanel>(brkdrive_server, ICON_MDI_TABLE_EYE" Watch SDO", "Watch SDO", true);
        auto datapanel = std::make_shared<ui::brkdrive::DataPanel>(brkdrive_server, ICON_MDI_TABLE" TPDO Data", "TPDO Data", true);
        auto statuspanel = std::make_shared<ui::brkdrive::StatusPanel>(brkdrive_server, ICON_MDI_INFORMATION_OUTLINE" Status", "Status", true);
        serversetuppanel = std::make_shared<ui::ServerSetupPanel>(brkdrive_server, ICON_MDI_TOOLS" Setup", "Setup", false);
        
        views.push_back(controlpanel);
        views.push_back(watchpanel);
        views.push_back(datapanel);
        views.push_back(statuspanel);
        views.push_back(serversetuppanel);
        
        watchplots.push_back(std::make_shared<ui::WatchPlot>(brkdrive_server, "Plot 1", "Watch Plot 1", true));
        watchplots.push_back(std::make_shared<ui::WatchPlot>(brkdrive_server, "Plot 2", "Watch Plot 2", false));
        watchplots.push_back(std::make_shared<ui::WatchPlot>(brkdrive_server, "Plot 3", "Watch Plot 3", false));
        watchplots.push_back(std::make_shared<ui::WatchPlot>(brkdrive_server, "Plot 4", "Watch Plot 4", false));
    } else if (server_name == "loco-drive") {
        auto loco_server = std::make_shared<loco::Server>(can_socket, ucanopen::NodeId(0x01), server_name);
        ucanopen_client->register_server(loco_server);

        auto controlpanel = std::make_shared<ui::loco::ControlPanel>(loco_server, ICON_MDI_GAMEPAD_OUTLINE" Control", "Control", true);
        auto datapanel = std::make_shared<ui::loco::DataPanel>(loco_server, ICON_MDI_TABLE" TPDO Data", "TPDO Data", true);
        watchpanel = std::make_shared<ui::WatchPanel>(loco_server, ICON_MDI_TABLE_EYE" Watch SDO", "Watch SDO", true);
        auto statuspanel = std::make_shared<ui::loco::StatusPanel>(loco_server, ICON_MDI_INFORMATION_OUTLINE" Status", "Status", true);

        views.push_back(controlpanel);
        views.push_back(watchpanel);
        views.push_back(datapanel);
        views.push_back(statuspanel);

        watchplots.push_back(std::make_shared<ui::WatchPlot>(loco_server, "Plot 1", "Watch Plot 1", true));
        watchplots.push_back(std::make_shared<ui::WatchPlot>(loco_server, "Plot 2", "Watch Plot 2", false));
        watchplots.push_back(std::make_shared<ui::WatchPlot>(loco_server, "Plot 3", "Watch Plot 3", false));
        watchplots.push_back(std::make_shared<ui::WatchPlot>(loco_server, "Plot 4", "Watch Plot 4", false));
    } else {
        // TODO Error
    }

    // GUI Creation
    auto options = std::make_shared<ui::Options>(can_socket, ucanopen_client);
    auto mainview = std::make_shared<ui::MainView>(options, views, tools, watchplots);

    // Main View Loop
    while (!glfwWindowShouldClose(window) && !mainview->should_close()) {
        // Poll and handle events (inputs, window resize, etc.)
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        mainview->draw();

        // Rendering
        // (Your code clears your framebuffer, renders your other stuff etc.)
        ImGui::Render();


        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

