#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>

#include <icons_font_awesome/IconsFontAwesome6.h>

#include <ui/log/log.h>
#include <ui/mainview/mainview.h>
#include <ui/options/options.h>
#include <ui/serverselector/serverselector.h>


#include <iostream>
#include <fstream>


#include <ucanopen/client/client.h>
#include <ucanopen_devices/srmdrive/server/srmdrive_server.h>
#include <ucanopen_devices/crd600/server/crd600_server.h>
#include <ucanopen_devices/launchpad/server/launchpad_server.h>
#include <ucanopen_devices/atvvcu/server/atvvcu_server.h>


const std::vector<std::string> server_names = {"srmdrive", "atv-vcu"};


static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}


int main(int argc, char** argv) {
    bsclog::add_sink(std::shared_ptr<std::ostream>(&std::cout, [](void*){}));
    auto logfile = std::make_shared<std::ofstream>("logfile.txt");
    bsclog::add_sink(logfile);
    bsclog::add_sink(ui::Log::instance().stream());
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
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;   // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;    // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;       // Enable Docking

    // Fonts
    float base_fontsize = 18.0f;
    float icon_fontsize = base_fontsize;// * 2.0f / 3.0f;
    io.Fonts->AddFontFromFileTTF("../assets/fonts/SourceCodePro-Regular.otf", base_fontsize, NULL, io.Fonts->GetGlyphRangesDefault());

        // merge in icons from Font Awesome
    static const ImWchar icons_ranges[] = {ICON_MIN_FA, ICON_MAX_16_FA, 0};
    ImFontConfig icons_config; 
    icons_config.MergeMode = true; 
    icons_config.PixelSnapH = true; 
    icons_config.GlyphMinAdvanceX = icon_fontsize;
    io.Fonts->AddFontFromFileTTF("../assets/fonts/FontAwesome.otf", icon_fontsize, &icons_config, icons_ranges);

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    bool show_demo_window = true;
    bool show_another_window = false;

    // Server Selection Loop
    while (!glfwWindowShouldClose(window) && !ui::ServerSelector::instance().server_is_selected()) {
        // Poll and handle events (inputs, window resize, etc.)
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ui::ServerSelector::instance().show(server_names);

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

    // Server Creation
    auto can_socket = std::make_shared<can::Socket>();
    auto ucanopen_client = std::make_shared<ucanopen::Client>(ucanopen::NodeId(127), can_socket);

    auto server_name = ui::ServerSelector::instance().selected_server();
    if (server_name == "srmdrive") {
        auto srmdrive_server = std::make_shared<srmdrive::Server>(can_socket, ucanopen::NodeId(0x01), server_name);
        ucanopen_client->register_server(srmdrive_server);
    } else if (server_name == "atv-vcu") {
        auto atvvcu_server = std::make_shared<atvvcu::Server>(can_socket, ucanopen::NodeId(0x0A), server_name);
        ucanopen_client->register_server(atvvcu_server);
    } else {
        goto cleanup;
    }

    ui::Options::instance().init(can_socket, ucanopen_client);
    
    // TODO

    while (!glfwWindowShouldClose(window) && !ui::MainView::instance().should_close()) {
        // Poll and handle events (inputs, window resize, etc.)
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ui::MainView::instance().draw();

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

cleanup:
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}


// used in VALA code
bool backend_is_ready = false;
const char* backend_ucanopen_server;
const char* backend_ucanopen_server_list[5] = {"SRM-Drive-80", "CRD600", "LaunchPad", "BMS-Main", "ATV-VCU"};
const char* backend_ucanopen_server_config_category;


namespace {
std::thread thread_main;
std::promise<void> signal_exit_main;
}


// extern "C" 
// int backend_main_loop(std::future<void> signal_exit) {
//     Log() << "Started backend main loop thread. Thread id: " << std::this_thread::get_id() << ".\n" << LogPrefix::ok;





//     std::shared_ptr<srmdrive::Server> srmdrive_server;
//     std::shared_ptr<crd600::Server> crd600_server;
//     std::shared_ptr<launchpad::Server> launchpad_server;
//     std::shared_ptr<bmsmain::Server> bmsmain_server;
//     std::shared_ptr<atvvcu::Server> atvvcu_server;

//     std::string server_name(backend_ucanopen_server);
//     if (server_name == "SRM-Drive-80") {
//         srmdrive_server = std::make_shared<srmdrive::Server>(can_socket, ucanopen::NodeId(0x01), server_name);
//         ucanopen_client->register_server(srmdrive_server);
//         api::register_srmdrive_server(srmdrive_server);
//     } else if (server_name == "CRD600") {
//         crd600_server = std::make_shared<crd600::Server>(can_socket, ucanopen::NodeId(0x01), server_name);
//         ucanopen_client->register_server(crd600_server);
//         api::register_crd600_server(crd600_server);
//     } else if (server_name == "LaunchPad") {
//         launchpad_server = std::make_shared<launchpad::Server>(can_socket, ucanopen::NodeId(0x01), server_name);
//         ucanopen_client->register_server(launchpad_server);

//         auto callback_create_tpdo1 = [launchpad_server](){ return launchpad_server->create_client_tpdo1(); };
//         auto callback_create_tpdo2 = [launchpad_server](){ return launchpad_server->create_client_tpdo2(); };
//         auto callback_create_tpdo3 = [launchpad_server](){ return launchpad_server->create_client_tpdo3(); };
//         auto callback_create_tpdo4 = [launchpad_server](){ return launchpad_server->create_client_tpdo4(); };

//         ucanopen_client->register_tpdo(ucanopen::TpdoType::tpdo1, std::chrono::milliseconds(50), callback_create_tpdo1);
//         ucanopen_client->register_tpdo(ucanopen::TpdoType::tpdo2, std::chrono::milliseconds(100), callback_create_tpdo2);
//         ucanopen_client->register_tpdo(ucanopen::TpdoType::tpdo3, std::chrono::milliseconds(250), callback_create_tpdo3);
//         ucanopen_client->register_tpdo(ucanopen::TpdoType::tpdo4, std::chrono::milliseconds(1000), callback_create_tpdo4);

//         api::register_launchpad_server(launchpad_server);
//     } else if (server_name == "BMS-Main") {
//         bmsmain_server = std::make_shared<bmsmain::Server>(can_socket, ucanopen::NodeId(0x20), server_name);
//         ucanopen_client->register_server(bmsmain_server);
//         api::register_bmsmain_server(bmsmain_server);
//     } else if (server_name == "ATV-VCU") {
//         atvvcu_server = std::make_shared<atvvcu::Server>(can_socket, ucanopen::NodeId(0x01), server_name);
//         ucanopen_client->register_server(atvvcu_server);
//         api::register_atvvcu_server(atvvcu_server);
//     }

//     backend_ucanopen_server_config_category = ucanopen_client->server(server_name)->dictionary().config.config_category.data();

//     Log() << "Backend ready.\n" << LogPrefix::ok;
//     backend_is_ready = true;

//     while (signal_exit.wait_for(std::chrono::milliseconds(100)) == std::future_status::timeout) {
//         // wait for promise to exit
//     }

//     Log() << "Stopped backend main loop thread.\n" << LogPrefix::ok;
//     return 0;
// }


// extern "C"
// int backend_main_enter() {
//     Log() << "GUI thread id: " << std::this_thread::get_id() << ".\n" << LogPrefix::align;
//     Log() << "Starting new thread for backend main loop...\n" << LogPrefix::align;

//     std::future<void> signal_exit = signal_exit_main.get_future();
//     thread_main = std::thread(backend_main_loop, std::move(signal_exit));
//     return 0;
// }


// extern "C"
// void backend_main_exit() {
//     Log() << "Sending signal to backend main loop thread to stop...\n" << LogPrefix::align;

//     signal_exit_main.set_value();
//     thread_main.join();
// }

