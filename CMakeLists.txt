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

# Link directories
link_directories(${GLEW_LIB_DIR})
link_directories(${GLFW_LIB_DIR})

# Collect source files
file(GLOB_RECURSE SOURCES src/main.cpp src/*.cpp include/*.h)
file(GLOB_RECURSE RIO_SOURCES lib/rio/src/*.cpp lib/rio/include/*.h)

include_directories(include/ lib/rio/include)

# Define compiler options
add_definitions(-DRIO_DEBUG)

# Add executable
add_executable(CMakeTarget ${SOURCES} ${RIO_SOURCES})

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
