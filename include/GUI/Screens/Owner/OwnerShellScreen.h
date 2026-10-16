#pragma once

#include <memory>
#include "GUI/App/Screen.h"
#include "GUI/Screens/Owner/CreateFieldWizardScreen.h"
#include "GUI/Screens/Owner/DashboardScreen.h"
#include "GUI/Screens/Owner/FieldManagementScreen.h"
#include "GUI/Screens/Owner/GiaiScreen.h"
#include "GUI/Screens/Owner/KhachHangScreen.h"

namespace GUI::Screens::Owner {

// LÝ DO CHỌN: OwnerShellScreen đóng vai trò khung vỏ (Shell) ứng dụng cho Chủ sân,
// tích hợp điều hướng mượt mà giữa Dashboard, Quản lý sân, Wizard tạo sân, Quản lý khách hàng và Quản lý giải đấu.
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

    int selectedTab_ = 0;

    // Các màn hình con chuyên trách
    std::unique_ptr<DashboardScreen> dashboardScreen_;
    std::unique_ptr<FieldManagementScreen> fieldMgmtScreen_;
    std::unique_ptr<CreateFieldWizardScreen> wizardScreen_;
    std::unique_ptr<KhachHangScreen> khachHangScreen_;
    std::unique_ptr<GiaiScreen> giaiScreen_;
};

}  // namespace GUI::Screens::Owner
