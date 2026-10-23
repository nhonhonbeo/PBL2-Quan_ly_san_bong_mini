#pragma once

#include <functional>
#include <map>
#include <string>
#include <vector>
#include "GUI/App/Facade.h"
#include "GUI/App/Screen.h"
#include "GUI/Widgets/SlotGrid.h"
#include "GUI/Widgets/Table.h"

namespace GUI::Screens::Owner {

// LÝ DO CHỌN: FieldManagementScreen quản lý toàn diện các khía cạnh của sân bóng mini qua 4 tab:
// Danh sách sân (CRUD), Khung giờ hoạt động, Bảng giá ma trận và Tình trạng SlotGrid theo ngày.
class FieldManagementScreen : public GUI::App::Screen {
public:
    explicit FieldManagementScreen(std::function<void()> onChuyenSangWizard = nullptr);
    ~FieldManagementScreen() override = default;

    void onEnter() override;
    void render() override;
    void onExit() override;

    // Nạp lại toàn bộ ViewModel từ Facade sau khi có thay đổi dữ liệu
    void reloadViewModel();

private:
    void renderTabDanhSachSan();
    void renderTabKhungGio();
    void renderTabGiaThueMaTran();
    void renderTabTinhTrangTheoNgay();

    void renderModalXoaSan();
    void renderModalSuaSan();
    void renderModalXoaKhungGio();
    void renderModalThemNhanh();

    // ==========================================
    // DỮ LIỆU VIEWMODEL (Nạp ở onEnter & sau thao tác)
    // ==========================================
    // Tab a: Danh sách sân
    std::vector<GUI::App::SanBongInfo> danhSachSan_;
    std::vector<Widgets::TableColumn> tableColsSan_;
    std::vector<Widgets::TableRow> tableRowsSan_;
    Widgets::TableState tableStateSan_;

    std::string maSanDangSua_;
    std::string tenSanDangSua_;
    int loaiSanDangSua_ = 5;
    int giaThueDangSua_ = 250000;
    std::string maSanCanXoa_;
    int loaiSanThemNhanh_ = 5;
    int soLuongSanThemNhanh_ = 1;
    std::string loiThemNhanh_;

    // Tab b: Khung giờ
    std::string gioMoCua_ = "06:00";
    std::string gioDongCua_ = "23:00";
    int buocThoiLuongIdx_ = 1; // 0: 60p, 1: 90p, 2: 120p
    std::vector<GUI::App::KhungGioInfo> danhSachKhungGio_;
    std::string idKhungGioCanXoa_;

    // Tab c: Giá thuê ma trận
    bool apDungTatCaCungLoai_ = false;

    // Tab d: Tình trạng theo ngày (SlotGrid)
    std::string ngayXemSlot_ = "17/10/2026";
    std::vector<std::string> dsMaSanSlot_;
    std::vector<std::string> dsKhungGioSlot_;
    std::map<std::pair<std::string, std::string>, Widgets::SlotInfo> maTranSlot_;

    std::function<void()> onChuyenSangWizardCallback_;
};

}  // namespace GUI::Screens::Owner
