#include "GUI/App/ScreenManager.h"

namespace GUI::App {

ScreenManager::ScreenManager() = default;

ScreenManager::~ScreenManager() {
    while (!screenStack_.empty()) {
        if (screenStack_.back()) {
            screenStack_.back()->onExit();
        }
        screenStack_.pop_back();
    }
}

ScreenManager& ScreenManager::getInstance() {
    static ScreenManager instance;
    return instance;
}

void ScreenManager::replace(std::unique_ptr<Screen> newScreen) {
    PendingAction action;
    action.type = ActionType::Replace;
    action.screen = std::move(newScreen);
    pendingActions_.push_back(std::move(action));
}

void ScreenManager::setScreen(std::unique_ptr<Screen> newScreen) {
    replace(std::move(newScreen));
}

void ScreenManager::push(std::unique_ptr<Screen> newScreen) {
    PendingAction action;
    action.type = ActionType::Push;
    action.screen = std::move(newScreen);
    pendingActions_.push_back(std::move(action));
}

void ScreenManager::pushScreen(std::unique_ptr<Screen> newScreen) {
    push(std::move(newScreen));
}

void ScreenManager::pop() {
    PendingAction action;
    action.type = ActionType::Pop;
    action.screen = nullptr;
    pendingActions_.push_back(std::move(action));
}

void ScreenManager::popScreen() {
    pop();
}

void ScreenManager::processPendingTransitions() {
    if (isProcessing_ || pendingActions_.empty()) {
        return;
    }

    isProcessing_ = true;

    // Lấy danh sách lệnh cần xử lý và xóa hàng đợi gốc
    std::vector<PendingAction> actionsToProcess = std::move(pendingActions_);
    pendingActions_.clear();

    for (auto& action : actionsToProcess) {
        switch (action.type) {
            case ActionType::Replace:
                // Dọn dẹp toàn bộ màn hình cũ
                while (!screenStack_.empty()) {
                    if (screenStack_.back()) {
                        screenStack_.back()->onExit();
                    }
                    screenStack_.pop_back();
                }
                // Nạp màn hình mới
                if (action.screen) {
                    action.screen->onEnter();
                    screenStack_.push_back(std::move(action.screen));
                }
                break;

            case ActionType::Push:
                if (action.screen) {
                    action.screen->onEnter();
                    screenStack_.push_back(std::move(action.screen));
                }
                break;

            case ActionType::Pop:
                if (!screenStack_.empty()) {
                    if (screenStack_.back()) {
                        screenStack_.back()->onExit();
                    }
                    screenStack_.pop_back();
                }
                break;

            case ActionType::None:
            default:
                break;
        }
    }

    isProcessing_ = false;
}

void ScreenManager::renderCurrentScreen() {
    if (!screenStack_.empty() && screenStack_.back()) {
        screenStack_.back()->render();
    }
    // LÝ DO CHỌN: Luôn xử lý các chuyển cảnh an toàn ở cuối khung hình sau khi hàm render() hoàn tất
    processPendingTransitions();
}

bool ScreenManager::isEmpty() const {
    return screenStack_.empty();
}

Screen* ScreenManager::getCurrentScreen() const {
    if (screenStack_.empty()) {
        return nullptr;
    }
    return screenStack_.back().get();
}

}  // namespace GUI::App
