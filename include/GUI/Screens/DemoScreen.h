#pragma once

#include <string>
#include <vector>
#include "GUI/App/Screen.h"
#include "GUI/Widgets/Sidebar.h"
#include "GUI/Widgets/Table.h"

namespace GUI::Screens {

// LÝ DO CHỌN: DemoScreen kế thừa Screen đóng vai trò phòng trưng bày (Showcase) toàn bộ
// các widget của hệ thống, giúp kiểm tra trực quan về mặt thẩm mỹ, tính thống nhất và độ ổn định.
class DemoScreen : public GUI::App::Screen {
public:
    DemoScreen() = default;
    ~DemoScreen() override = default;

    void onEnter() override;
    void render() override;
    void onExit() override;

private:
    // Tách nhỏ các khối giao diện theo quy tắc kiến trúc AGENTS.md
    void renderSidebar();
    void renderTopBar();
    void renderButtonSection();
    void renderInputSection();
    void renderValidationSection();
    void renderPickerSection();
    void renderFeedbackSection();
    void renderTableSection();
    void renderModalSection();

    // ==========================================
    // DỮ LIỆU VIEWMODEL CỦA DEMO
    // ==========================================
    // Sidebar
    std::vector<Widgets::SidebarItem> sidebarItems_;
    int selectedSidebarIdx_ = 4; // Mặc định trỏ vào mục "Thư viện UI"

    // Ô nhập liệu cơ bản
    std::string textHoTen_ = u8"Nguyễn Văn Bình";
    std::string textMatKhau_ = "MatKhau@123";
    bool hienMatKhau_ = false;
    int intGiaThue_ = 350000;
    std::string textSoDienThoai_ = "0912345678";

    // Form Validator Test
    std::string valTenKhach_ = "";
    std::string valSDT_ = "0987654321";
    std::string valMatKhau_ = "12345";
    std::string valNgay_ = "29/02/2024"; // Năm nhuận hợp lệ

    std::string errTenKhach_;
    std::string errSDT_;
    std::string errMatKhau_;
    std::string errNgay_;

    // Combo, Radio, Pickers
    int selectedComboLoaiSan_ = 1;
    std::vector<std::string> danhSachLoaiSan_;
    int selectedRadioThanhToan_ = 0;
    std::string datePickerVal_ = "20/10/2026";
    std::string timePickerVal_ = "18:30";

    // Đánh giá & Tiến độ
    int ratingChinhDuoc_ = 4;
    int ratingChiXem_ = 5;
    float progressTiLe_ = 0.72f;

    // Bảng dữ liệu 50 dòng mẫu
    std::vector<Widgets::TableColumn> tableColumns_;
    std::vector<Widgets::TableRow> tableRows_;
    Widgets::TableState tableState_;
    std::string selectedRowInfo_;

    // Modal
    bool moModalXoa_ = false;
};

}  // namespace GUI::Screens
