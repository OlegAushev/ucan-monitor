#
cmake_minimum_required(VERSION 3.12)

project(ucan-monitor VERSION 1.0)

################################################################################
# Threads
# set(CMAKE_THREAD_PREFER_PTHREAD ON)
# set(THREADS_PREFER_PTHREAD_FLAG ON)
# find_package(Threads REQUIRED)

################################################################################
# Options
option(MULTI_SERVER "Multi-server build" ON)
set(servers srmdrive atv-vcu brake-drive)
set(SERVER "" CACHE STRING "Selected single server")

if (NOT MULTI_SERVER)
    if(NOT SERVER IN_LIST servers)
        message(FATAL_ERROR "Selected server must be one of ${servers}")
    else()
        message(STATUS "Selected single server: ${SERVER}")
    endif()
else()
    message(STATUS "Multi-server build: ON")
endif()

option(FLATPAK_BUILD "Flatpak build configuration" OFF)

################################################################################
# Headers and Sources
set(app_incdirs
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src
)

file(GLOB_RECURSE app_sources CONFIGURE_DEPENDS
    ${PROJECT_SOURCE_DIR}/src/*.c
    ${PROJECT_SOURCE_DIR}/src/*.cpp
)

################################################################################
# Dear ImGUI
find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)

file(GLOB imgui_sources CONFIGURE_DEPENDS
    ${PROJECT_SOURCE_DIR}/include/imgui/*.cpp
    ${PROJECT_SOURCE_DIR}/include/implot/*.cpp
    ${PROJECT_SOURCE_DIR}/include/imguifiledialog/*.cpp 
)
file(GLOB imgui_backend_sources CONFIGURE_DEPENDS
    ${PROJECT_SOURCE_DIR}/include/imgui/backends/*glfw*.cpp
    ${PROJECT_SOURCE_DIR}/include/imgui/backends/*opengl3*.cpp
)

set(imgui_incdirs 
    ${PROJECT_SOURCE_DIR}/include/imgui
    ${PROJECT_SOURCE_DIR}/include/implot
)

################################################################################
# Executable
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(executable ${CMAKE_PROJECT_NAME})

add_executable(${executable}
    ${app_sources}
    ${imgui_sources}
    ${imgui_backend_sources}
)

if(NOT MULTI_SERVER)
    target_compile_definitions(${executable} PRIVATE
        SINGLE_SERVER="${SERVER}"
    ) 
endif()

target_compile_definitions(${executable} PRIVATE
    $<$<CONFIG:Debug>:DEBUG>
    IMGUI_USE_WCHAR32
)

if(FLATPAK_BUILD)
    message(STATUS "Flatpak build: ON")
    target_compile_definitions(${executable} PRIVATE FLATPAK_BUILD)
else()
    message(STATUS "Flatpak build: OFF")
endif()

target_include_directories(${executable} PRIVATE
    ${app_incdirs}
    ${imgui_incdirs}
)

target_link_libraries(${executable} PRIVATE
    atomic
    # Threads::Threads
    glfw
    ${OPENGL_LIBRARIES}
)

if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "13")
    message(STATUS "Compiler version ${CMAKE_CXX_COMPILER_VERSION} doesn`t support std::format, fmtlib is required")
    find_package(fmt REQUIRED)
    target_link_libraries(${executable} PRIVATE
        fmt::fmt
    )
endif()

################################################################################
# Compiler Options and Features
target_compile_options(${executable} PRIVATE
    #$<$<CONFIG:Debug>:-Og -g3 -ggdb>
    #$<$<CONFIG:Release>:-Og -g0>
    -Wall
    -Wextra
    -Wpedantic
    -fdiagnostics-color=always
)

target_compile_features(${executable} PRIVATE
    c_std_11
    cxx_std_23
)

################################################################################
# Git Version Tracking
add_subdirectory(cmake/git-version-tracking)

target_link_directories(${executable} PRIVATE
    ${CMAKE_BINARY_DIR}/cmake/git-version-tracking
)

target_link_libraries(${executable} PRIVATE
    git-version-tracking
)

################################################################################
# Misc Options
set(CMAKE_VERBOSE_MAKEFILE OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

################################################################################
# Set install target for flatpak-builder
install(TARGETS ${CMAKE_PROJECT_NAME}
    RUNTIME
    DESTINATION /app/bin
)

install(DIRECTORY ${PROJECT_SOURCE_DIR}/assets
    DESTINATION /app   
)
