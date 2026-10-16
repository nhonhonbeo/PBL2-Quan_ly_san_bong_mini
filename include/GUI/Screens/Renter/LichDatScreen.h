#pragma once

#include <string>
#include <vector>
#include "GUI/App/Facade.h"
#include "GUI/App/Screen.h"
#include "GUI/Widgets/Table.h"

namespace GUI::Screens::Renter {

// LÝ DO CHỌN: LichDatScreen quản lý danh sách các đơn đặt sân bóng của tài khoản người thuê hiện tại,
// hỗ trợ lọc theo trạng thái (Sắp tới / Đã qua / Đã hủy), và cảnh báo số lần hủy qua Modal trước khi hủy sân.
class LichDatScreen : public GUI::App::Screen {
public:
    LichDatScreen() = default;
    ~LichDatScreen() override = default;

    void onEnter() override;
    void render() override;
    void onExit() override;

    void reloadViewModel();

private:
    void renderBoLoc();
    void renderTableDonDat();
    void renderModalXacNhanHuy();

    // Dữ liệu ViewModel
    std::vector<GUI::App::DonDatSanInfo> danhSachDonGoc_;
    std::vector<GUI::App::DonDatSanInfo> danhSachDonHienThi_;

    std::vector<Widgets::TableColumn> tableCols_;
    std::vector<Widgets::TableRow> tableRows_;
    Widgets::TableState tableState_;

    int locTrangThai_ = 0; // 0: Tất cả, 1: Sắp tới, 2: Đã qua, 3: Đã hủy
    std::string maDonCanHuy_;
    int soLanDaHuy_ = 0;
};

}  // namespace GUI::Screens::Renter
