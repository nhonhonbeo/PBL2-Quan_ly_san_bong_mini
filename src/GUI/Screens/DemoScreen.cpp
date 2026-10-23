#include "GUI/Screens/DemoScreen.h"

#include <cstdio>
#include <iomanip>
#include <sstream>
#include <imgui.h>

#include "GUI/Theme/Theme.h"
#include "GUI/App/ScreenManager.h"
#include "GUI/App/Session.h"
#include "GUI/Screens/Auth/AuthScreen.h"
#include "GUI/Widgets/Badge.h"
#include "GUI/Widgets/Button.h"
#include "GUI/Widgets/Card.h"
#include "GUI/Widgets/ComboRadio.h"
#include "GUI/Widgets/DatePicker.h"
#include "GUI/Widgets/FormValidator.h"
#include "GUI/Widgets/InputFields.h"
#include "GUI/Widgets/Modal.h"
#include "GUI/Widgets/ProgressBar.h"
#include "GUI/Widgets/StarRating.h"
#include "GUI/Widgets/TimePicker.h"
#include "GUI/Widgets/Toast.h"
#include "GUI/Widgets/TopBar.h"

namespace GUI::Screens {

void DemoScreen::onEnter() {
    // 1. Nạp danh mục Sidebar
    sidebarItems_ = {
        {"0", "", u8"Tổng quan"},
        {"1", "", u8"Quản lý sân"},
        {"2", "", u8"Lịch đặt sân"},
        {"3", "", u8"Khách hàng"},
        {"4", "", u8"Thư viện UI"}
    };
    selectedSidebarIdx_ = 4;

    // 2. Nạp danh sách Combo loại sân
    danhSachLoaiSan_ = {
        u8"Sân bóng 5 người (Cỏ nhân tạo)",
        u8"Sân bóng 7 người (Khuôn viên ngoài trời)",
        u8"Sân bóng 11 người (Sân cỏ tự nhiên thi đấu)"
    };

    // 3. Nạp danh sách 50 dòng mẫu cho Table
    tableColumns_ = {
        {u8"Mã sân", 70.0f, ImGuiTableColumnFlags_WidthFixed},
        {u8"Tên sân bóng", 160.0f, ImGuiTableColumnFlags_None},
        {u8"Loại sân", 90.0f, ImGuiTableColumnFlags_WidthFixed},
        {u8"Giá thuê (VNĐ/h)", 130.0f, ImGuiTableColumnFlags_WidthFixed},
        {u8"Khung giờ mở", 120.0f, ImGuiTableColumnFlags_WidthFixed},
        {u8"Trạng thái", 120.0f, ImGuiTableColumnFlags_WidthFixed},
        {u8"Đánh giá", 110.0f, ImGuiTableColumnFlags_WidthFixed}
    };

    tableRows_.clear();
    tableRows_.reserve(50);

    const char* tenSann[] = {
        u8"Sao Vàng", u8"Bernabeu", u8"Camp Nou", u8"Mỹ Đình", u8"Anfield",
        u8"Old Trafford", u8"San Siro", u8"Thống Nhất", u8"Hàng Đẫy", u8"Emirates"
    };

    for (int i = 1; i <= 50; ++i) {
        char maSanBuf[16];
        std::snprintf(maSanBuf, sizeof(maSanBuf), "SB%02d", i);

        std::string tenSan = std::string(u8"Sân ") + tenSann[(i - 1) % 10] + " " + (i < 10 ? "0" : "") + std::to_string(i);

        int loaiNum = (i % 3 == 1) ? 5 : ((i % 3 == 2) ? 7 : 11);
        std::string loaiStr = u8"Sân " + std::to_string(loaiNum);

        int giaTien = (loaiNum == 5) ? 250000 : ((loaiNum == 7) ? 400000 : 750000);
        giaTien += ((i % 5) * 20000);

        std::string gioMoStr = (i % 2 == 0) ? "06:00 - 23:00" : "05:30 - 22:30";

        // Trạng thái xen kẽ: Trống, Đã đặt, Bảo trì, VIP
        std::string ttStr;
        if (i % 4 == 1) ttStr = u8"Trống";
        else if (i % 4 == 2) ttStr = u8"Đã đặt";
        else if (i % 4 == 3) ttStr = u8"Bảo trì";
        else ttStr = u8"VIP";

        int sao = 3 + (i % 3); // 3, 4, 5 sao

        Widgets::TableRow row;
        row.id = maSanBuf;
        row.oDuLieu = {
            maSanBuf,
            tenSan,
            loaiStr,
            std::to_string(giaTien),
            gioMoStr,
            ttStr,
            std::to_string(sao)
        };
        tableRows_.push_back(row);
    }

    selectedRowInfo_ = u8"Chưa chọn dòng nào";

}

void DemoScreen::onExit() {
    // Dọn dẹp tài nguyên nếu có
}

void DemoScreen::render() {
    // LÝ DO CHỌN: Cửa sổ Root chiếm trọn 100% kích thước khung nhìn (Viewport) và loại bỏ thanh tiêu đề
    // mặc định của ImGui để tạo phong cách ứng dụng desktop hiện đại, không bị viền lồng viền.
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);

