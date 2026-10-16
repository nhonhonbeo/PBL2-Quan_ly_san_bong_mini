#pragma once

// LÝ DO CHỌN: Sử dụng mẫu Application bọc toàn bộ vòng đời (init, run, shutdown)
// theo nguyên lý RAII để quản lý tài nguyên GLFW/OpenGL/ImGui tập trung, tránh rò rỉ bộ nhớ.
class Application {
public:
    Application() = default;
    ~Application();

    // LÝ DO CHỌN: Xóa copy constructor và copy assignment operator để đảm bảo tính duy nhất
    // của context ứng dụng đồ họa (không sao chép tài nguyên cửa sổ/context đồ họa).
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    // Khởi tạo cửa sổ GLFW, nạp OpenGL3, cấu hình Dear ImGui và font chữ
    bool init();

    // Vòng lặp chính xử lý sự kiện và render khung hình
    void run();

    // Dọn dẹp tài nguyên và kết thúc ứng dụng an toàn
    void shutdown();

private:
    struct GLFWwindow* window_ = nullptr;
    bool initialized_ = false;
};
