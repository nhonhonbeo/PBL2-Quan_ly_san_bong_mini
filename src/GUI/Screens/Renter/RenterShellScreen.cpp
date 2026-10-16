#include "GUI/Screens/Renter/RenterShellScreen.h"

#include <vector>
#include <imgui.h>

#include "Account/Customer.h"
#include "GUI/App/ScreenManager.h"
#include "GUI/App/Session.h"
#include "GUI/Screens/Auth/AuthScreen.h"
#include "GUI/Theme/Theme.h"
#include "GUI/Widgets/Modal.h"
#include "GUI/Widgets/Sidebar.h"
#include "GUI/Widgets/Toast.h"
#include "GUI/Widgets/TopBar.h"

namespace GUI::Screens::Renter {

RenterShellScreen::RenterShellScreen() {
    // Khi bấm "Xem lịch của tôi" trên phiếu đặt sân -> tự động chuyển sang tab Lịch đặt (tab 1)
    datSanScreen_ = std::make_unique<DatSanScreen>([this]() {
        selectedTab_ = 1;
        if (lichDatScreen_) {
            lichDatScreen_->reloadViewModel();
        }
    });

    lichDatScreen_ = std::make_unique<LichDatScreen>();
    taoGiaiScreen_ = std::make_unique<TaoGiaiScreen>();
    uuDaiScreen_ = std::make_unique<UuDaiScreen>();
    danhGiaScreen_ = std::make_unique<DanhGiaScreen>();
    profileScreen_ = std::make_unique<ProfileScreen>();
}

void RenterShellScreen::onEnter() {
    selectedTab_ = 0;
    if (datSanScreen_) datSanScreen_->onEnter();
    if (lichDatScreen_) lichDatScreen_->onEnter();
    if (taoGiaiScreen_) taoGiaiScreen_->onEnter();
    if (uuDaiScreen_) uuDaiScreen_->onEnter();
    if (danhGiaScreen_) danhGiaScreen_->onEnter();
    if (profileScreen_) profileScreen_->onEnter();
}

void RenterShellScreen::onExit() {
    if (datSanScreen_) datSanScreen_->onExit();
    if (lichDatScreen_) lichDatScreen_->onExit();
    if (taoGiaiScreen_) taoGiaiScreen_->onExit();
    if (uuDaiScreen_) uuDaiScreen_->onExit();
    if (danhGiaScreen_) danhGiaScreen_->onExit();
    if (profileScreen_) profileScreen_->onExit();
}

void RenterShellScreen::render() {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration |
                             ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoSavedSettings |
                             ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    if (ImGui::Begin("##RenterShellRoot", nullptr, flags)) {
        // 1. Sidebar menu trái
        renderSidebar();

        ImGui::SameLine(0.0f, 0.0f);

        // 2. Vùng nội dung bên phải
        ImGui::PushStyleColor(ImGuiCol_ChildBg, GUI::Theme::Colors::LightBg);
        if (ImGui::BeginChild("##RenterRightContent", ImVec2(0.0f, 0.0f), false)) {
            renderTopBar();

            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20.0f, 16.0f));
            if (ImGui::BeginChild("##RenterScrollContent", ImVec2(0.0f, 0.0f), false)) {
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
            u8"Bạn có chắc chắn muốn đăng xuất tài khoản Người thuê không?",
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

void RenterShellScreen::renderSidebar() {
    std::vector<Widgets::SidebarItem> menuItems = {
        {"0", "[⚽]", u8"Đặt sân trực tuyến"},
        {"1", "[📅]", u8"Lịch đặt của tôi"},
        {"2", "[🏆]", u8"Đăng ký tạo giải"},
        {"3", "[🎁]", u8"Ưu đãi thành viên"},
        {"4", "[⭐]", u8"Đánh giá sân bóng"},
        {"5", "[👤]", u8"Thông tin cá nhân"}
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

    // Kích hoạt nạp lại ViewModel khi đổi tab qua Sidebar
    if (selectedTab_ != prevTab) {
        if (selectedTab_ == 0 && datSanScreen_) datSanScreen_->reloadViewModel();
        else if (selectedTab_ == 1 && lichDatScreen_) lichDatScreen_->reloadViewModel();
        else if (selectedTab_ == 2 && taoGiaiScreen_) taoGiaiScreen_->reloadViewModel();
        else if (selectedTab_ == 3 && uuDaiScreen_) uuDaiScreen_->reloadViewModel();
        else if (selectedTab_ == 4 && danhGiaScreen_) danhGiaScreen_->reloadViewModel();
        else if (selectedTab_ == 5 && profileScreen_) profileScreen_->reloadViewModel();
    }
}

void RenterShellScreen::renderTopBar() {
    const char* tabTitles[] = {
        u8"Đặt sân bóng trực tuyến (Luồng từng bước)",
        u8"Lịch đặt của tôi & Quản lý hủy sân",
        u8"Đăng ký tổ chức giải đấu phong trào",
        u8"Ưu đãi tích lũy & Hạng thành viên",
        u8"Đánh giá chất lượng sân bóng",
        u8"Hồ sơ cá nhân & Đổi mật khẩu"
    };

    std::string currentTabName = tabTitles[selectedTab_];
    std::vector<std::string> breadcrumbs = {u8"Người thuê", currentTabName};

    User* user = GUI::App::Session::getInstance().getUser();
    std::string userName = user ? user->get_name() : u8"Người thuê";

    Widgets::TopBar(
        currentTabName.c_str(),
        breadcrumbs,
        userName.c_str(),
        u8"Người thuê",
        62.0f
    );
}

void RenterShellScreen::renderContentArea() {
    switch (selectedTab_) {
        case 0:
            if (datSanScreen_) {
                datSanScreen_->render();
            }
            break;
        case 1:
            if (lichDatScreen_) {
                lichDatScreen_->render();
            }
            break;
        case 2:
            if (taoGiaiScreen_) {
                taoGiaiScreen_->render();
            }
            break;
        case 3:
            if (uuDaiScreen_) {
                uuDaiScreen_->render();
            }
            break;
        case 4:
            if (danhGiaScreen_) {
                danhGiaScreen_->render();
            }
            break;
        case 5:
            if (profileScreen_) {
                profileScreen_->render();
            }
            break;
        default:
            break;
    }
}

}  // namespace GUI::Screens::Renter
