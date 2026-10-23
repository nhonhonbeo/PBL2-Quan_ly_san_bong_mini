#include "GUI/Screens/Owner/GiaiScreen.h"

#include <algorithm>
#include <cstdio>
#include <string>
#include <vector>
#include <imgui.h>

#include "GUI/Theme/Theme.h"
#include "GUI/Widgets/Badge.h"
#include "GUI/Widgets/Button.h"
#include "GUI/Widgets/Card.h"
#include "GUI/Widgets/ComboRadio.h"
#include "GUI/Widgets/DatePicker.h"
#include "GUI/Widgets/FormValidator.h"
#include "GUI/Widgets/InputFields.h"
#include "GUI/Widgets/Modal.h"
#include "GUI/Widgets/Toast.h"

namespace GUI::Screens::Owner {

namespace {

// Chuyển chuỗi ngày "DD/MM/YYYY" hoặc "YYYY-MM-DD" thành số nguyên YYYYMMDD để so sánh khoảng thời gian
int parseNgaySangSo(const std::string& str) {
    if (str.length() == 10) {
        if (str[2] == '/' && str[5] == '/') {
            try {
                int d = std::stoi(str.substr(0, 2));
                int m = std::stoi(str.substr(3, 2));
                int y = std::stoi(str.substr(6, 4));
                return y * 10000 + m * 100 + d;
            } catch (...) {}
        } else if (str[4] == '-' && str[7] == '-') {
            try {
                int y = std::stoi(str.substr(0, 4));
                int m = std::stoi(str.substr(5, 2));
                int d = std::stoi(str.substr(8, 2));
                return y * 10000 + m * 100 + d;
            } catch (...) {}
        }
    }
    return 0;
}

// Kiểm tra một ngày cụ thể có nằm trong khoảng [ngayBatDau, ngayKetThuc] hay không
bool namTrongKhoangNgay(const std::string& targetNgay, const std::string& startNgay, const std::string& endNgay) {
    int t = parseNgaySangSo(targetNgay);
    int s = parseNgaySangSo(startNgay);
    int e = parseNgaySangSo(endNgay);
    if (t == 0 || s == 0 || e == 0) return false;
    return (t >= s && t <= e);
}

const char* tenTrangThai(GUI::App::TrangThaiGiaiDau tt) {
    switch (tt) {
        case GUI::App::TrangThaiGiaiDau::ChoDuyet:   return u8"Chờ duyệt";
        case GUI::App::TrangThaiGiaiDau::DaDuyet:    return u8"Đã duyệt";
        case GUI::App::TrangThaiGiaiDau::DangDienRa: return u8"Đang diễn ra";
        case GUI::App::TrangThaiGiaiDau::DaKetThuc:  return u8"Đã kết thúc";
        case GUI::App::TrangThaiGiaiDau::TuChoi:     return u8"Bị từ chối";
    }
    return u8"Không xác định";
}

}  // namespace

void GiaiScreen::onEnter() {
    tableColsGiai_ = {
        {u8"Mã giải", 90.0f, ImGuiTableColumnFlags_WidthFixed},
        {u8"Tên giải đấu", 220.0f, ImGuiTableColumnFlags_None},
        {u8"Khách đặt", 150.0f, ImGuiTableColumnFlags_WidthFixed},
        {u8"Số điện thoại", 130.0f, ImGuiTableColumnFlags_WidthFixed},
        {u8"Bắt đầu", 110.0f, ImGuiTableColumnFlags_WidthFixed},
        {u8"Kết thúc", 110.0f, ImGuiTableColumnFlags_WidthFixed},
        {u8"Trạng thái", 130.0f, ImGuiTableColumnFlags_WidthFixed},
        {u8"Thao tác", 190.0f, ImGuiTableColumnFlags_WidthFixed}
    };

    calendarState_.thang = 10;
    calendarState_.nam = 2026;
    calendarState_.ngayDangChon = "20/10/2026";
    calendarState_.daKhoiTao = true;

    // Khởi tạo ngày mặc định cho Form thêm giải
    formNgayBatDau_ = "25/10/2026";
    formNgayKetThuc_ = "30/10/2026";

    reloadViewModel();
}

void GiaiScreen::onExit() {
}

void GiaiScreen::reloadViewModel() {
    auto& facade = GUI::App::Facade::getInstance();

    // 1. Nạp danh sách giải đấu toàn bộ
    // LÝ DO CHỌN: LinkedList lưu trữ danh sách giải đấu cho phép chèn và xóa phần tử ở mọi vị trí với O(1)
    danhSachGiai_ = facade.getDanhSachGiaiDau();
    danhSachChoDuyet_ = facade.getYeuCauGiaiDauChoDuyet();

    // 2. Chuyển đổi sang TableRows cho Tab 1
    tableRowsGiai_.clear();
    for (const auto& g : danhSachGiai_) {
        Widgets::TableRow row;
        row.id = g.maGiai;
        row.oDuLieu = {
            g.maGiai,
            g.tenGiai,
            g.tenKhachDat,
            g.sdtKhach,
            g.ngayBatDau,
            g.ngayKetThuc,
            tenTrangThai(g.trangThai),
            "" // Cột thao tác vẽ tùy chỉnh
        };
        tableRowsGiai_.push_back(row);
    }

    // 3. Nạp danh sách cho Form Combos
    danhSachKhachHang_ = facade.getDanhSachKhachHang();
    danhSachSan_ = facade.getDanhSachSanBong();
    danhSachKhungGio_ = facade.getDanhSachKhungGio();

    // 4. Nạp danh sách sự kiện trên Lịch Calendar
    danhSachSuKienLich_.clear();
    for (const auto& g : danhSachGiai_) {
        // Chỉ vẽ sự kiện các giải đã duyệt hoặc đang diễn ra
        if (g.trangThai == GUI::App::TrangThaiGiaiDau::DaDuyet || g.trangThai == GUI::App::TrangThaiGiaiDau::DangDienRa) {
            // Đánh dấu sự kiện ngày bắt đầu và ngày kết thúc
            Widgets::SuKienLich skStart;
            skStart.ngay = g.ngayBatDau;
            skStart.nhan = u8"Bắt đầu: " + g.tenGiai + u8" (Sân " + g.maSan + ")";
            skStart.mau = (g.trangThai == GUI::App::TrangThaiGiaiDau::DangDienRa)
                          ? IM_COL32(245, 179, 1, 255)   // Vàng cho đang diễn ra
                          : IM_COL32(27, 138, 90, 255);  // Xanh cỏ cho đã duyệt
            skStart.id = g.maGiai;
            danhSachSuKienLich_.push_back(skStart);

            if (g.ngayKetThuc != g.ngayBatDau) {
                Widgets::SuKienLich skEnd;
                skEnd.ngay = g.ngayKetThuc;
                skEnd.nhan = u8"Chung kết: " + g.tenGiai + u8" (Sân " + g.maSan + ")";
                skEnd.mau = IM_COL32(220, 53, 69, 255); // Đỏ cho chung kết
                skEnd.id = g.maGiai;
                danhSachSuKienLich_.push_back(skEnd);
            }
        }
    }

    // 5. Lọc danh sách giải đấu trong ngày đang chọn trên Calendar
    danhSachGiaiNgayDangChon_.clear();
    for (const auto& g : danhSachGiai_) {
        if (namTrongKhoangNgay(calendarState_.ngayDangChon, g.ngayBatDau, g.ngayKetThuc)) {
            danhSachGiaiNgayDangChon_.push_back(g);
        }
    }
}

void GiaiScreen::render() {
    if (ImGui::BeginTabBar("##GiaiTabBar", ImGuiTabBarFlags_None)) {
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::WhiteCard);
        bool showTournamentList = ImGui::BeginTabItem(u8"1. Danh sách giải đấu");
        ImGui::PopStyleColor();
        if (showTournamentList) {
            renderTabDanhSachGiai();
            ImGui::EndTabItem();
        }

        // Tab có hiển thị số lượng yêu cầu chờ duyệt
        std::string tabDuyetTitle = u8"2. Yêu cầu chờ duyệt (" + std::to_string(danhSachChoDuyet_.size()) + u8")";
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::WhiteCard);
        bool showPending = ImGui::BeginTabItem(tabDuyetTitle.c_str());
        ImGui::PopStyleColor();
        if (showPending) {
            renderTabYeuCauChoDuyet();
            ImGui::EndTabItem();
        }

        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::WhiteCard);
        bool showManualEntry = ImGui::BeginTabItem(u8"3. Thêm giải thủ công");
        ImGui::PopStyleColor();
        if (showManualEntry) {
            renderTabThemGiaiThuCong();
            ImGui::EndTabItem();
        }

        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::WhiteCard);
        bool showEventSchedule = ImGui::BeginTabItem(u8"4. Lịch sự kiện giải đấu");
        ImGui::PopStyleColor();
        if (showEventSchedule) {
            renderTabLichSuKien();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    // Render các Modal xác nhận và Popup
    renderModalXacNhanDuyet();
    renderModalTuChoi();
    renderModalSuaGiai();
    renderModalHuyGiai();
}

