#pragma once

class Application {
public:
    Application() = default;
    ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    bool init();
    void run();
    void shutdown();

private:
    struct GLFWwindow* window_ = nullptr;
    bool initialized_ = false;
};