    ImGuiWindowFlags rootFlags = ImGuiWindowFlags_NoDecoration |
                                 ImGuiWindowFlags_NoMove |
                                 ImGuiWindowFlags_NoSavedSettings |
                                 ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    if (ImGui::Begin("##DemoScreenRootWindow", nullptr, rootFlags)) {
        // 1. Sidebar cố định bên trái
        renderSidebar();

        ImGui::SameLine(0.0f, 0.0f);

        // 2. Khu vực nội dung chính bên phải (tự co giãn linh hoạt)
        ImGui::PushStyleColor(ImGuiCol_ChildBg, GUI::Theme::Colors::LightBg);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        if (ImGui::BeginChild("##MainContentChild", ImVec2(0.0f, 0.0f), false)) {
            // Thanh TopBar
            renderTopBar();

            // Nội dung cuộn bên dưới TopBar
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(22.0f, 20.0f));
            if (ImGui::BeginChild(
                    "##ScrollableContent",
                    ImVec2(0.0f, 0.0f),
                    false
                )) {
                renderButtonSection();
                renderInputSection();
                renderValidationSection();
                renderPickerSection();
                renderFeedbackSection();
                renderTableSection();
                renderModalSection();
            }
            ImGui::EndChild();
            ImGui::PopStyleVar();
        }
        ImGui::EndChild();

        ImGui::PopStyleVar();
        ImGui::PopStyleColor();
    }
    ImGui::End();

    ImGui::PopStyleVar(2);
}

void DemoScreen::renderSidebar() {
    Widgets::Sidebar(
        sidebarItems_,
        selectedSidebarIdx_,
        240.0f,
        []() {
            GUI::App::Session::getInstance().logout();
            Widgets::ToastThongTin(u8"Đã đăng xuất tài khoản thành công!");
            GUI::App::ScreenManager::getInstance().replace(
                std::make_unique<GUI::Screens::Auth::AuthScreen>()
            );
        }
    );

}

void DemoScreen::renderTopBar() {
    std::vector<std::string> breadcrumbs = {u8"Hệ thống", u8"Thư viện UI", u8"Demo Widgets"};
    Widgets::TopBar(
        u8"Thư viện Widget & Thành phần giao diện",
        breadcrumbs,
        u8"Trần Quản Trị",
        u8"Chủ sân",
        62.0f
    );
}

void DemoScreen::renderButtonSection() {
    Widgets::Card("card_buttons", u8"1. Nút bấm (Buttons) & Hộp thoại", [this]() {
        ImGui::TextUnformatted(u8"Ba kiểu nút chuẩn hệ thống (Chính, Phụ, Nguy hiểm):");
        ImGui::Spacing();

        Widgets::NutChinh(u8"Nút Chính (#1B8A5A)");
        ImGui::SameLine();
        Widgets::NutPhu(u8"Nút Phụ (Border)");
        ImGui::SameLine();
        Widgets::NutNguyHiem(u8"Nút Nguy Hiểm (#DC3545)");

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::TextUnformatted(u8"Kích hoạt hộp thoại xác nhận thao tác:");
        ImGui::Spacing();
        if (Widgets::NutNguyHiem(u8"Mở hộp thoại xác nhận xóa sân", ImVec2(260.0f, 34.0f))) {
            Widgets::MoModalXacNhan(u8"Xác nhận xóa sân");
        }
    });
}