void GiaiScreen::renderTabDanhSachGiai() {
    Widgets::Card("card_owner_tournaments", u8"Quản lý danh sách các giải đấu bóng đá", [this]() {
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
        ImGui::TextWrapped(u8"Theo dõi và quản lý mọi giải đấu phong trào tại cụm sân. Tìm kiếm theo tên giải hoặc khách tổ chức. Bấm Sửa hoặc Hủy giải khi có thay đổi kế hoạch.");
        ImGui::PopStyleColor();
        ImGui::Spacing();

        Widgets::Table(
            "tbl_owner_tournaments",
            tableColsGiai_,
            tableRowsGiai_,
            tableStateGiai_,
            [this](const Widgets::TableRow& row, int col) {
                if (col == 6) {
                    // Cột Trạng thái Badge
                    const std::string& tt = row.oDuLieu[6];
                    if (tt == u8"Chờ duyệt") {
                        Widgets::Badge(tt.c_str(), GUI::Theme::Colors::GoldAccent, GUI::Theme::Colors::WhiteCard);
                    } else if (tt == u8"Đã duyệt") {
                        Widgets::Badge(tt.c_str(), GUI::Theme::Colors::GrassGreen, GUI::Theme::Colors::WhiteCard);
                    } else if (tt == u8"Đang diễn ra") {
                        Widgets::BadgeVIP(tt.c_str());
                    } else if (tt == u8"Bị từ chối") {
                        Widgets::BadgeDanhSachDen(tt.c_str());
                    } else {
                        Widgets::Badge(tt.c_str(), GUI::Theme::Colors::GrayHover, GUI::Theme::Colors::DarkText);
                    }
                } else if (col == 7) {
                    // Cột Thao tác: Sửa / Hủy
                    std::string btnEditId = u8"Sửa##" + row.id;
                    std::string btnCancelId = u8"Hủy##" + row.id;

                    if (Widgets::NutPhu(btnEditId.c_str(), ImVec2(54.0f, 24.0f))) {
                        auto* g = GUI::App::Facade::getInstance().getGiaiDau(row.id);
                        if (g) {
                            giaiDangSua_ = *g;
                            moPopupSua_ = true;
                        }
                    }

                    ImGui::SameLine(0.0f, 6.0f);

                    if (Widgets::NutNguyHiem(btnCancelId.c_str(), ImVec2(54.0f, 24.0f))) {
                        maGiaiCanHuy_ = row.id;
                        Widgets::MoModalXacNhan(u8"Xác nhận hủy giải đấu");
                    }
                } else {
                    ImGui::TextUnformatted(row.oDuLieu[col].c_str());
                }
            }
        );
    });
}

