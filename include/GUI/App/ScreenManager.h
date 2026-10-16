#pragma once

#include <memory>
#include <vector>
#include "GUI/App/Screen.h"

namespace GUI::App {

// LÝ DO CHỌN: ScreenManager quản lý ngăn xếp (stack) màn hình với cơ chế chuyển cảnh an toàn
// (hàng đợi lệnh chuyển cảnh), không xóa màn hình đang render giữa frame để tránh lỗi vùng nhớ (use-after-free).
class ScreenManager {
public:
    enum class ActionType {
        None,
        Push,
        Pop,
        Replace
    };

    struct PendingAction {
        ActionType type = ActionType::None;
        std::unique_ptr<Screen> screen;
    };

    ScreenManager();
    ~ScreenManager();

    // Lấy instance toàn cục dùng chung cho toàn bộ ứng dụng
    static ScreenManager& getInstance();

    // Hàng đợi lệnh: Thay thế toàn bộ stack bằng một màn hình mới
    void replace(std::unique_ptr<Screen> newScreen);
    void setScreen(std::unique_ptr<Screen> newScreen); // Alias của replace

    // Hàng đợi lệnh: Đẩy màn hình mới lên đỉnh ngăn xếp
    void push(std::unique_ptr<Screen> newScreen);
    void pushScreen(std::unique_ptr<Screen> newScreen); // Alias của push

    // Hàng đợi lệnh: Quay lại màn hình trước đó
    void pop();
    void popScreen(); // Alias của pop

    // Render màn hình hiện tại và xử lý chuyển cảnh an toàn sau frame
    void renderCurrentScreen();

    // Xử lý các lệnh chuyển cảnh đang chờ trong hàng đợi
    void processPendingTransitions();

    // Kiểm tra xem stack có trống hay không
    bool isEmpty() const;

    // Lấy con trỏ màn hình đang ở đỉnh stack
    Screen* getCurrentScreen() const;

private:
    std::vector<std::unique_ptr<Screen>> screenStack_;
    std::vector<PendingAction> pendingActions_;
    bool isProcessing_ = false;
};

}  // namespace GUI::App
