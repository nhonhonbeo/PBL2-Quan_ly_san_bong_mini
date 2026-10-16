#include "GUI/Screens/Renter/RenterShellScreen.h"

#include <vector>
#include <imgui.h>

#include "Account/Customer.h"
#include "GUI/App/ScreenManager.h"
#include "GUI/App/Session.h"
#include "GUI/Screens/Auth/AuthScreen.h"
#include "GUI/Theme/Theme.h"
#include "GUI/Widgets/Badge.h"
#include "GUI/Widgets/Button.h"
#include "GUI/Widgets/Card.h"
#include "GUI/Widgets/Modal.h"
#include "GUI/Widgets/Sidebar.h"
#include "GUI/Widgets/StarRating.h"
#include "GUI/Widgets/Toast.h"
#include "GUI/Widgets/TopBar.h"

namespace GUI::Screens::Renter {

RenterShellScreen::RenterShellScreen() = default;

void RenterShellScreen::onEnter() {
    selectedTab_ = 0;
}

void RenterShellScreen::onExit() {
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
        {"0", "[⚽]", u8"Trang chủ & Đặt sân"},
        {"1", "[📅]", u8"Lịch sử đặt & Hủy"},
        {"2", "[⭐]", u8"Đánh giá sân bóng"},
        {"3", "[🏆]", u8"Giải đấu phong trào"},
        {"4", "[👤]", u8"Cá nhân & Ưu đãi"}
    };

    Widgets::Sidebar(
        menuItems,
        selectedTab_,
        240.0f,
        []() {
            Widgets::MoModalXacNhan(u8"Xác nhận đăng xuất");
        }
    );
}

void RenterShellScreen::renderTopBar() {
    const char* tabTitles[] = {
        u8"Trang chủ & Đặt sân",
        u8"Lịch sử đặt sân",
        u8"Đánh giá sân bóng",
        u8"Giải đấu & Sự kiện",
        u8"Thông tin cá nhân & Ưu đãi"
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
            renderTabTimDatSan();
            break;
        case 1:
            renderTabLichSuDatSan();
            break;
        case 2:
            renderTabDanhGiaSan();
            break;
        case 3:
            renderTabGiaiDau();
            break;
        case 4:
            renderTabThongTinCaNhan();
            break;
        default:
            break;
    }
}

