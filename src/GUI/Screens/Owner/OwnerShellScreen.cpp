#include "GUI/Screens/Owner/OwnerShellScreen.h"

#include <vector>
#include <imgui.h>

#include "GUI/App/ScreenManager.h"
#include "GUI/App/Session.h"
#include "GUI/Screens/Auth/AuthScreen.h"
#include "GUI/Theme/Theme.h"
#include "GUI/Widgets/Badge.h"
#include "GUI/Widgets/Button.h"
#include "GUI/Widgets/Card.h"
#include "GUI/Widgets/Modal.h"
#include "GUI/Widgets/Sidebar.h"
#include "GUI/Widgets/Toast.h"
#include "GUI/Widgets/TopBar.h"

namespace GUI::Screens::Owner {

OwnerShellScreen::OwnerShellScreen() {
    dashboardScreen_ = std::make_unique<DashboardScreen>();

    fieldMgmtScreen_ = std::make_unique<FieldManagementScreen>([this]() {
        selectedTab_ = 2;
        if (wizardScreen_) {
            wizardScreen_->onEnter();
        }
    });

    wizardScreen_ = std::make_unique<CreateFieldWizardScreen>([this]() {
        selectedTab_ = 1;
        if (fieldMgmtScreen_) {
            fieldMgmtScreen_->reloadViewModel();
        }
    });

    khachHangScreen_ = std::make_unique<KhachHangScreen>();
    giaiScreen_ = std::make_unique<GiaiScreen>();
}

void OwnerShellScreen::onEnter() {
    selectedTab_ = 0;
    if (dashboardScreen_) dashboardScreen_->onEnter();
    if (fieldMgmtScreen_) fieldMgmtScreen_->onEnter();
    if (wizardScreen_) wizardScreen_->onEnter();
    if (khachHangScreen_) khachHangScreen_->onEnter();
    if (giaiScreen_) giaiScreen_->onEnter();
}

void OwnerShellScreen::onExit() {
    if (dashboardScreen_) dashboardScreen_->onExit();
    if (fieldMgmtScreen_) fieldMgmtScreen_->onExit();
    if (wizardScreen_) wizardScreen_->onExit();
    if (khachHangScreen_) khachHangScreen_->onExit();
    if (giaiScreen_) giaiScreen_->onExit();
}

void OwnerShellScreen::render() {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration |
                             ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoSavedSettings |
                             ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    if (ImGui::Begin("##OwnerShellRootWindow", nullptr, flags)) {
        if (yeuCauDangXuat_) {
            Widgets::MoModalXacNhan(u8"Xác nhận đăng xuất");
            yeuCauDangXuat_ = false;
        }

        // 1. Sidebar menu trái
        renderSidebar();

        ImGui::SameLine(0.0f, 0.0f);

        // 2. Vùng nội dung bên phải
        ImGui::PushStyleColor(ImGuiCol_ChildBg, GUI::Theme::Colors::Background);
        if (ImGui::BeginChild("##OwnerRightContent", ImVec2(0.0f, 0.0f), false)) {
            renderTopBar();

            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(22.0f, 18.0f));
            if (ImGui::BeginChild("##OwnerScrollContent", ImVec2(0.0f, 0.0f), false)) {
                renderContentArea();
            }
            ImGui::EndChild();
            ImGui::PopStyleVar();
        }
        ImGui::EndChild();
        ImGui::PopStyleColor();

        // Modal xác nhận đăng xuất
        Widgets::ModalXacNhan(
            u8"Xác nhận đăng xuất",
            u8"Xác nhận đăng xuất khỏi phiên làm việc của Chủ sân.\nMọi thay đổi chưa lưu sẽ được hệ thống tự động đồng bộ an toàn.",
            []() {
                GUI::App::Session::getInstance().logout();
                Widgets::ToastThongTin(u8"Đã đăng xuất tài khoản Chủ sân an toàn!");
                GUI::App::ScreenManager::getInstance().replace(
                    std::make_unique<GUI::Screens::Auth::AuthScreen>()
                );
            },
            u8"Đăng xuất",
            u8"Hủy bỏ",
            false
        );
    }
    ImGui::End();

    ImGui::PopStyleVar(2);
}

void OwnerShellScreen::renderSidebar() {
    std::vector<Widgets::SidebarItem> menuItems = {
        {"0", "", u8"Tổng quan cơ sở"},
        {"1", "", u8"Quản lý sân bóng"},
        {"2", "", u8"Khởi tạo sân mới"},
        {"3", "", u8"Quản lý khách hàng"},
        {"4", "", u8"Quản lý giải đấu"}
    };

    int prevTab = selectedTab_;
    Widgets::Sidebar(
        menuItems,
        selectedTab_,
        240.0f,
        [this]() {
            yeuCauDangXuat_ = true;
        }
    );

    if (selectedTab_ != prevTab) {
        if (selectedTab_ == 0 && dashboardScreen_) dashboardScreen_->onEnter();
        else if (selectedTab_ == 1 && fieldMgmtScreen_) fieldMgmtScreen_->reloadViewModel();
        else if (selectedTab_ == 2 && wizardScreen_) wizardScreen_->onEnter();
        else if (selectedTab_ == 3 && khachHangScreen_) khachHangScreen_->reloadViewModel();
        else if (selectedTab_ == 4 && giaiScreen_) giaiScreen_->reloadViewModel();
    }
}

void OwnerShellScreen::renderTopBar() {
    const char* tabTitles[] = {
        u8"Tổng quan & Báo cáo cơ sở sân",
        u8"Quản lý danh sách sân & Lịch đặt",
        u8"Khởi tạo sân bóng mới (Wizard 4 bước)",
        u8"Quản lý danh bạ khách hàng & Ưu đãi VIP",
        u8"Quản lý sự kiện & Giải đấu thể thao"
    };

    std::string currentTabName = tabTitles[selectedTab_];
    std::vector<std::string> breadcrumbs = {u8"Chủ sân", currentTabName};

    User* user = GUI::App::Session::getInstance().getUser();
    std::string userName = user ? user->get_name() : u8"Chủ sân bóng";

    Widgets::TopBar(
        currentTabName.c_str(),
        breadcrumbs,
        userName.c_str(),
        u8"Chủ sân",
        60.0f,
        [this]() { yeuCauDangXuat_ = true; }
    );
}

void OwnerShellScreen::renderContentArea() {
    switch (selectedTab_) {
        case 0:
            if (dashboardScreen_) dashboardScreen_->render();
            break;
        case 1:
            if (fieldMgmtScreen_) fieldMgmtScreen_->render();
            break;
        case 2:
            if (wizardScreen_) wizardScreen_->render();
            break;
        case 3:
            if (khachHangScreen_) khachHangScreen_->render();
            break;
        case 4:
            if (giaiScreen_) giaiScreen_->render();
            break;
        default:
            break;
    }
}

}  // namespace GUI::Screens::Owner
