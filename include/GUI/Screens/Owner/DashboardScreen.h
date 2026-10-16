#pragma once

#include <string>
#include <vector>
#include "GUI/App/Facade.h"
#include "GUI/App/Screen.h"

namespace GUI::Screens::Owner {

// LÝ DO CHỌN: DashboardScreen quản lý màn hình tổng quan của Chủ sân với 4 thẻ KPI thống kê,
// biểu đồ cột doanh thu 7 ngày tự vẽ vector bằng ImDrawList và bảng danh sách lượt đặt sắp tới.
class DashboardScreen : public GUI::App::Screen {
public:
    DashboardScreen() = default;
    ~DashboardScreen() override = default;

    void onEnter() override;
    void render() override;
    void onExit() override;

private:
    void renderStatCards();
    void renderRevenueChart();
    void renderUpcomingBookings();

    // ==========================================
    // DỮ LIỆU VIEWMODEL (Nạp ở onEnter)
    // ==========================================
    int tongSoSan_ = 0;
    int luotDatHomNay_ = 0;
    long long doanhThuThang_ = 0;
    int soKhachMoi_ = 0;

    std::vector<std::pair<std::string, long long>> doanhThu7Ngay_;
    std::vector<GUI::App::DonDatSanInfo> luotDatSapToi_;
};

}  // namespace GUI::Screens::Owner
