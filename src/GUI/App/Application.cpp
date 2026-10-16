#include "GUI/App/Application.h"

#include <filesystem>
#include <iostream>

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include "GUI/Theme/Fonts.h"

#if __has_include(<stb_image.h>)
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define HAS_STB_IMAGE 1
#else
#define HAS_STB_IMAGE 0
#endif

namespace {

void glfwErrorCallback(int error, const char* description) {
    std::cerr << "Lỗi GLFW " << error << ": " << description << '\n';
}

void setWindowIcon(GLFWwindow* window) {
#if HAS_STB_IMAGE
    if (!std::filesystem::exists("assets/icon.png")) {
        return;
    }

    GLFWimage icon{};
    int channels = 0;
    icon.pixels = stbi_load("assets/icon.png", &icon.width, &icon.height, &channels, 4);
    if (icon.pixels == nullptr) {
        std::cerr << "Không thể đọc assets/icon.png; bỏ qua icon cửa sổ.\n";
        return;
    }
    glfwSetWindowIcon(window, 1, &icon);
    stbi_image_free(icon.pixels);
#else
    (void)window;
#endif
}

}  // namespace

Application::~Application() {
    shutdown();
}

bool Application::init() {
    glfwSetErrorCallback(glfwErrorCallback);
    if (glfwInit() == GLFW_FALSE) {
        std::cerr << "Không thể khởi tạo GLFW.\n";
        return false;
    }

#if defined(__APPLE__)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    window_ = glfwCreateWindow(1280, 760, u8"Quản lý sân bóng mini", nullptr, nullptr);
    if (window_ == nullptr) {
        std::cerr << "Không thể tạo cửa sổ GLFW.\n";
        glfwTerminate();
        return false;
    }

    setWindowIcon(window_);

    glfwMakeContextCurrent(window_);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    if (!ImGui_ImplGlfw_InitForOpenGL(window_, true) ||
        !ImGui_ImplOpenGL3_Init("#version 130")) {
        std::cerr << "Không thể khởi tạo backend ImGui.\n";
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        glfwDestroyWindow(window_);
        window_ = nullptr;
        glfwTerminate();
        return false;
    }
    initialized_ = true;
    return true;
}

void Application::run() {
    if (!initialized_) {
        return;
    }

    const GUI::Theme::Fonts fonts = GUI::Theme::loadFonts();
    while (glfwWindowShouldClose(window_) == GLFW_FALSE) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(32.0F, 32.0F), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(700.0F, 260.0F), ImGuiCond_FirstUseEver);
        ImGui::Begin(u8"Kiểm tra font tiếng Việt");
        ImGui::PushFont(fonts.heading);
        ImGui::TextUnformatted(u8"Quản lý sân bóng mini");
        ImGui::PopFont();
        ImGui::PushFont(fonts.regular);
        ImGui::TextUnformatted(u8"Ắ Ằ Ẳ Ẵ Ặ Ế Ề Ể Ễ Ệ Ố Ồ Ổ Ỗ Ộ Ứ Ừ Ử Ữ Ự");
        ImGui::PopFont();
        ImGui::PushFont(fonts.small);
        ImGui::TextUnformatted(u8"Dòng này dùng để kiểm tra bộ glyph tiếng Việt.");
        ImGui::PopFont();
        ImGui::End();

        ImGui::Render();
        int displayWidth = 0;
        int displayHeight = 0;
        glfwGetFramebufferSize(window_, &displayWidth, &displayHeight);
        glViewport(0, 0, displayWidth, displayHeight);
        glClearColor(0.08F, 0.09F, 0.12F, 1.0F);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window_);
    }
}

void Application::shutdown() {
    if (!initialized_) {
        return;
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window_);
    window_ = nullptr;
    glfwTerminate();
    initialized_ = false;
}
