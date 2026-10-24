#pragma once

#include <string>
#include <vector>
#include "GUI/App/Facade.h"
#include "GUI/App/Screen.h"
#include "GUI/Widgets/Calendar.h"
#include "GUI/Widgets/Table.h"

namespace GUI::Screens::Owner {

// LÝ DO CHỌN: GiaiScreen kế thừa Screen theo mô hình đa hình, quản lý toàn diện các sự kiện giải đấu bóng đá
// cho cụm sân với 4 tab: Danh sách giải, Phê duyệt yêu cầu (kèm badge đếm số lượng), Thêm giải thủ công,
// và Lịch sự kiện trực quan bằng Calendar kết hợp panel danh sách bên cạnh.
class GiaiScreen : public GUI::App::Screen {
public:
    GiaiScreen() = default;
    ~GiaiScreen() override = default;

    void onEnter() override;
    void render() override;
    void onExit() override;

    // Nạp lại toàn bộ dữ liệu ViewModel từ Facade sau khi có thay đổi
    void reloadViewModel();

private:
    void renderTabDanhSachGiai();
    void renderTabYeuCauChoDuyet();
    void renderTabThemGiaiThuCong();
    void renderTabLichSuKien();

    // Các Modal xác nhận và biểu mẫu chỉnh sửa
    void renderModalXacNhanDuyet();
    void renderModalTuChoi();
    void renderModalSuaGiai();
    void renderModalHuyGiai();

    // ==========================================
    // DỮ LIỆU VIEWMODEL
    // ==========================================
    // Tab 1: Danh sách giải đấu
    std::vector<GUI::App::GiaiDauInfo> danhSachGiai_;
    std::vector<Widgets::TableColumn> tableColsGiai_;
    std::vector<Widgets::TableRow> tableRowsGiai_;
    Widgets::TableState tableStateGiai_;

    // Tab 2: Yêu cầu chờ duyệt
    std::vector<GUI::App::GiaiDauInfo> danhSachChoDuyet_;
    std::string maGiaiDangXemDuyet_;
    std::string maGiaiCanDuyet_;
    std::string maGiaiCanTuChoi_;
    std::string lyDoTuChoi_;

    // Tab 3: Thêm giải thủ công
    std::string formTenGiai_;
    bool formTenGiaiDaCham_ = false;
    bool daGuiYeuCau_ = false;
    int formLoaiKhach_ = 0; // 0: Chọn từ danh bạ, 1: Nhập mới
    int formIndexKhach_ = 0;
    std::string formTenKhachMoi_;
    std::string formSDTKhachMoi_;
    std::string formNgayBatDau_;
    std::string formNgayKetThuc_;
    int formIndexSan_ = 0;
    int formIndexKhungGio_ = 0;
    std::string formMoTa_;

    // Dữ liệu nguồn cho Form Combos
    std::vector<GUI::App::KhachHangInfo> danhSachKhachHang_;
    std::vector<GUI::App::SanBongInfo> danhSachSan_;
    std::vector<GUI::App::KhungGioInfo> danhSachKhungGio_;

    // Tab 4: Lịch sự kiện
    Widgets::CalendarState calendarState_;
    std::vector<Widgets::SuKienLich> danhSachSuKienLich_;
    std::vector<GUI::App::GiaiDauInfo> danhSachGiaiNgayDangChon_;

    // Modal Sửa và Hủy giải
    std::string maGiaiCanHuy_;
    GUI::App::GiaiDauInfo giaiDangSua_;
    bool moPopupSua_ = false;
};

}  // namespace GUI::Screens::Owner