void GiaiScreen::renderTabYeuCauChoDuyet() {
    Widgets::Card("card_pending_tournaments", u8"Danh sách yêu cầu đăng ký giải đấu chờ duyệt", [this]() {
        if (danhSachChoDuyet_.empty()) {
            ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::GrassGreen);
            ImGui::TextUnformatted(u8"Hiện tại không có yêu cầu tổ chức giải đấu nào đang chờ duyệt.");
            ImGui::PopStyleColor();
            return;
        }

        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
        ImGui::Text(u8"Có %zu yêu cầu đang chờ phê duyệt. Khi bấm Duyệt, hệ thống sẽ tự động kiểm tra trùng lịch sân bóng.", danhSachChoDuyet_.size());
        ImGui::PopStyleColor();
        ImGui::Spacing();
        ImGui::Spacing();

        for (const auto& g : danhSachChoDuyet_) {
            std::string subCardId = "card_req_" + g.maGiai;
            std::string subTitle = g.maGiai + " - " + g.tenGiai;

            Widgets::Card(subCardId.c_str(), subTitle.c_str(), [this, &g]() {
                ImGui::Columns(2, nullptr, false);
                ImGui::SetColumnWidth(0, 360.0f);

                ImGui::Text(u8"• Người đặt: %s", g.tenKhachDat.c_str());
                ImGui::Text(u8"• Số điện thoại: %s", g.sdtKhach.c_str());
                ImGui::Text(u8"• Sân đăng ký: %s", g.maSan.c_str());
                ImGui::Text(u8"• Khung giờ: %s", g.khungGio.c_str());
                ImGui::Text(u8"• Thời gian thi đấu: %s  ->  %s", g.ngayBatDau.c_str(), g.ngayKetThuc.c_str());

                ImGui::NextColumn();

                ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
                ImGui::TextUnformatted(u8"Điều lệ / Ghi chú sự kiện:");
                ImGui::PopStyleColor();
                ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::DarkText);
                ImGui::TextWrapped(u8"\"%s\"", g.moTa.empty() ? u8"(Không có ghi chú thêm)" : g.moTa.c_str());
                ImGui::PopStyleColor();

                ImGui::Spacing();
                ImGui::Spacing();

                // Nút Duyệt và Nút Từ chối
                std::string btnApproveId = u8"Duyệt giải đấu##" + g.maGiai;
                std::string btnRejectId = u8"Từ chối##" + g.maGiai;

                if (Widgets::NutChinh(btnApproveId.c_str(), ImVec2(160.0f, 32.0f))) {
                    maGiaiCanDuyet_ = g.maGiai;
                    Widgets::MoModalXacNhan(u8"Xác nhận phê duyệt giải đấu");
                }

                ImGui::SameLine(0.0f, 12.0f);

                if (Widgets::NutNguyHiem(btnRejectId.c_str(), ImVec2(120.0f, 32.0f))) {
                    maGiaiCanTuChoi_ = g.maGiai;
                    lyDoTuChoi_.clear();
                    Widgets::MoModalXacNhan(u8"Nhập lý do từ chối yêu cầu");
                }

                ImGui::Columns(1);
            });

            ImGui::Spacing();
        }
    });
}

