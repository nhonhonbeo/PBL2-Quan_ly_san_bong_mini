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

    // Khi bấm nút tạo sân wizard trong FieldManagementScreen -> chuyển tab sang Wizard
    fieldMgmtScreen_ = std::make_unique<FieldManagementScreen>([this]() {
        selectedTab_ = 2;
        if (wizardScreen_) {
            wizardScreen_->onEnter();
        }
    });

    // Khi Wizard tạo sân hoàn tất -> tự động chuyển về Quản lý sân
    wizardScreen_ = std::make_unique<CreateFieldWizardScreen>([this]() {
        selectedTab_ = 1;
        if (fieldMgmtScreen_) {
            fieldMgmtScreen_->reloadViewModel();
        }
    });
}

void OwnerShellScreen::onEnter() {
    selectedTab_ = 0;
    if (dashboardScreen_) dashboardScreen_->onEnter();
    if (fieldMgmtScreen_) fieldMgmtScreen_->onEnter();
    if (wizardScreen_) wizardScreen_->onEnter();
}

void OwnerShellScreen::onExit() {
    if (dashboardScreen_) dashboardScreen_->onExit();
    if (fieldMgmtScreen_) fieldMgmtScreen_->onExit();
    if (wizardScreen_) wizardScreen_->onExit();
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
        // 1. Sidebar menu trái
        renderSidebar();

        ImGui::SameLine(0.0f, 0.0f);

        // 2. Vùng nội dung bên phải
        ImGui::PushStyleColor(ImGuiCol_ChildBg, GUI::Theme::Colors::LightBg);
        if (ImGui::BeginChild("##OwnerRightContent", ImVec2(0.0f, 0.0f), false)) {
            renderTopBar();

            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20.0f, 16.0f));
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
            u8"Bạn có chắc chắn muốn đăng xuất tài khoản Chủ sân không?",
            []() {
                GUI::App::Session::getInstance().logout();
                Widgets::ToastThongTin(u8"Đã đăng xuất tài khoản thành công!");
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
        {"0", "[📊]", u8"Tổng quan cơ sở"},
        {"1", "[⚽]", u8"Quản lý sân bóng"},
        {"2", "[✨]", u8"Khởi tạo sân mới"},
        {"3", "[👥]", u8"Quản lý khách hàng"},
        {"4", "[🏆]", u8"Quản lý giải đấu"}
    };

    int prevTab = selectedTab_;
    Widgets::Sidebar(
        menuItems,
        selectedTab_,
        240.0f,
        []() {
            Widgets::MoModalXacNhan(u8"Xác nhận đăng xuất");
        }
    );

    // Kích hoạt onEnter tương ứng khi đổi tab qua Sidebar
    if (selectedTab_ != prevTab) {
        if (selectedTab_ == 0 && dashboardScreen_) dashboardScreen_->onEnter();
        else if (selectedTab_ == 1 && fieldMgmtScreen_) fieldMgmtScreen_->reloadViewModel();
        else if (selectedTab_ == 2 && wizardScreen_) wizardScreen_->onEnter();
    }
}

void OwnerShellScreen::renderTopBar() {
    const char* tabTitles[] = {
        u8"Tổng quan & Báo cáo cơ sở sân",
        u8"Quản lý sân bóng mini (4 tab)",
        u8"Khởi tạo sân bóng mới (Wizard 4 bước)",
        u8"Quản lý khách hàng & Ưu đãi",
        u8"Quản lý sự kiện & Giải đấu"
    };

    std::string currentTabName = tabTitles[selectedTab_];
    std::vector<std::string> breadcrumbs = {u8"Chủ sân", currentTabName};

    User* user = GUI::App::Session::getInstance().getUser();
    std::string userName = user ? user->get_name() : u8"Nguyễn Văn Chủ";

    Widgets::TopBar(
        currentTabName.c_str(),
        breadcrumbs,
        userName.c_str(),
        u8"Chủ sân",
        62.0f
    );
}

void OwnerShellScreen::renderContentArea() {
    switch (selectedTab_) {
        case 0:
            if (dashboardScreen_) {
                dashboardScreen_->render();
            }
            break;
        case 1:
            if (fieldMgmtScreen_) {
                fieldMgmtScreen_->render();
            }
            break;
        case 2:
            if (wizardScreen_) {
                wizardScreen_->render();
            }
            break;
        case 3:
            renderTabQuanLyKhachHang();
            break;
        case 4:
            renderTabQuanLyGiaiDau();
            break;
        default:
            break;
    }
}

void OwnerShellScreen::renderTabQuanLyKhachHang() {
    Widgets::Card("card_owner_cust_mgmt", u8"Quản lý khách hàng, Ưu đãi & Danh sách đen (Blacklist)", []() {
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
        ImGui::TextWrapped(u8"Theo dõi danh bạ khách hàng theo số điện thoại: Tự động tính số lần đặt sân để áp dụng ưu đãi giảm giá (VIP), hoặc đưa vào danh sách đen nếu hủy đặt nhiều lần quá quy định.");
        ImGui::PopStyleColor();
        ImGui::Spacing();

        Widgets::BadgeVIP(u8"Khách hàng thân thiết (VIP)");
        ImGui::SameLine();
        Widgets::BadgeDanhSachDen(u8"Danh sách đen (Hủy nhiều)");
        ImGui::Spacing();
        ImGui::Spacing();

        if (Widgets::NutPhu(u8"Xem đánh giá & phản hồi của khách")) {
            Widgets::ToastThongTin(u8"Hiển thị danh sách nhận xét sao đánh giá");
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
        ImGui::TextWrapped(u8"// TODO: Sinh viên triển khai liên kết với CustomerManager, Blacklist và Discount trong include/Customer.");
        ImGui::PopStyleColor();
    });
}

void OwnerShellScreen::renderTabQuanLyGiaiDau() {
    Widgets::Card("card_owner_tournament", u8"Quản lý giải đấu & Sự kiện sân bóng", []() {
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
        ImGui::TextWrapped(u8"Tổ chức và phê duyệt các giải đấu bóng đá phong trào: Tên giải đấu, khách đặt sự kiện thi đấu, lịch thi đấu và điều phối trọng tài/sân bãi.");
        ImGui::PopStyleColor();
        ImGui::Spacing();

        Widgets::Badge(u8"Mô đun Giải đấu", GUI::Theme::Colors::GoldAccent, GUI::Theme::Colors::WhiteCard);
        ImGui::Spacing();
        ImGui::Spacing();

        if (Widgets::NutChinh(u8"+ Tạo giải đấu mới")) {
            Widgets::ToastThongTin(u8"Tạo giải đấu: gọi Facade::themGiaiDau");
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
        ImGui::TextWrapped(u8"// TODO: Sinh viên triển khai liên kết với TournamentManager trong include/Tournament.");
        ImGui::PopStyleColor();
    });
}

}  // namespace GUI::Screens::Owner
