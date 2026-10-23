#pragma once

#include <functional>
#include <map>
#include <string>
#include <vector>
#include "GUI/App/Facade.h"
#include "GUI/App/Screen.h"
#include "GUI/Widgets/SlotGrid.h"

namespace GUI::Screens::Renter {

// LÝ DO CHỌN: DatSanScreen quản lý toàn bộ luồng quy trình đặt giữ chỗ sân bóng cho người thuê
// theo dạng wizard từng bước trực quan, tự động chặn nếu tài khoản thuộc danh sách đen,
// và xuất phiếu đặt sân điện tử sau khi thanh toán thành công.
class DatSanScreen : public GUI::App::Screen {
public:
    DatSanScreen(std::function<void()> onXemLichCuaToi = nullptr);
    ~DatSanScreen() override = default;

    void onEnter() override;
    void render() override;
    void onExit() override;

    void reloadViewModel();

private:
    void renderThanhTienTrinh();
    void renderCanhBaoDanhSachDen();

    // Các bước đặt sân
    void renderBuoc1ChonLoaiSan();
    void renderBuoc2ChonNgayVaKhungGio();
    void renderBuoc3XacNhan();
    void renderPhieuDatSan();

    std::function<void()> onXemLichCuaToi_;

    // Trạng thái Wizard
    int buocHienTai_ = 1; // 1: Loại sân, 2: Ngày và SlotGrid, 3: Xác nhận, 4: Phiếu đặt
    bool biChanDatSan_ = false;

    // Dữ liệu lựa chọn đặt sân
    int loaiSanChon_ = 5; // 5, 7, 11
    std::string ngayChon_;
    std::string maSanChon_;
    std::string khungGioChon_;
    int giaGoc_ = 0;
    int phanTramGiam_ = 0;
    int soTienGiam_ = 0;
    int tongTien_ = 0;
    std::string maDonDaDat_;

    // Dữ liệu ViewModel
    std::map<int, int> giaThapNhatTheoLoai_;
    std::vector<GUI::App::SanBongInfo> danhSachSanTheoLoai_;
    std::vector<GUI::App::KhungGioInfo> danhSachKhungGio_;
    std::map<std::pair<std::string, std::string>, Widgets::SlotInfo> slotDataMap_;
};

}  // namespace GUI::Screens::Renter
