#include "GUI/App/Application.h"

#include <filesystem>
#include <iostream>
#include <memory>

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include "GUI/App/Facade.h"
#include "GUI/App/ScreenManager.h"
#include "GUI/Screens/Auth/AuthScreen.h"
#include "GUI/Screens/DemoScreen.h"
#include "GUI/Theme/Fonts.h"
#include "GUI/Theme/Theme.h"
#include "GUI/Widgets/Toast.h"

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

// Callback khi người dùng nhấn nút đóng cửa sổ: đảm bảo lưu trữ toàn bộ dữ liệu an toàn
void windowCloseCallback(GLFWwindow* /*window*/) {
    GUI::App::Facade::getInstance().luuDuLieu();
}

}  // namespace

Application::Application(bool runDemo)
    : runDemo_(runDemo) {}

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

    window_ = glfwCreateWindow(1320, 800, u8"Quản lý sân bóng mini", nullptr, nullptr);
    if (window_ == nullptr) {
        std::cerr << "Không thể tạo cửa sổ GLFW.\n";
        glfwTerminate();
        return false;
    }

    setWindowIcon(window_);
    glfwMakeContextCurrent(window_);
    glfwSwapInterval(1); // VSync

    // Đăng ký callback lưu dữ liệu khi cửa sổ đóng
    glfwSetWindowCloseCallback(window_, windowCloseCallback);

    // Khởi tạo Dear ImGui context và thiết lập backend GLFW + OpenGL3
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // Áp dụng bảng màu và kiểu dáng sân bóng mini toàn cục
    GUI::Theme::apply();

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

    // Nạp bộ font chữ UTF-8 tiếng Việt
    GUI::Theme::loadFonts();

    // LÝ DO CHỌN: Nạp dữ liệu từ FileManager qua Facade khi ứng dụng khởi động
    GUI::App::Facade::getInstance().napDuLieu();

    // Thiết lập màn hình khởi động vào ScreenManager
    if (runDemo_) {
        GUI::App::ScreenManager::getInstance().replace(std::make_unique<GUI::Screens::DemoScreen>());
    } else {
        // Mặc định nạp màn hình Xác thực & Chào mừng (AuthScreen)
        GUI::App::ScreenManager::getInstance().replace(std::make_unique<GUI::Screens::Auth::AuthScreen>());
    }

    initialized_ = true;
    return true;
}

void Application::run() {
    if (!initialized_) {
        return;
    }

    while (glfwWindowShouldClose(window_) == GLFW_FALSE) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Render màn hình hiện tại thông qua ScreenManager toàn cục
        GUI::App::ScreenManager::getInstance().renderCurrentScreen();

        // Render hàng đợi thông báo Toast trên cùng
        Widgets::RenderToasts();

        ImGui::Render();
        int displayWidth = 0;
        int displayHeight = 0;
        glfwGetFramebufferSize(window_, &displayWidth, &displayHeight);
        glViewport(0, 0, displayWidth, displayHeight);

        glClearColor(0.957f, 0.969f, 0.961f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window_);
    }
}

void Application::shutdown() {
    if (!initialized_) {
        return;
    }

    // LÝ DO CHỌN: Lưu lại toàn bộ dữ liệu tài khoản và nghiệp vụ trước khi thoát ứng dụng
    GUI::App::Facade::getInstance().luuDuLieu();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window_);
    window_ = nullptr;
    glfwTerminate();
    initialized_ = false;
}
