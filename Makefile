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
    EXE_EXT := .exe
else
    GLFW_INCLUDE :=
    GLFW_LIB := -lglfw -lGL -ldl -lpthread
    EXE_EXT :=
endif

GUI_CXXFLAGS := $(CXXFLAGS) -Ithird_party/imgui -Ithird_party/imgui/backends -Ithird_party/stb $(GLFW_INCLUDE)

all: gui

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

gui:
	@test -f third_party/imgui/imgui.h || (echo "Loi: thieu third_party/imgui (Dear ImGui)."; exit 1)
	@test -f third_party/glfw/include/GLFW/glfw3.h || (echo "Loi: thieu GLFW tai third_party/glfw/include."; exit 1)
	@mkdir -p $(BUILD_DIR)
	$(file >$(BUILD_DIR)/sources.rsp,$(BUSINESS_SOURCES) $(GUI_SOURCES) $(IMGUI_SOURCES))
	$(CXX) $(GUI_CXXFLAGS) -o $(GUI_TARGET) @$(BUILD_DIR)/sources.rsp $(GLFW_LIB)

run: gui
	./$(GUI_TARGET)

run-gui: gui
	./$(GUI_TARGET)

test-core:
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) tests/core/test_linked_list.cpp -o $(BUILD_DIR)/test_linked_list$(EXE_EXT)
	$(CXX) $(CXXFLAGS) tests/core/test_queue.cpp -o $(BUILD_DIR)/test_queue$(EXE_EXT)
	$(CXX) $(CXXFLAGS) tests/core/test_hash_table.cpp -o $(BUILD_DIR)/test_hash_table$(EXE_EXT)
	$(BUILD_DIR)/test_linked_list$(EXE_EXT)
	$(BUILD_DIR)/test_queue$(EXE_EXT)
	$(BUILD_DIR)/test_hash_table$(EXE_EXT)

test-booking:
	@mkdir -p $(BUILD_DIR)
	$(file >$(BUILD_DIR)/biz_sources.rsp,$(BUSINESS_SOURCES))
	$(CXX) $(CXXFLAGS) tests/test_booking_consistency.cpp @$(BUILD_DIR)/biz_sources.rsp -o $(BUILD_DIR)/test_booking$(EXE_EXT)
	$(BUILD_DIR)/test_booking$(EXE_EXT)

test-customer:
	@mkdir -p $(BUILD_DIR)
	$(file >$(BUILD_DIR)/biz_sources.rsp,$(BUSINESS_SOURCES))
	$(CXX) $(CXXFLAGS) tests/test_customer.cpp @$(BUILD_DIR)/biz_sources.rsp -o $(BUILD_DIR)/test_customer$(EXE_EXT)
	$(BUILD_DIR)/test_customer$(EXE_EXT)

test-file-manager:
	@mkdir -p $(BUILD_DIR)
	$(file >$(BUILD_DIR)/biz_sources.rsp,$(BUSINESS_SOURCES))
	$(CXX) $(CXXFLAGS) tests/test_file_manager.cpp @$(BUILD_DIR)/biz_sources.rsp -o $(BUILD_DIR)/test_file_manager$(EXE_EXT)
	$(BUILD_DIR)/test_file_manager$(EXE_EXT)

GUI_NO_MAIN := $(filter-out src/main_gui.cpp,$(GUI_SOURCES))

test-facade:
	@mkdir -p $(BUILD_DIR)
	$(file >$(BUILD_DIR)/facade_sources.rsp,$(BUSINESS_SOURCES) $(GUI_NO_MAIN) $(IMGUI_SOURCES))
	$(CXX) $(GUI_CXXFLAGS) tests/test_facade.cpp @$(BUILD_DIR)/facade_sources.rsp $(GLFW_LIB) -o $(BUILD_DIR)/test_facade$(EXE_EXT)
	$(BUILD_DIR)/test_facade$(EXE_EXT)

clean:
	rm -f $(GUI_TARGET) $(GUI_TARGET).exe

.PHONY: all gui run run-gui test-core test-booking test-customer test-file-manager test-facade clean
