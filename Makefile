# Default compiler and flags
# Prepend toolchain prefix that you can choose
CXX := $(TOOLCHAIN_PREFIX)g++

INCLUDES := -I../yaml-cpp-0.8.0/include -I../ninTexUtils/include -I../rio/include -I../FFL-Windows/include -I../imgui/backends -I../imgui -Iinclude -Iincludes/helpers -I/usr/lib/gcc/x86_64-linux-gnu/12/include $(INCLUDES)

# --- Definitions

# MLC path is where it expects "sys" and "usr" to be,
# , where it can open the database and resource files
# NDEBUG is required to build on 64 bit (ignore static asserts)
# FFL_NO_OPEN_DATABASE makes sure it doesn't try to open the database by default if you don't need it
# FFL_TEST_DISABLE_MII_COLOR_VERIFY is a testing option that allows you to use out of bound color values in CharInfo
DEFS := -DFFL_MLC_PATH="\"./\"" -DRIO_DEBUG -DNDEBUG -DFFL_NO_OPEN_DATABASE -DFFL_TEST_DISABLE_MII_COLOR_VERIFY -DRIO_AUDIO_USE_SDL_MIXER $(DEFS)

# Binary name which you can change if you want
EXEC := ffl_testing_2_debug64

# --- LDFLAGS

# using pkg-config to make sure packages are installed
# and also because glfw is included as -lglfw3 on some systems and -lglfw on others
# NOTE: glew's pkg-config definition requires "glu" for some reason
# even though glu isn't used, but sometimes it is not available on mingw
# for this reason we add to the PKG_CONFIG_PATH and there is a spoofed glu in there
PKG_CONFIG_PATH := include/.pkg-config-path-dummy-for-makefile/:$(PKG_CONFIG_PATH)
#export PKG_CONFIG_PATH  # not working???

# libraries passed to pkg-config
LIBS := zlib glew glfw3 SDL2_mixer
# use pkg-config output as LDFLAGS and CFLAGS later on
PKG_CONFIG_CFLAGS_CMD := $(TOOLCHAIN_PREFIX)pkg-config --cflags $(LIBS)
$(info pkg-config cflags command: $(PKG_CONFIG_CFLAGS_CMD))
PKG_CONFIG_CFLAGS_OUTPUT := $(shell PKG_CONFIG_PATH="$(PKG_CONFIG_PATH)" $(PKG_CONFIG_CFLAGS_CMD))
$(info $(PKG_CONFIG_CFLAGS_OUTPUT))

PKG_CONFIG_LDFLAGS_CMD := $(TOOLCHAIN_PREFIX)pkg-config --libs $(LIBS)
$(info pkg-config ldflags command: $(PKG_CONFIG_LDFLAGS_CMD))

LDFLAGS := $(shell PKG_CONFIG_PATH="$(PKG_CONFIG_PATH)" $(PKG_CONFIG_LDFLAGS_CMD)) $(LDFLAGS)
$(info $(LDFLAGS))

# $(shell echo "$(LDFLAGS)")

# Check the status and stop if there's an error
#ifneq ($(.SHELLSTATUS), 0)
#$(error pkg-config command failed.\
#please make sure the following \
#libraries are installed: $(LIBS)))
#endif

# --- Platform specific flags

# macOS (doesn't support cross compiling tho)
ifeq ($(shell uname), Darwin)
	LDFLAGS += -framework OpenGL -framework Foundation -framework CoreGraphics -framework AppKit -framework IOKit
	EXEC := ffl_testing_2_debug_apple64
	# clip control is for opengl 4.5 and later and macos only supports 4.1
	DEFS += -DRIO_NO_CLIP_CONTROL
endif
# mingw
# usually these ldflags are added by pkg-config but in this case they weren't
ifeq ($(OS), Windows_NT)
	LDFLAGS += -lopengl32 -lws2_32
# or try to find mingw32 in the CXX
else ifneq (,$(findstring mingw32, $(CXX)))
	# pkg-config should include opengl32 but not socket stuff
	LDFLAGS += -lws2_32
endif

# --- CXXFLAGS and source

# MINGW64 has the default include path in here
#ifneq ($(PKG_CONFIG_SYSTEM_INCLUDE_PATH),)
#INCLUDES += -I$(PKG_CONFIG_SYSTEM_INCLUDE_PATH)
#endif

