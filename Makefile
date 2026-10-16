CXX ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Iinclude

BUILD_DIR := build
CONSOLE_TARGET := $(BUILD_DIR)/QuanLySanBong
GUI_TARGET := $(BUILD_DIR)/QuanLySanBong_GUI

# Phần console giữ nguyên danh sách mã nguồn cũ để không kéo mã GUI vào.
CONSOLE_SRC_DIRS := src src/Core src/Account src/Field src/Booking src/Customer src/Tournament src/Storage src/UI
CONSOLE_SOURCES := $(filter-out src/main_gui.cpp,$(foreach dir,$(CONSOLE_SRC_DIRS),$(wildcard $(dir)/*.cpp)))
CONSOLE_OBJECTS := $(CONSOLE_SOURCES:.cpp=.o)

# Mọi mã nghiệp vụ, trừ hai entry point, được liên kết vào ứng dụng GUI.
BUSINESS_SOURCES := $(filter-out src/main.cpp src/main_gui.cpp,$(foreach dir,$(CONSOLE_SRC_DIRS),$(wildcard $(dir)/*.cpp)))
GUI_SOURCES := src/main_gui.cpp $(shell find src/GUI -type f -name '*.cpp' 2>/dev/null)
IMGUI_SOURCES := $(wildcard third_party/imgui/*.cpp) \
                third_party/imgui/backends/imgui_impl_glfw.cpp \
                third_party/imgui/backends/imgui_impl_opengl3.cpp

ifeq ($(OS),Windows_NT)
    GUI_TARGET := $(GUI_TARGET).exe
    GLFW_INCLUDE := -Ithird_party/glfw/include
    GLFW_LIB := -Lthird_party/glfw/lib -lglfw3 -lopengl32 -lgdi32
else
    GLFW_INCLUDE :=
    GLFW_LIB := -lglfw -lGL -ldl -lpthread
endif

GUI_CXXFLAGS := $(CXXFLAGS) -Ithird_party/imgui -Ithird_party/imgui/backends -Ithird_party/stb $(GLFW_INCLUDE)

all: $(CONSOLE_TARGET)

$(CONSOLE_TARGET): $(CONSOLE_OBJECTS)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

gui:
	@test -f third_party/imgui/imgui.h || (echo "Loi: thieu third_party/imgui (Dear ImGui)."; exit 1)
	@test -f third_party/glfw/include/GLFW/glfw3.h || (echo "Loi: thieu GLFW tai third_party/glfw/include."; exit 1)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(GUI_CXXFLAGS) -o $(GUI_TARGET) $(BUSINESS_SOURCES) $(GUI_SOURCES) $(IMGUI_SOURCES) $(GLFW_LIB)

run: all
	./$(CONSOLE_TARGET)

run-gui: gui
	./$(GUI_TARGET)

clean:
	rm -f $(CONSOLE_OBJECTS) $(CONSOLE_TARGET) $(GUI_TARGET)

.PHONY: all gui run run-gui clean
