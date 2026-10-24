#pragma once

#include <string>
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
    ReportDashboardMetrics metrics_;
    std::string loiBaoCao_;
    LinkedList<ReportRevenueDay> doanhThu7Ngay_;
    LinkedList<ReportBookingRecord> luotDatGanDay_;
};

}  // namespace GUI::Screens::Owner
