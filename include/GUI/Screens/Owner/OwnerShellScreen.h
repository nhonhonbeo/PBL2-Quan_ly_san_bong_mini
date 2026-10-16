#pragma once

#include <memory>
#include "GUI/App/Screen.h"
#include "GUI/Screens/Owner/CreateFieldWizardScreen.h"
#include "GUI/Screens/Owner/DashboardScreen.h"
#include "GUI/Screens/Owner/FieldManagementScreen.h"

namespace GUI::Screens::Owner {

// LÝ DO CHỌN: OwnerShellScreen đóng vai trò khung vỏ (Shell) ứng dụng cho Chủ sân,
// tích hợp điều hướng mượt mà giữa Dashboard, Quản lý sân 4 tab và Wizard tạo sân.
class OwnerShellScreen : public GUI::App::Screen {
public:
    OwnerShellScreen();
    ~OwnerShellScreen() override = default;

    void onEnter() override;
    void render() override;
    void onExit() override;

private:
    void renderSidebar();
    void renderTopBar();
    void renderContentArea();

    void renderTabQuanLyKhachHang();
    void renderTabQuanLyGiaiDau();

    int selectedTab_ = 0;

    // Các màn hình con chuyên trách
    std::unique_ptr<DashboardScreen> dashboardScreen_;
    std::unique_ptr<FieldManagementScreen> fieldMgmtScreen_;
    std::unique_ptr<CreateFieldWizardScreen> wizardScreen_;
};

}  // namespace GUI::Screens::Owner
