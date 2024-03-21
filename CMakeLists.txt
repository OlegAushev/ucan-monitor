#
cmake_minimum_required(VERSION 3.12)

project(ucan-monitor VERSION 0.1)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake/git-info)
include(GitInfo)
GitInfoSetup()

################################################################################
# Threads
# set(CMAKE_THREAD_PREFER_PTHREAD ON)
# set(THREADS_PREFER_PTHREAD_FLAG ON)
# find_package(Threads REQUIRED)

################################################################################
# Options
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

target_link_libraries(${executable}
    atomic
    # Threads::Threads
    glfw
    ${OPENGL_LIBRARIES}
)

target_link_libraries(${executable}
    git-info
)

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
