cmake_minimum_required(VERSION 3.0.0)
project(lab1 VERSION 1.0.0)

set(CMAKE_PREFIX_PATH third_party/GLFW)
find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)

include_directories(./third_party)
include_directories(./include)
include_directories(./include/imgui)
include_directories(./include/polynomial)

add_executable(lab1_console
src/console/main.c
src/polynomial.c
)
add_executable(lab1_gui
src/gui/main.cpp
src/gui/window.cpp
src/polynomial.c
third_party/imgui/imgui.cpp
third_party/imgui/imgui_widgets.cpp
third_party/imgui/imgui_tables.cpp
third_party/imgui/imgui_impl_opengl3.cpp
third_party/imgui/imgui_impl_glfw.cpp
third_party/imgui/imgui_draw.cpp
third_party/imgui/imgui_demo.cpp
)
target_link_libraries(lab1_gui OpenGL::GL glfw)
