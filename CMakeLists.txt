# CMakeList.txt : CMake project for RIO(e), include source and define
# project specific logic here.
#

file(GLOB_RECURSE SOURCES src/main.cpp src/*.cpp include/*.h)

# Add source to this project's executable.
include_directories(include/)
add_executable (CMakeTarget ${SOURCES})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET CMakeTarget PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