void GiaiScreen::renderTabThemGiaiThuCong() {
    Widgets::Card("card_create_tournament_manual", u8"Đăng ký giải đấu & sự kiện sân bóng mới", [this]() {
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
        ImGui::TextWrapped(u8"Điền đầy đủ thông tin giải đấu và chọn sân bãi, thời gian. Nghiệp vụ sẽ kiểm tra trùng lịch sân bóng trước khi tạo thành công.");
        ImGui::PopStyleColor();
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Columns(2, "cols_form_tournament", false);
        ImGui::SetColumnWidth(0, 480.0f);

        // 1. Tên giải đấu
        Widgets::InputText(u8"Tên giải đấu (*)", "##form_t_name", formTenGiai_, u8"Ví dụ: Cúp Tứ Hùng Doanh Nghiệp IT 2026");
        ImGui::Spacing();

        // 2. Chọn khách hàng
        ImGui::TextUnformatted(u8"Thông tin khách hàng tổ chức (*):");
        Widgets::Radio(u8"Chọn từ danh bạ khách", formLoaiKhach_, 0);
        ImGui::SameLine(0.0f, 16.0f);
        Widgets::Radio(u8"Nhập khách hàng mới", formLoaiKhach_, 1);
        ImGui::Spacing();

        if (formLoaiKhach_ == 0) {
            std::vector<std::string> dsTenKhach;
            for (const auto& kh : danhSachKhachHang_) {
                dsTenKhach.push_back(kh.hoTen + " (" + kh.sdt + ")");
            }
            if (!dsTenKhach.empty()) {
                if (formIndexKhach_ >= static_cast<int>(dsTenKhach.size())) formIndexKhach_ = 0;
                Widgets::Combo(u8"Khách hàng đại diện", "##form_t_cust_combo", formIndexKhach_, dsTenKhach);
            }
        } else {
            Widgets::InputText(u8"Họ và tên khách (*)", "##form_t_cust_name", formTenKhachMoi_, u8"Nguyễn Văn A");
            Widgets::InputPhone(u8"Số điện thoại (*)", "##form_t_cust_phone", formSDTKhachMoi_);
        }
        ImGui::Spacing();

        // 3. Sân bóng và Khung giờ
        std::vector<std::string> dsTenSan;
        for (const auto& s : danhSachSan_) {
            dsTenSan.push_back(s.maSan + " - " + s.tenSan);
        }
        if (!dsTenSan.empty()) {
            if (formIndexSan_ >= static_cast<int>(dsTenSan.size())) formIndexSan_ = 0;
            Widgets::Combo(u8"Sân thi đấu (*)", "##form_t_field_combo", formIndexSan_, dsTenSan);
        }
        ImGui::Spacing();

        std::vector<std::string> dsKhungGio;
        for (const auto& kg : danhSachKhungGio_) {
            dsKhungGio.push_back(kg.batDau + " - " + kg.ketThuc);
        }
        if (!dsKhungGio.empty()) {
            if (formIndexKhungGio_ >= static_cast<int>(dsKhungGio.size())) formIndexKhungGio_ = 0;
            Widgets::Combo(u8"Khung giờ thi đấu (*)", "##form_t_slot_combo", formIndexKhungGio_, dsKhungGio);
        }

        ImGui::NextColumn();

        // Cột phải: Ngày bắt đầu / kết thúc bằng DatePicker & Điều lệ
        Widgets::DatePicker(u8"Ngày bắt đầu giải (*)", "##dp_t_start", formNgayBatDau_);
        ImGui::Spacing();

        Widgets::DatePicker(u8"Ngày kết thúc giải (*)", "##dp_t_end", formNgayKetThuc_);
        ImGui::Spacing();

        ImGui::TextUnformatted(u8"Điều lệ / Quy chế giải đấu:");
        ImGui::Spacing();
        char descBuf[1024];
        std::snprintf(descBuf, sizeof(descBuf), "%s", formMoTa_.c_str());
        if (ImGui::InputTextMultiline("##form_t_desc", descBuf, sizeof(descBuf), ImVec2(-1.0f, 130.0f))) {
            formMoTa_ = descBuf;
        }

        ImGui::Spacing();
        ImGui::Spacing();

        // Nút Lưu tạo giải đấu
        if (Widgets::NutChinh(u8"+ Xác nhận tạo giải đấu", ImVec2(-1.0f, 40.0f))) {
            // Kiểm tra tính hợp lệ của Form
            if (formTenGiai_.empty()) {
                return;
            }

            GUI::App::GiaiDauInfo gMoi;
            gMoi.tenGiai = formTenGiai_;
            if (formLoaiKhach_ == 0 && !danhSachKhachHang_.empty()) {
                gMoi.tenKhachDat = danhSachKhachHang_[formIndexKhach_].hoTen;
                gMoi.sdtKhach = danhSachKhachHang_[formIndexKhach_].sdt;
            } else {
                std::string loiSDT = Widgets::FormValidator::kiemTraSDT(formSDTKhachMoi_);
                if (formTenKhachMoi_.empty() || !loiSDT.empty()) {
                    return;
                }
                gMoi.tenKhachDat = formTenKhachMoi_;
                gMoi.sdtKhach = formSDTKhachMoi_;
            }

            if (!danhSachSan_.empty()) {
                gMoi.maSan = danhSachSan_[formIndexSan_].maSan;
            }
            if (!danhSachKhungGio_.empty()) {
                gMoi.khungGio = danhSachKhungGio_[formIndexKhungGio_].batDau + " - " + danhSachKhungGio_[formIndexKhungGio_].ketThuc;
            }
            gMoi.ngayBatDau = formNgayBatDau_;
            gMoi.ngayKetThuc = formNgayKetThuc_;
            gMoi.moTa = formMoTa_;

            // Gọi Facade kiểm tra trùng lịch và thêm giải
            auto kq = GUI::App::Facade::getInstance().taoGiaiDauThuCong(gMoi);
            if (kq.ok) {
                formTenGiai_.clear();
                formMoTa_.clear();
                reloadViewModel();
            } else {
                // Hiển thị Toast đỏ nội dung lỗi do nghiệp vụ trả về
            }
        }

        ImGui::Columns(1);
    });
}

