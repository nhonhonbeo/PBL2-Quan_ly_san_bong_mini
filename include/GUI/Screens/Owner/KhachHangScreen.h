#pragma once

#include <map>
#include <string>
#include <vector>
#include "GUI/App/Facade.h"
#include "GUI/App/Screen.h"
#include "GUI/Widgets/Table.h"

namespace GUI::Screens::Owner {

// LÝ DO CHỌN: KhachHangScreen quản lý toàn bộ nghiệp vụ quan hệ khách hàng của cụm sân bóng,
// chia thành 4 tab: Danh bạ (kèm panel chi tiết), Ngưỡng ưu đãi & VIP, Danh sách đen (Blacklist & đề xuất chặn),
// và Đánh giá nhận xét của khách hàng.
class KhachHangScreen : public GUI::App::Screen {
public:
    KhachHangScreen() = default;
    ~KhachHangScreen() override = default;

    void onEnter() override;
    void render() override;
    void onExit() override;

    // Nạp lại toàn bộ dữ liệu ViewModel từ Facade sau khi có thao tác thay đổi
    void reloadViewModel();

private:
    void renderTabDanhBa();
    void renderTabUuDai();
    void renderTabDanhSachDen();
    void renderTabDanhGia();

    void renderPanelChiTietKhachHang();

    void renderModalXacNhanChan();
    void renderModalXacNhanGoChan();
    void renderModalApDungUuDai();

    // ==========================================
    // DỮ LIỆU VIEWMODEL (Nạp ở onEnter & sau thao tác)
    // ==========================================
    // Tab 1: Danh bạ & Panel chi tiết
    std::vector<GUI::App::KhachHangInfo> danhSachKhach_;
    std::vector<Widgets::TableColumn> tableColsDanhBa_;
    std::vector<Widgets::TableRow> tableRowsDanhBa_;
    Widgets::TableState tableStateDanhBa_;

    // Khách hàng đang được chọn xem chi tiết
    std::string sdtDangChon_;
    GUI::App::KhachHangInfo khachDangChon_;
    std::vector<GUI::App::DonDatSanInfo> lichSuDatCuaKhach_;
    std::vector<GUI::App::DanhGiaInfo> danhGiaCuaKhach_;

    // Tab 2: Ngưỡng ưu đãi
    std::vector<GUI::App::NguongUuDai> danhSachNguongUuDai_;

    // Tab 3: Danh sách đen & Đề xuất
    int nguongHuyDeXuat_ = 3;
    std::vector<GUI::App::KhachHangInfo> danhSachDen_;
    std::vector<GUI::App::KhachHangInfo> danhSachDeXuatChan_;

    // Biến trung gian cho Modal chặn / gỡ chặn
    std::string sdtCanChan_;
    std::string lyDoCanChan_ = u8"Hủy đặt sân nhiều lần";
    std::string sdtCanGoChan_;

    // Tab 4: Đánh giá & Điểm trung bình sân
    std::vector<GUI::App::DanhGiaInfo> danhSachDanhGia_;
    int locSoSao_ = 0; // 0: Tất cả, 1..5 sao
    float diemTBToanCoSo_ = 5.0f;
    std::map<std::string, float> diemTBTungSan_;
};

}  // namespace GUI::Screens::Owner
