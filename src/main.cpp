#include <GLFW/glfw3.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <imgui.h>
#include <implot.h>

//#include <icons_font_awesome/IconsFontAwesome6.h>
#include <icons/IconsMaterialDesignIcons.h>

#include <ui/util/style.h>

#include <memory>
#include <ucanopen/client/client.h>

#include <ui/log/log.h>
#include <ui/mainview/mainview.h>
#include <ui/serverselector/serverselector.h>
#include <ui/serversetuppanel/serversetuppanel.h>
#include <ui/watchpanel/watchpanel.h>
#include <ui/watchplot/watchplot.h>

#include <ui/pdopanel/pdopanel.h>
#include <ui/statuspanel/statuspanel.h>
#include <ui/ucanopen_servers/adptetk/controlpanel/controlpanel.hpp>

#include <ucanopen/od/description.h>

#include <fstream>
#include <iostream>

#include <csv_writer/csv_writer.h>

const std::vector<std::string> server_names = {"adpt-etk-inverter"};

static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv) {
    bsclog::add_sink(std::shared_ptr<std::ostream>(&std::cout, [](void*) {}));
    auto logfile = std::make_shared<std::ofstream>("log.txt");
    bsclog::add_sink(logfile);
    auto gui_log = std::make_shared<ui::Log>();
    bsclog::add_sink(gui_log->stream());
    bsclog::success("Initialized bsclog. Sink count: {}", bsclog::sink_count());

    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        return 1;
    }

    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    // Create window with graphics context
    GLFWwindow* window =
            glfwCreateWindow(1280, 720, "uCAN Monitor", nullptr, nullptr);
    if (window == nullptr) {
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |=
            ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |=
            ImGuiConfigFlags_NavEnableGamepad;        // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking

    // Style
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 0.0f;
    style.WindowBorderSize = 1.0f;
    style.FrameRounding = 7.0f;
    style.FrameBorderSize = 1.0f;

    // Fonts
    float base_fontsize = 20.0f;
    float icon_fontsize = base_fontsize;

    // ImGui puts in its atlas only the glyphs of these ranges and draws «?»
    // for any other: Latin and Cyrillic, and what the texts of the program
    // and of the descriptions take besides, dashes and the ellipsis, Greek,
    // №, arrows and the signs of comparison. The atlas keeps a pointer to
    // the ranges.
    static const ImWchar text_extra_ranges[] = {
            0x0370, 0x03FF, // Greek
            0x2000, 0x206F, // General Punctuation
            0x2100, 0x214F, // Letterlike Symbols
            0x2190, 0x21FF, // Arrows
            0x2200, 0x22FF, // Mathematical Operators
            0,
    };
    static ImVector<ImWchar> text_ranges;
    ImFontGlyphRangesBuilder text_ranges_builder;
    text_ranges_builder.AddRanges(io.Fonts->GetGlyphRangesCyrillic());
    text_ranges_builder.AddRanges(text_extra_ranges);
    text_ranges_builder.BuildRanges(&text_ranges);
    io.Fonts->AddFontFromFileTTF("../assets/fonts/IBMPlexMono-Regular.ttf",
                                 base_fontsize,
                                 NULL,
                                 text_ranges.Data);

    // merge in icons from Font Awesome
    static const ImWchar icons_ranges[] = {ICON_MIN_MDI, ICON_MAX_MDI, 0};
    ImFontConfig icons_config;
    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;
    icons_config.GlyphMinAdvanceX = icon_fontsize;
    icons_config.GlyphOffset = {0, 2};
    io.Fonts->AddFontFromFileTTF("../assets/fonts/" FONT_ICON_FILE_NAME_MDI,
                                 icon_fontsize,
                                 &icons_config,
                                 icons_ranges);

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

#ifndef SINGLE_SERVER
    // Server Selection Loop
    while (!glfwWindowShouldClose(window) &&
           !ui::ServerSelector::instance().server_is_selected()) {
        // Poll and handle events (inputs, window resize, etc.)
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ui::ServerSelector::instance().show(server_names, ImVec2{400, 300});

        // Rendering
        // (Your code clears your framebuffer, renders your other stuff etc.)
        ImGui::Render();

        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w,
                     clear_color.y * clear_color.w,
                     clear_color.z * clear_color.w,
                     clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }
#endif

    // Server Creation
    auto can_socket = std::make_shared<can::Socket>();
    auto ucanopen_client =
            std::make_shared<ucanopen::Client>(ucanopen::NodeId(127),
                                               can_socket);

    std::optional<ImGuiWindowFlags> mainview_window_flags;
    std::vector<std::shared_ptr<ui::View>> views;
    std::vector<std::shared_ptr<ui::View>> tools;
    views.push_back(gui_log);

    std::shared_ptr<ui::ServerSetupPanel> serversetuppanel;
    std::shared_ptr<ui::WatchPanel> watchpanel;
    std::vector<std::shared_ptr<ui::WatchPlot>> watchplots;

#ifndef SINGLE_SERVER
    std::string server_name = ui::ServerSelector::instance().selected_server();
#else
    std::string server_name = SINGLE_SERVER;
#endif
    if (server_name == "adpt-etk-inverter") {
        // The server decodes the frames of the device by the description of
        // its dictionary, which lies with the program, as the fonts do.
        std::string const description_path =
                "../assets/od/adpt-etk-inverter.od.jsonl";
        auto description = ucanopen::od::load(description_path);
        if (!description.has_value()) {
            bsclog::error("Не удалось загрузить описание словаря {}: {}.",
                          description_path,
                          description.error());
        } else {
            bsclog::success("Загружено описание словаря {}: {}, отпечаток "
                            "0x{:08X}.",
                            description_path,
                            (*description)->device(),
                            (*description)->fingerprint());

            auto adptetk_server = std::make_shared<adptetk::Server>(
                    can_socket,
                    ucanopen::NodeId(0x01),
                    server_name,
                    *description);
            ucanopen_client->register_server(adptetk_server);

            auto controlpanel = std::make_shared<ui::adptetk::ControlPanel>(
                    adptetk_server,
                    ICON_MDI_GAMEPAD_OUTLINE " Управление",
                    "Управление",
                    true);

            auto statuspanel = std::make_shared<ui::StatusPanel>(
                    adptetk_server,
                    ICON_MDI_INFORMATION_OUTLINE " Статус",
                    "Статус",
                    true);

            watchpanel = std::make_shared<ui::WatchPanel>(adptetk_server,
                                                          ICON_MDI_TABLE_EYE
                                                          " Набл. Переменные",
                                                          "Набл. Переменные",
                                                          true);

            auto pdopanel = std::make_shared<ui::PdoPanel>(
                    adptetk_server,
                    ICON_MDI_TABLE " Данные PDO",
                    "Данные PDO",
                    true);

            serversetuppanel = std::make_shared<ui::ServerSetupPanel>(
                    adptetk_server,
                    ICON_MDI_TOOLS " Настройка",
                    "Настройка",
                    false);

            views.push_back(controlpanel);
            views.push_back(statuspanel);
            views.push_back(watchpanel);
            views.push_back(pdopanel);
            views.push_back(serversetuppanel);

            watchplots.push_back(
                    std::make_shared<ui::WatchPlot>(adptetk_server,
                                                    "Панель диаграмм 1",
                                                    "Панель Диаграмм 1",
                                                    true));
            watchplots.push_back(
                    std::make_shared<ui::WatchPlot>(adptetk_server,
                                                    "Панель диаграмм 2",
                                                    "Панель диаграмм 2",
                                                    false));
            watchplots.push_back(
                    std::make_shared<ui::WatchPlot>(adptetk_server,
                                                    "Панель диаграмм 3",
                                                    "Панель диаграмм 3",
                                                    false));
            watchplots.push_back(
                    std::make_shared<ui::WatchPlot>(adptetk_server,
                                                    "Панель диаграмм 4",
                                                    "Панель диаграмм 4",
                                                    false));
        }
    } else {
        // TODO Error
    }

    // GUI Creation
    auto options = std::make_shared<ui::Options>(can_socket, ucanopen_client);
    auto mainview = std::make_shared<ui::MainView>(
            options,
            views,
            tools,
            watchplots,
            mainview_window_flags.value_or(ui::MainView::default_window_flags));

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
        glClearColor(clear_color.x * clear_color.w,
                     clear_color.y * clear_color.w,
                     clear_color.z * clear_color.w,
                     clear_color.w);
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
