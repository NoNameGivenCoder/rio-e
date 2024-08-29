cmake_minimum_required(VERSION 3.10)
project(RIOe)

# Define directories
set(GLEW_INCLUDE_DIR "../glew/include")
set(GLEW_LIB_DIR "../glew/lib/Release/x64")

set(GLFW_INCLUDE_DIR "../glfw/include")
set(GLFW_LIB_DIR "../glfw/lib-vc2022")

# Include directories
include_directories(${GLEW_INCLUDE_DIR})
include_directories(${GLFW_INCLUDE_DIR})

include_directories(include/)

include_directories(lib/rio/include)

include_directories(lib/imgui)
include_directories(lib/imgui/backends)

# Link directories
link_directories(${GLEW_LIB_DIR})
link_directories(${GLFW_LIB_DIR})

# Collect source files
file(GLOB_RECURSE SOURCES src/main.cpp src/*.cpp src/UI/*.cpp include/*.h)
file(GLOB_RECURSE RIO_SOURCES lib/rio/src/*.cpp lib/rio/include/*.h)
file(GLOB_RECURSE IMGUI_SOURCES lib/imgui/imgui.cpp lib/imgui/imgui_demo.cpp lib/imgui/imgui_draw.cpp lib/imgui/imgui_tables.cpp lib/imgui/imgui_widgets.cpp lib/imgui/backends/imgui_impl_opengl3.cpp lib/imgui/backends/imgui_impl_glfw.cpp lib/imgui/*.h)

# Define compiler options
add_definitions(-DRIO_DEBUG)

# Add executable
add_executable(CMakeTarget ${SOURCES} ${RIO_SOURCES} ${IMGUI_SOURCES})

# Link libraries
target_link_libraries(CMakeTarget
    opengl32
    glew32
    glfw3
)

# Set C++ standard
if (CMAKE_VERSION VERSION_GREATER 3.12)
    set_property(TARGET CMakeTarget PROPERTY CXX_STANDARD 20)
else()
    set(CMAKE_CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
