#pragma once

#include "GUI/App/Screen.h"

namespace GUI::Screens::Renter {

// LÝ DO CHỌN: RenterShellScreen đóng vai trò khung vỏ (Shell) ứng dụng cho Người thuê sân,
// tích hợp Sidebar, TopBar và vùng nội dung co giãn linh hoạt cho các nghiệp vụ đặt sân và cá nhân.
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

    // Các hàm render từng tab nghiệp vụ của Người thuê
    void renderTabTimDatSan();
    void renderTabLichSuDatSan();
    void renderTabDanhGiaSan();
    void renderTabGiaiDau();
    void renderTabThongTinCaNhan();

    int selectedTab_ = 0;
};

}  // namespace GUI::Screens::Renter