# Build for debug by default, use C++17
CXXFLAGS := -g -std=c++17 $(CXXFLAGS) $(INCLUDES) $(PKG_CONFIG_CFLAGS_OUTPUT) $(DEFS)

# Source directories
# glob all files in here for now
NINTEXUTILS_SRC := $(shell find ../ninTexUtils/src/ninTexUtils -name '*.c' -o -name '*.cpp')
RIO_SRC := $(shell find ../rio/src -name '*.c' -o -name '*.cpp')
FFL_SRC := $(shell find ../FFL-Windows/src -name '*.c' -o -name '*.cpp')
YAML_PARSER_SRC := $(shell find ../yaml-cpp-0.8.0/src -name '*.c' -o -name '*.cpp')

SHADER ?= src/Shader.cpp
# Main source
SRC := src/main.cpp src/helpers/ui/ThemeMgr.cpp src/helpers/editor/EditorMgr.cpp src/helpers/editor/ConversionMgr.cpp src/helpers/editor/Texture2DUtil.cpp src/helpers/properties/Property.cpp src/helpers/properties/gfx/MeshProperty.cpp src/helpers/properties/examples/ExampleEnumProperty.cpp src/helpers/properties/MiiHeadProperty.cpp src/helpers/common/FFLMgr.cpp src/helpers/common/Node.cpp src/helpers/properties/map/CameraProperty.cpp src/helpers/properties/gfx/PrimitiveProperty.cpp src/helpers/common/NodeMgr.cpp src/helpers/properties/audio/AudioProperty.cpp src/RootTask.cpp ../imgui/backends/imgui_impl_glfw.cpp ../imgui/backends/imgui_impl_opengl3.cpp ../imgui/imgui.cpp ../imgui/imgui_demo.cpp ../imgui/imgui_draw.cpp ../imgui/imgui_tables.cpp ../imgui/imgui_widgets.cpp ../imgui/misc/cpp/imgui_stdlib.cpp $(SHADER)

# Object files
NINTEXUTILS_OBJ := $(NINTEXUTILS_SRC:.c=.o)
NINTEXUTILS_OBJ := $(NINTEXUTILS_OBJ:.cpp=.o)
RIO_OBJ := $(RIO_SRC:.cpp=.o)
FFL_OBJ := $(FFL_SRC:.cpp=.o)
YAML_PARSER_OBJ := $(YAML_PARSER_SRC:.cpp=.o)
OBJ := $(SRC:.cpp=.o)

# --- Targets

# Default target
all: $(EXEC)
# TODO: add git submodule update --init --recursive, maybe?
# TODO: or a reminder if you did not init the submodules and the folders don't have content?

# no_clip_control target
no_clip_control: CXXFLAGS += -DRIO_NO_CLIP_CONTROL
no_clip_control: EXEC := $(EXEC)_no_clip_control
no_clip_control: $(EXEC)_no_clip_control
# clone of exec target bc idk how else to do this
$(EXEC)_no_clip_control: $(NINTEXUTILS_OBJ) $(RIO_OBJ) $(FFL_OBJ) $(OBJ) $(YAML_PARSER_OBJ)
	$(CXX) $^ -o $@ $(CXXFLAGS) $(LDFLAGS)

# Linking the executable
$(EXEC): $(NINTEXUTILS_OBJ) $(RIO_OBJ) $(FFL_OBJ) $(OBJ) $(YAML_PARSER_OBJ)
	$(CXX) $^ -o $@ $(CXXFLAGS) $(LDFLAGS)

# Compiling source files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.c
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up
clean:
	rm -f $(NINTEXUTILS_OBJ) $(RIO_OBJ) $(FFL_OBJ) $(OBJ) $(YAML_PARSER_OBJ) $(EXEC) src/Shader*.o ../../imgui/backends/*.o ../../imgui/backends/*.d ../../imgui/backends/*.map

# Clean within the project folder
cleanin:
	rm -f $(OBJ) $(EXEC) src/Shader*.o

# Phony targets
.PHONY: all clean no_clip_control

# Mode for chainloading Makefile.wut
wut:
	$(MAKE) -f Makefile.wut SRC="$(SRC)" INCLUDES="$(foreach include,$(INCLUDES),$(patsubst -I%,%,$(include)))"