void GiaiScreen::renderTabLichSuKien() {
    float availW = ImGui::GetContentRegionAvail().x;
    float calW = availW * 0.58f;
    float panelW = availW - calW - 16.0f;

    // Cột trái: Lịch tháng Calendar
    ImGui::BeginGroup();
    ImGui::PushItemWidth(calW);

    Widgets::Card("card_cal_tournaments", u8"Lịch sự kiện & giải đấu cụm sân bóng", [this, calW]() {
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
        ImGui::TextWrapped(u8"Các ngày có chấm màu là ngày có giải đấu hoặc chung kết. Click vào ngày để xem danh sách các giải diễn ra.");
        ImGui::PopStyleColor();
        ImGui::Spacing();

        Widgets::Calendar(
            "cal_events",
            calendarState_,
            danhSachSuKienLich_,
            [this](const std::string& ngay) {
                calendarState_.ngayDangChon = ngay;
                danhSachGiaiNgayDangChon_.clear();
                for (const auto& g : danhSachGiai_) {
                    if (namTrongKhoangNgay(ngay, g.ngayBatDau, g.ngayKetThuc)) {
                        danhSachGiaiNgayDangChon_.push_back(g);
                    }
                }
            },
            ImVec2(calW - 24.0f, 380.0f)
        );
    });

    ImGui::PopItemWidth();
    ImGui::EndGroup();

    ImGui::SameLine(0.0f, 16.0f);

    // Cột phải: Panel danh sách các giải đấu trong ngày đang chọn
    ImGui::BeginGroup();
    ImGui::PushItemWidth(panelW);

    std::string panelTitle = u8"Giải đấu ngày: " + calendarState_.ngayDangChon;
    Widgets::Card("card_cal_day_detail", panelTitle.c_str(), [this]() {
        if (danhSachGiaiNgayDangChon_.empty()) {
            ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
            ImGui::TextWrapped(u8"Không có giải đấu nào diễn ra trong ngày %s.", calendarState_.ngayDangChon.c_str());
            ImGui::PopStyleColor();
            return;
        }

        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::GrassGreen);
        ImGui::Text(u8"Có %zu giải đấu đang hoạt động trong ngày này:", danhSachGiaiNgayDangChon_.size());
        ImGui::PopStyleColor();
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        for (const auto& g : danhSachGiaiNgayDangChon_) {
            ImGui::BeginGroup();

            ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::DarkText);
            ImGui::Text(u8"%s (%s)", g.tenGiai.c_str(), g.maGiai.c_str());
            ImGui::PopStyleColor();

            ImGui::Spacing();
            Widgets::Badge((u8"Sân " + g.maSan).c_str(), GUI::Theme::Colors::GrassGreen, GUI::Theme::Colors::WhiteCard);
            ImGui::SameLine(0.0f, 6.0f);
            Widgets::Badge(g.khungGio.c_str(), GUI::Theme::Colors::GrayHover, GUI::Theme::Colors::DarkText);
            ImGui::SameLine(0.0f, 6.0f);
            Widgets::Badge(tenTrangThai(g.trangThai), GUI::Theme::Colors::GoldAccent, GUI::Theme::Colors::WhiteCard);

            ImGui::Spacing();
            ImGui::Text(u8"• Khách đặt: %s (%s)", g.tenKhachDat.c_str(), g.sdtKhach.c_str());
            ImGui::Text(u8"• Thời gian: %s -> %s", g.ngayBatDau.c_str(), g.ngayKetThuc.c_str());

            ImGui::Spacing();
            std::string btnEditId = u8"Chỉnh sửa##sub_" + g.maGiai;
            std::string btnCancelId = u8"Hủy giải##sub_" + g.maGiai;

            if (Widgets::NutPhu(btnEditId.c_str(), ImVec2(90.0f, 26.0f))) {
                auto* ptr = GUI::App::Facade::getInstance().getGiaiDau(g.maGiai);
                if (ptr) {
                    giaiDangSua_ = *ptr;
                    moPopupSua_ = true;
                }
            }

            ImGui::SameLine(0.0f, 8.0f);

            if (Widgets::NutNguyHiem(btnCancelId.c_str(), ImVec2(90.0f, 26.0f))) {
                maGiaiCanHuy_ = g.maGiai;
                Widgets::MoModalXacNhan(u8"Xác nhận hủy giải đấu");
            }

            ImGui::EndGroup();

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
        }
    });

    ImGui::PopItemWidth();
    ImGui::EndGroup();
}

