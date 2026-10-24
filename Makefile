CXX ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Iinclude -Iinclude/features

BUILD_DIR := build
GUI_TARGET := $(BUILD_DIR)/QuanLySanBong_GUI

# Mọi mã nghiệp vụ, trừ hai entry point, được liên kết vào ứng dụng GUI.
BUSINESS_SRC_DIRS := src/Core src/features/Account src/features/Field src/features/Booking src/features/Customer src/Repository src/Service src/features/Payment src/features/Tournament src/features/Storage
BUSINESS_SOURCES := $(foreach dir,$(BUSINESS_SRC_DIRS),$(wildcard $(dir)/*.cpp))
GUI_SRC_DIRS := src/GUI src/GUI/App src/GUI/Theme src/GUI/Widgets src/GUI/Screens src/GUI/Screens/Auth src/GUI/Screens/Owner src/GUI/Screens/Renter
GUI_SOURCES := src/main_gui.cpp $(foreach dir,$(GUI_SRC_DIRS),$(wildcard $(dir)/*.cpp))
IMGUI_SOURCES := $(wildcard third_party/imgui/*.cpp) \
                third_party/imgui/backends/imgui_impl_glfw.cpp \
                third_party/imgui/backends/imgui_impl_opengl3.cpp

ifeq ($(OS),Windows_NT)
    GUI_TARGET := $(GUI_TARGET).exe
    GLFW_INCLUDE := -Ithird_party/glfw/include
    GLFW_LIB := -Lthird_party/glfw/lib -lglfw3 -lopengl32 -lgdi32 -static-libgcc -static-libstdc++
else
    GLFW_INCLUDE :=
    GLFW_LIB := -lglfw -lGL -ldl -lpthread
endif

GUI_CXXFLAGS := $(CXXFLAGS) -Ithird_party/imgui -Ithird_party/imgui/backends -Ithird_party/stb $(GLFW_INCLUDE)

all: gui

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

gui:
	@test -f third_party/imgui/imgui.h || (echo "Loi: thieu third_party/imgui (Dear ImGui)."; exit 1)
	@test -f third_party/glfw/include/GLFW/glfw3.h || (echo "Loi: thieu GLFW tai third_party/glfw/include."; exit 1)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(GUI_CXXFLAGS) -o $(GUI_TARGET) $(BUSINESS_SOURCES) $(GUI_SOURCES) $(IMGUI_SOURCES) $(GLFW_LIB)

run: gui
	./$(GUI_TARGET)

run-gui: gui
	./$(GUI_TARGET)

clean:
	rm -f $(GUI_TARGET) $(GUI_TARGET).exe

.PHONY: all gui run run-gui clean
