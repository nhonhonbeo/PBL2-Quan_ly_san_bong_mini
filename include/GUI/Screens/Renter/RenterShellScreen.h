#pragma once

#include <memory>
#include "GUI/App/Screen.h"
#include "GUI/Screens/Renter/DanhGiaScreen.h"
#include "GUI/Screens/Renter/DatSanScreen.h"
#include "GUI/Screens/Renter/LichDatScreen.h"
#include "GUI/Screens/Renter/ProfileScreen.h"
#include "GUI/Screens/Renter/TaoGiaiScreen.h"
#include "GUI/Screens/Renter/UuDaiScreen.h"

namespace GUI::Screens::Renter {

// LÝ DO CHỌN: RenterShellScreen đóng vai trò khung vỏ (Shell) ứng dụng cho Người thuê sân,
// tích hợp Sidebar, TopBar và quản lý điều hướng giữa 6 nghiệp vụ chuyên biệt: Đặt sân theo luồng,
// Lịch đặt của tôi, Đăng ký giải đấu, Ưu đãi thành viên, Đánh giá chất lượng và Thông tin cá nhân.
class RenterShellScreen : public GUI::App::Screen {
public:
    RenterShellScreen();
    ~RenterShellScreen() override = default;

    void onEnter() override;
    void render() override;
    void onExit() override;

private:
    void renderSidebar();
    void renderTopBar();
    void renderContentArea();

    int selectedTab_ = 0;

    // Các màn hình con chuyên trách
    std::unique_ptr<DatSanScreen> datSanScreen_;
    std::unique_ptr<LichDatScreen> lichDatScreen_;
    std::unique_ptr<TaoGiaiScreen> taoGiaiScreen_;
    std::unique_ptr<UuDaiScreen> uuDaiScreen_;
    std::unique_ptr<DanhGiaScreen> danhGiaScreen_;
    std::unique_ptr<ProfileScreen> profileScreen_;
};

}  // namespace GUI::Screens::Renter