void GiaiScreen::renderModalXacNhanDuyet() {
    std::string noiDung = u8"Bạn có chắc chắn muốn phê duyệt giải đấu mã " + maGiaiCanDuyet_ +
                          u8"Hệ thống sẽ kiểm tra trùng lịch sân bóng trước khi xác nhận.";

    Widgets::ModalXacNhan(
        u8"Xác nhận phê duyệt giải đấu",
        noiDung.c_str(),
        [this]() {
            // Khi duyệt gọi Facade kiểm tra trùng lịch sân bóng
            auto kq = GUI::App::Facade::getInstance().duyetGiaiDau(maGiaiCanDuyet_);
            if (kq.ok) {
                reloadViewModel();
            } else {
                // Hiện Toast đỏ nội dung lỗi do nghiệp vụ trả về
            }
        },
        u8"Phê duyệt",
        u8"Đóng",
        false
    );
}

void GiaiScreen::renderModalTuChoi() {
    Widgets::KiemTraVaMoModal(u8"Nhập lý do từ chối yêu cầu");

    if (ImGui::BeginPopupModal(u8"Nhập lý do từ chối yêu cầu", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::DarkText);
        ImGui::Text(u8"Vui lòng nhập lý do từ chối giải đấu mã %s:", maGiaiCanTuChoi_.c_str());
        ImGui::PopStyleColor();
        ImGui::Spacing();

        char buf[256];
        std::snprintf(buf, sizeof(buf), "%s", lyDoTuChoi_.c_str());
        ImGui::SetNextItemWidth(360.0f);
        if (ImGui::InputText("##input_reject_reason", buf, sizeof(buf))) {
            lyDoTuChoi_ = buf;
        }

        ImGui::Spacing();
        ImGui::Spacing();

        if (Widgets::NutNguyHiem(u8"Xác nhận từ chối", ImVec2(160.0f, 32.0f))) {
            auto kq = GUI::App::Facade::getInstance().tuChoiGiaiDau(maGiaiCanTuChoi_, lyDoTuChoi_);
            if (kq.ok) {
                reloadViewModel();
            } else {
            }
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine(0.0f, 12.0f);

        if (Widgets::NutPhu(u8"Hủy bỏ", ImVec2(100.0f, 32.0f))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void GiaiScreen::renderModalSuaGiai() {
    if (moPopupSua_) {
        ImGui::OpenPopup(u8"Chỉnh sửa thông tin giải đấu");
        moPopupSua_ = false;
    }

    if (ImGui::BeginPopupModal(u8"Chỉnh sửa thông tin giải đấu", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::DarkText);
        ImGui::Text(u8"Cập nhật thông tin cho giải đấu: %s", giaiDangSua_.maGiai.c_str());
        ImGui::PopStyleColor();
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Tên giải
        char tenBuf[128];
        std::snprintf(tenBuf, sizeof(tenBuf), "%s", giaiDangSua_.tenGiai.c_str());
        ImGui::SetNextItemWidth(380.0f);
        if (ImGui::InputText(u8"Tên giải đấu", tenBuf, sizeof(tenBuf))) {
            giaiDangSua_.tenGiai = tenBuf;
        }
        ImGui::Spacing();

        // Ngày bắt đầu / kết thúc
        Widgets::DatePicker(u8"Ngày bắt đầu", "##edit_dp_start", giaiDangSua_.ngayBatDau);
        Widgets::DatePicker(u8"Ngày kết thúc", "##edit_dp_end", giaiDangSua_.ngayKetThuc);
        ImGui::Spacing();

        // Sân bóng
        char sanBuf[32];
        std::snprintf(sanBuf, sizeof(sanBuf), "%s", giaiDangSua_.maSan.c_str());
        ImGui::SetNextItemWidth(180.0f);
        if (ImGui::InputText(u8"Mã sân", sanBuf, sizeof(sanBuf))) {
            giaiDangSua_.maSan = sanBuf;
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (Widgets::NutChinh(u8"Lưu thay đổi", ImVec2(130.0f, 32.0f))) {
            auto kq = GUI::App::Facade::getInstance().suaGiaiDau(giaiDangSua_);
            if (kq.ok) {
                reloadViewModel();
                ImGui::CloseCurrentPopup();
            } else {
            }
        }

        ImGui::SameLine(0.0f, 12.0f);

        if (Widgets::NutPhu(u8"Đóng", ImVec2(90.0f, 32.0f))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void GiaiScreen::renderModalHuyGiai() {
    std::string noiDung = u8"Bạn có chắc chắn muốn hủy bỏ giải đấu " + maGiaiCanHuy_ +
                          u8"Thao tác này sẽ giải phóng toàn bộ khung giờ của sân bóng.";

    Widgets::ModalXacNhan(
        u8"Xác nhận hủy giải đấu",
        noiDung.c_str(),
        [this]() {
            auto kq = GUI::App::Facade::getInstance().huyGiaiDau(maGiaiCanHuy_);
            if (kq.ok) {
                reloadViewModel();
            } else {
            }
        },
        u8"Xác nhận hủy",
        u8"Đóng",
        true // Nút đỏ nguy hiểm
    );
}

}  // namespace GUI::Screens::Owner