void DemoScreen::renderInputSection() {
    Widgets::Card("card_inputs", u8"2. Ô nhập liệu (Input Fields)", [this]() {
        // InputText
        Widgets::InputText(
            u8"Họ và tên khách hàng:",
            "input_hoten",
            textHoTen_,
            nullptr,
            u8"Nhập họ và tên..."
        );
        ImGui::Spacing();

        // InputPassword
        Widgets::InputPassword(
            u8"Mật khẩu tài khoản (có nút chuyển đổi Hiện/Ẩn):",
            "input_matkhau",
            textMatKhau_,
            hienMatKhau_,
            nullptr,
            u8"Nhập mật khẩu..."
        );
        ImGui::Spacing();

        // InputInt
        Widgets::InputInt(
            u8"Giá thuê sân theo giờ (VNĐ):",
            "input_giathue",
            intGiaThue_,
            nullptr,
            10000,
            50000
        );
        ImGui::Spacing();

        // InputPhone
        Widgets::InputPhone(
            u8"Số điện thoại (chỉ nhận số, giới hạn đúng 10 số):",
            "input_sdt",
            textSoDienThoai_,
            nullptr,
            u8"09xxxxxxxx"
        );
    });
}

void DemoScreen::renderValidationSection() {
    Widgets::Card("card_validation", u8"3. Kiểm tra tính hợp lệ biểu mẫu (FormValidator)", [this]() {
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
        ImGui::TextWrapped(u8"Kiểm tra tính hợp lệ theo thời gian thực (SĐT 10 số bắt đầu bằng 0, mật khẩu ≥ 6 ký tự, không để trống, kiểm tra ngày tháng và năm nhuận).");
        ImGui::PopStyleColor();
        ImGui::Spacing();

        Widgets::InputText(
            u8"Tên người đặt sân (*):",
            "val_ten",
            valTenKhach_,
            errTenKhach_.empty() ? nullptr : errTenKhach_.c_str(),
            u8"Nhập tên..."
        );
        ImGui::Spacing();

        Widgets::InputPhone(
            u8"Số điện thoại liên hệ (*):",
            "val_sdt",
            valSDT_,
            errSDT_.empty() ? nullptr : errSDT_.c_str(),
            u8"0xxxxxxxxx"
        );
        ImGui::Spacing();

        Widgets::InputText(
            u8"Mật khẩu xác nhận (*):",
            "val_pass",
            valMatKhau_,
            errMatKhau_.empty() ? nullptr : errMatKhau_.c_str(),
            u8"Tối thiểu 6 ký tự..."
        );
        ImGui::Spacing();

        Widgets::InputText(
            u8"Ngày tổ chức sự kiện (* dd/MM/yyyy):",
            "val_ngay",
            valNgay_,
            errNgay_.empty() ? nullptr : errNgay_.c_str(),
            u8"dd/MM/yyyy (ví dụ: 29/02/2024)"
        );
        ImGui::Spacing();

        if (Widgets::NutChinh(u8"Kiểm tra hợp lệ toàn bộ biểu mẫu", ImVec2(-1.0f, 36.0f))) {
            errTenKhach_ = Widgets::FormValidator::kiemTraKhongDeTrong(valTenKhach_, u8"Tên người đặt sân");
            errSDT_      = Widgets::FormValidator::kiemTraSDT(valSDT_);
            errMatKhau_  = Widgets::FormValidator::kiemTraMatKhau(valMatKhau_, 6);
            errNgay_     = Widgets::FormValidator::kiemTraNgayHopLe(valNgay_);

        }
    });
}

void DemoScreen::renderPickerSection() {
    Widgets::Card("card_pickers", u8"4. Lựa chọn & Bộ chọn Ngày / Giờ (Pickers)", [this]() {
        // Combo
        Widgets::Combo(
            u8"Chọn loại sân bóng:",
            "combo_loaisan",
            selectedComboLoaiSan_,
            danhSachLoaiSan_
        );
        ImGui::Spacing();

        // RadioGroup
        std::vector<std::pair<int, std::string>> radioOptions = {
            {0, u8"Tiền mặt"},
            {1, u8"Chuyển khoản QR"},
            {2, u8"Ví MoMo"}
        };
        Widgets::RadioGroup(
            u8"Hình thức thanh toán cọc:",
            "rg_thanhtoan",
            selectedRadioThanhToan_,
            radioOptions,
            true
        );
        ImGui::Spacing();
        ImGui::Spacing();

        // DatePicker tự vẽ
        Widgets::DatePicker(
            u8"Chọn ngày đặt sân (DatePicker với lịch popup tự vẽ):",
            "dp_datsan",
            datePickerVal_
        );
        ImGui::Spacing();

        // TimePicker
        Widgets::TimePicker(
            u8"Chọn khung giờ bắt đầu (TimePicker HH:mm):",
            "tp_datsan",
            timePickerVal_
        );
    });
}

