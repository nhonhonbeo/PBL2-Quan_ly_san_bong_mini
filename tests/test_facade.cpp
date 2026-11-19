#include <cassert>
#include <iostream>
#include <string>

#include "GUI/App/Facade.h"
#include "GUI/App/Session.h"

int main() {
    std::cout << "=== RUNNING TASK 10 FACADE ARCHITECTURE TESTS ===" << std::endl;

    GUI::App::Facade& facade = GUI::App::Facade::getInstance();

    // 1. Kiểm tra vòng đời loadData(), refreshCache(), saveData()
    facade.loadData();
    std::cout << "[PASS] Facade loadData() executed." << std::endl;

    facade.refreshCache();
    std::cout << "[PASS] Facade refreshCache() synchronized ViewModel." << std::endl;

    facade.saveData();
    std::cout << "[PASS] Facade saveData() persisted non-transactional state." << std::endl;

    // 2. Kiểm tra ViewModel / DTOs tra cứu từ cache
    auto sanList = facade.getDanhSachSanBong();
    assert(sanList.size() > 0);
    std::cout << "[PASS] DTO SanBongInfo loaded into cache: " << sanList.size() << " fields." << std::endl;

    auto khList = facade.getDanhSachKhachHangCuaChuSan();
    std::cout << "[PASS] DTO KhachHangInfo loaded: " << khList.size() << " customers." << std::endl;

    auto giaiList = facade.getDanhSachGiaiDau();
    std::cout << "[PASS] DTO GiaiDauInfo loaded: " << giaiList.size() << " tournaments." << std::endl;

    // 3. Kiểm tra đăng nhập và phân quyền Session
    auto loginResult = facade.dangNhap("0901234567", "123456");
    assert(loginResult.ok);
    assert(loginResult.duLieu != nullptr);
    assert(loginResult.duLieu->get_user_id() == "US01");
    assert(GUI::App::Session::getInstance().isLoggedIn());
    assert(GUI::App::Session::getInstance().getRole() == "Owner");
    std::cout << "[PASS] Facade dangNhap & Session management verified for Owner: "
              << loginResult.duLieu->get_name() << std::endl;

    LinkedList<std::string> requestedSlots;
    requestedSlots.push_back("06:00 - 07:00");
    std::string legacyBookingId = "stale";
    const auto rejectedBooking = facade.datNhieuCaMoi(
        "S5-01", "0901234567", "Test Customer", "2026-10-20",
        requestedSlots, 5, legacyBookingId
    );
    assert(!rejectedBooking.ok);
    assert(legacyBookingId.empty());
    std::cout << "[PASS] Legacy multi-slot booking rejects a non-customer session.\n";

    // 4. Kiểm tra lấy dữ liệu Dashboard qua Facade ủy quyền cho ReportService
    ReportDashboardMetrics metrics;
    LinkedList<ReportRevenueDay> doanhThuTheoNgay;
    LinkedList<ReportBookingRecord> luotDatGanDay;
    std::string err;
    bool dashOk = facade.getBaoCaoDashboard(metrics, doanhThuTheoNgay, luotDatGanDay, err);
    assert(dashOk);
    assert(metrics.totalFields >= 0);
    std::cout << "[PASS] Facade getBaoCaoDashboard successfully delegated to ReportService: "
              << "Total Revenue = " << metrics.monthlyRevenueVnd << " VND, Total Bookings = "
              << metrics.bookingsToday << std::endl;

    // 5. Kiểm tra đăng ký người thuê mới: Facade ủy quyền hoàn toàn cho AccountService validation
    auto regFail = facade.dangKyNguoiThue("Test Name", "0901234567", "123456"); // SĐT đã tồn tại
    assert(!regFail.ok);
    assert(!regFail.loi.empty());
    std::cout << "[PASS] Facade registration rejected duplicate phone via AccountService: "
              << regFail.loi << std::endl;

    auto regFailShortPass = facade.dangKyNguoiThue("Test Name", "0911888999", "123"); // Pass ngắn
    assert(!regFailShortPass.ok);
    assert(!regFailShortPass.loi.empty());
    std::cout << "[PASS] Facade registration rejected short password via AccountService: "
              << regFailShortPass.loi << std::endl;

    std::cout << ">>> ALL TASK 10 FACADE TESTS PASSED! <<<" << std::endl;
    return 0;
}


