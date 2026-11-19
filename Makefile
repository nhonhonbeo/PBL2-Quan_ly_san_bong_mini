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

test-customer-index:
	@mkdir -p $(BUILD_DIR)
	$(file >$(BUILD_DIR)/biz_sources.rsp,$(BUSINESS_SOURCES))
	$(CXX) $(CXXFLAGS) tests/test_customer_repository_index.cpp @$(BUILD_DIR)/biz_sources.rsp -o $(BUILD_DIR)/test_customer_index$(EXE_EXT)
	$(BUILD_DIR)/test_customer_index$(EXE_EXT)

test-file-manager:
	@mkdir -p $(BUILD_DIR)
	$(file >$(BUILD_DIR)/biz_sources.rsp,$(BUSINESS_SOURCES))
	$(CXX) $(CXXFLAGS) tests/test_file_manager.cpp @$(BUILD_DIR)/biz_sources.rsp -o $(BUILD_DIR)/test_file_manager$(EXE_EXT)
	$(BUILD_DIR)/test_file_manager$(EXE_EXT)

test-payment:
	@mkdir -p $(BUILD_DIR)
	$(file >$(BUILD_DIR)/biz_sources.rsp,$(BUSINESS_SOURCES))
	$(CXX) $(CXXFLAGS) tests/test_payment_consistency.cpp @$(BUILD_DIR)/biz_sources.rsp -o $(BUILD_DIR)/test_payment_consistency$(EXE_EXT)
	$(BUILD_DIR)/test_payment_consistency$(EXE_EXT)

test-venue-images:
	@mkdir -p $(BUILD_DIR)
	$(file >$(BUILD_DIR)/biz_sources.rsp,$(BUSINESS_SOURCES))
	$(CXX) $(CXXFLAGS) tests/test_venue_images.cpp @$(BUILD_DIR)/biz_sources.rsp -o $(BUILD_DIR)/test_venue_images$(EXE_EXT)
	$(BUILD_DIR)/test_venue_images$(EXE_EXT)

GUI_NO_MAIN := $(filter-out src/main_gui.cpp,$(GUI_SOURCES))

test-facade:
	@mkdir -p $(BUILD_DIR)
	$(file >$(BUILD_DIR)/facade_sources.rsp,$(BUSINESS_SOURCES) $(GUI_NO_MAIN) $(IMGUI_SOURCES))
	$(CXX) $(GUI_CXXFLAGS) tests/test_facade.cpp @$(BUILD_DIR)/facade_sources.rsp $(GLFW_LIB) -o $(BUILD_DIR)/test_facade$(EXE_EXT)
	$(BUILD_DIR)/test_facade$(EXE_EXT)

test-domain:
	@mkdir -p $(BUILD_DIR)
	$(file >$(BUILD_DIR)/biz_sources.rsp,$(BUSINESS_SOURCES))
	$(CXX) $(CXXFLAGS) tests/test_domain_and_modules.cpp @$(BUILD_DIR)/biz_sources.rsp -o $(BUILD_DIR)/test_domain_and_modules$(EXE_EXT)
	$(BUILD_DIR)/test_domain_and_modules$(EXE_EXT)

test-service-responsibilities:
	@mkdir -p $(BUILD_DIR)
	$(file >$(BUILD_DIR)/biz_sources.rsp,$(BUSINESS_SOURCES))
	$(CXX) $(CXXFLAGS) tests/test_service_responsibilities.cpp @$(BUILD_DIR)/biz_sources.rsp -o $(BUILD_DIR)/test_service_responsibilities$(EXE_EXT)
	$(BUILD_DIR)/test_service_responsibilities$(EXE_EXT)

test-single-source:
	@mkdir -p $(BUILD_DIR)
	$(file >$(BUILD_DIR)/biz_sources.rsp,$(BUSINESS_SOURCES))
	$(CXX) $(CXXFLAGS) tests/test_single_source.cpp @$(BUILD_DIR)/biz_sources.rsp -o $(BUILD_DIR)/test_single_source$(EXE_EXT)
	$(BUILD_DIR)/test_single_source$(EXE_EXT)

test-task07-validation:
	@mkdir -p $(BUILD_DIR)
	$(file >$(BUILD_DIR)/biz_sources.rsp,$(BUSINESS_SOURCES))
	$(CXX) $(CXXFLAGS) tests/test_task07_validation.cpp @$(BUILD_DIR)/biz_sources.rsp -o $(BUILD_DIR)/test_task07_validation$(EXE_EXT)
	$(BUILD_DIR)/test_task07_validation$(EXE_EXT)

test-task08-di:
	@mkdir -p $(BUILD_DIR)
	$(file >$(BUILD_DIR)/biz_sources.rsp,$(BUSINESS_SOURCES))
	$(CXX) $(CXXFLAGS) tests/test_task08_di.cpp @$(BUILD_DIR)/biz_sources.rsp -o $(BUILD_DIR)/test_task08_di$(EXE_EXT)
	$(BUILD_DIR)/test_task08_di$(EXE_EXT)

test-tournament:
	@mkdir -p $(BUILD_DIR)
	$(file >$(BUILD_DIR)/biz_sources.rsp,$(BUSINESS_SOURCES))
	$(CXX) $(CXXFLAGS) tests/test_tournament.cpp @$(BUILD_DIR)/biz_sources.rsp -o $(BUILD_DIR)/test_tournament$(EXE_EXT)
	$(BUILD_DIR)/test_tournament$(EXE_EXT)

test:
	$(MAKE) test-core
	$(MAKE) test-booking
	$(MAKE) test-customer
	$(MAKE) test-customer-index
	$(MAKE) test-domain
	$(MAKE) test-facade
	$(MAKE) test-file-manager
	$(MAKE) test-payment
	$(MAKE) test-service-responsibilities
	$(MAKE) test-single-source
	$(MAKE) test-task07-validation
	$(MAKE) test-task08-di
	$(MAKE) test-tournament
	$(MAKE) test-venue-images

clean:
	rm -f $(BUILD_DIR)/*.o $(BUILD_DIR)/*.obj $(BUILD_DIR)/*.exe $(BUILD_DIR)/*.rsp

.PHONY: all gui run run-gui test test-core test-booking test-customer test-customer-index test-file-manager test-payment test-venue-images test-facade test-domain test-service-responsibilities test-single-source test-task07-validation test-task08-di test-tournament clean