void DemoScreen::renderFeedbackSection() {
    Widgets::Card("card_feedback", u8"5. Nhãn trạng thái (Badge), Đánh giá & Toast", [this]() {
        // Badges
        ImGui::TextUnformatted(u8"Nhãn trạng thái bo tròn hình viên thuốc (Badge):");
        ImGui::Spacing();

        Widgets::BadgeTrong();
        ImGui::SameLine();
        Widgets::BadgeDaDat();
        ImGui::SameLine();
        Widgets::BadgeBaoTri();
        ImGui::SameLine();
        Widgets::BadgeVIP();
        ImGui::SameLine();
        Widgets::BadgeDanhSachDen();

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // StarRating
        ImGui::TextUnformatted(u8"Đánh giá 5 sao (vẽ đa giác sắc nét bằng ImDrawList):");
        ImGui::Spacing();

        Widgets::StarRating(u8"Khách hàng chấm điểm sân (chỉnh được):", "star_edit", ratingChinhDuoc_, false, 24.0f);
        ImGui::Spacing();
        Widgets::StarRating(u8"Điểm trung bình hệ thống (chỉ xem):", "star_view", ratingChiXem_, true, 20.0f);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // ProgressBar
        ImGui::TextUnformatted(u8"Thanh tiến trình (ProgressBar có nhãn phần trăm):");
        ImGui::Spacing();
        Widgets::ProgressBar(
            progressTiLe_,
            u8"Tỷ lệ lấp đầy sân hôm nay",
            ImVec2(ImGui::GetContentRegionAvail().x, 30.0f)
        );
        ImGui::Spacing();

        // Thanh slider điều chỉnh tỷ lệ demo
        ImGui::SliderFloat(u8"Thử nghiệm kéo % tiến độ", &progressTiLe_, 0.0f, 1.0f, "%.2f");

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

    });
}

void DemoScreen::renderTableSection() {
    Widgets::Card("card_table", u8"6. Bảng dữ liệu chuyên nghiệp (ImGuiTable 50 dòng mẫu)", [this]() {
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
        ImGui::TextWrapped(u8"Bảng hỗ trợ: Ô tìm kiếm tức thời phía trên, phân trang (10 dòng mỗi trang), sắp xếp tăng/giảm theo từng cột khi click tiêu đề cột, tô xen kẽ hàng và callback khi bấm chọn dòng.");
        ImGui::PopStyleColor();
        ImGui::Spacing();

        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::GrassGreen);
        ImGui::Text(u8"Dòng đang được chọn: %s", selectedRowInfo_.c_str());
        ImGui::PopStyleColor();
        ImGui::Spacing();

        // Render Table với customRenderCell cho cột Trạng thái và Đánh giá
        Widgets::Table(
            "demo_table_50",
            tableColumns_,
            tableRows_,
            tableState_,
            [](const Widgets::TableRow& row, int col) {
                if (col == 5) {
                    // Cột trạng thái: vẽ Badge tương ứng
                    const std::string& st = row.oDuLieu[5];
                    if (st == u8"Trống") Widgets::BadgeTrong();
                    else if (st == u8"Đã đặt") Widgets::BadgeDaDat();
                    else if (st == u8"Bảo trì") Widgets::BadgeBaoTri();
                    else Widgets::BadgeVIP();
                } else if (col == 6) {
                    // Cột đánh giá: vẽ StarRating chỉ xem
                    int s = std::stoi(row.oDuLieu[6]);
                    Widgets::StarRating(nullptr, ("row_star_" + row.id).c_str(), s, true, 14.0f);
                } else {
                    ImGui::TextUnformatted(row.oDuLieu[col].c_str());
                }
            },
            [this](const Widgets::TableRow& clickedRow) {
                selectedRowInfo_ = clickedRow.oDuLieu[1] + " (" + clickedRow.oDuLieu[0] + u8", Giá: " + clickedRow.oDuLieu[3] + " đ)";
            },
            ImGuiTableFlags_ScrollX
        );
    });
}

void DemoScreen::renderModalSection() {
    Widgets::ModalXacNhan(
        u8"Xác nhận xóa sân",
        u8"Bạn có chắc chắn muốn xóa sân bóng mini này khỏi cơ sở dữ liệu không?\nHành động này mang tính phá hủy và không thể khôi phục lại.",
        []() {
        },
        u8"Xác nhận xóa",
        u8"Hủy bỏ",
        true // Chế độ nguy hiểm (nút màu đỏ)
    );
}

}  // namespace GUI::Screens