void RenterShellScreen::renderTabTimDatSan() {
    Widgets::Card("card_renter_booking", u8"Tìm kiếm sân trống & Đặt lịch thi đấu", []() {
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
        ImGui::TextWrapped(u8"Tra cứu sân bóng còn trống theo ngày, khung giờ và loại sân (5, 7, 11 người). Chọn sân ưng ý và đặt giữ chỗ ngay lập tức.");
        ImGui::PopStyleColor();
        ImGui::Spacing();

        Widgets::BadgeTrong(u8"Sân trống sẵn sàng");
        ImGui::SameLine();
        Widgets::BadgeDaDat(u8"Khung giờ đã kín");
        ImGui::Spacing();
        ImGui::Spacing();

        if (Widgets::NutChinh(u8"Đặt sân ngay bây giờ")) {
            Widgets::ToastThongTin(u8"Chức năng đặt sân: gọi Facade::datSan");
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
        ImGui::TextWrapped(u8"// TODO: Sinh viên triển khai giao diện lưới SlotGrid chọn khung giờ và form đặt sân kết nối với BookingManager.");
        ImGui::PopStyleColor();
    });
}

void RenterShellScreen::renderTabLichSuDatSan() {
    Widgets::Card("card_renter_history", u8"Lịch sử đặt sân & Quản lý hủy đặt", []() {
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
        ImGui::TextWrapped(u8"Theo dõi toàn bộ các đơn đặt sân của bạn: Ngày giờ thi đấu, mã sân, giá tiền và trạng thái đơn. Cho phép hủy đặt sân trước giờ quy định.");
        ImGui::PopStyleColor();
        ImGui::Spacing();

        Widgets::Badge(u8"Đã xác nhận", GUI::Theme::Colors::GrassGreen, GUI::Theme::Colors::WhiteCard);
        ImGui::SameLine();
        Widgets::Badge(u8"Đã hoàn thành", GUI::Theme::Colors::BlueInfo, GUI::Theme::Colors::WhiteCard);
        ImGui::SameLine();
        Widgets::Badge(u8"Đã hủy", GUI::Theme::Colors::RedDanger, GUI::Theme::Colors::WhiteCard);
        ImGui::Spacing();
        ImGui::Spacing();

        if (Widgets::NutNguyHiem(u8"Hủy đơn đặt sân")) {
            Widgets::ToastThongTin(u8"Hủy đơn đặt: gọi Facade::huyDatSan");
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
        ImGui::TextWrapped(u8"// TODO: Sinh viên triển khai bảng Table hiển thị lịch sử đặt sân của tài khoản hiện tại từ BookingManager.");
        ImGui::PopStyleColor();
    });
}

void RenterShellScreen::renderTabDanhGiaSan() {
    static int starRating = 5;

    Widgets::Card("card_renter_review", u8"Đánh giá chất lượng sân bóng & Dịch vụ", []() {
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
        ImGui::TextWrapped(u8"Chia sẻ trải nghiệm thi đấu của bạn về chất lượng mặt cỏ, ánh sáng đèn, thái độ phục vụ để giúp cộng đồng người chơi chọn được sân tốt nhất.");
        ImGui::PopStyleColor();
        ImGui::Spacing();

        Widgets::StarRating(u8"Chọn số sao đánh giá (1 - 5 sao):", "renter_star_vote", starRating, false, 26.0f);
        ImGui::Spacing();

        if (Widgets::NutChinh(u8"Gửi đánh giá & Nhận xét")) {
            Widgets::ToastThanhCong(u8"Cảm ơn bạn đã gửi đánh giá! (Gọi Facade::guiDanhGiaSan)");
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
        ImGui::TextWrapped(u8"// TODO: Sinh viên triển khai lưu nhận xét vào ReviewManager trong include/Customer/Review.h.");
        ImGui::PopStyleColor();
    });
}

void RenterShellScreen::renderTabGiaiDau() {
    Widgets::Card("card_renter_tournament", u8"Đăng ký tham gia giải đấu bóng đá mini", []() {
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
        ImGui::TextWrapped(u8"Xem danh sách các giải đấu bóng đá phong trào sắp diễn ra tại cơ sở. Đăng ký đội bóng của bạn hoặc liên hệ chủ sân để tổ chức giải.");
        ImGui::PopStyleColor();
        ImGui::Spacing();

        Widgets::Badge(u8"Đang mở đăng ký đội", GUI::Theme::Colors::GoldAccent, GUI::Theme::Colors::WhiteCard);
        ImGui::Spacing();
        ImGui::Spacing();

        if (Widgets::NutChinh(u8"Liên hệ chủ sân tổ chức giải đấu")) {
            Widgets::ToastThongTin(u8"Gửi yêu cầu tổ chức giải đấu tới chủ sân");
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
        ImGui::TextWrapped(u8"// TODO: Sinh viên triển khai hiển thị lịch thi đấu và đăng ký đội bóng vào TournamentManager.");
        ImGui::PopStyleColor();
    });
}

void RenterShellScreen::renderTabThongTinCaNhan() {
    User* user = GUI::App::Session::getInstance().getUser();
    auto customer = dynamic_cast<::Customer*>(user);

    Widgets::Card("card_renter_profile", u8"Thông tin cá nhân & Quyền lợi ưu đãi", [customer, user]() {
        ImGui::TextUnformatted(u8"Họ và tên:");
        ImGui::SameLine(180.0f);
        ImGui::TextUnformatted(user ? user->get_name().c_str() : u8"Trần Văn Thuê");

        ImGui::TextUnformatted(u8"Số điện thoại:");
        ImGui::SameLine(180.0f);
        ImGui::TextUnformatted(user ? user->get_phone_number().c_str() : u8"0909876543");

        ImGui::TextUnformatted(u8"Số lần đã đặt sân:");
        ImGui::SameLine(180.0f);
        int bookCount = customer ? customer->get_booking_count() : 0;
        ImGui::Text(u8"%d lần", bookCount);

        ImGui::TextUnformatted(u8"Số lần đã hủy sân:");
        ImGui::SameLine(180.0f);
        int cancelCount = customer ? customer->get_cancel_count() : 0;
        ImGui::Text(u8"%d lần", cancelCount);

        ImGui::TextUnformatted(u8"Hạng thành viên:");
        ImGui::SameLine(180.0f);
        if (customer && customer->reg_customer()) {
            Widgets::BadgeVIP(u8"Thành viên VIP (Giảm 10%)");
        } else {
            Widgets::Badge(u8"Khách hàng tiêu chuẩn", GUI::Theme::Colors::GrayHover, GUI::Theme::Colors::DarkText);
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (Widgets::NutPhu(u8"Đổi mật khẩu")) {
            Widgets::ToastThongTin(u8"Mở form đổi mật khẩu tài khoản");
        }
    });
}

}  // namespace GUI::Screens::Renter
