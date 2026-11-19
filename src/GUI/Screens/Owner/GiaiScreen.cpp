#include "GUI/Screens/Owner/GiaiScreen.h"

#include <algorithm>
#include <cstdio>
#include <string>
#include <vector>
#include <imgui.h>

#include "Core/DateTime.h"
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
#include "GUI/App/ViewFormat.h"
#include "GiaiScreenHelpers.h"

namespace GUI::Screens::Owner {

using namespace GUI::App::ViewFormat;
using namespace detail;

void GiaiScreen::onEnter() {
    formTenGiaiDaCham_ = false;
    daGuiYeuCau_ = false;
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
    formNgayBatDau_ = ngayHienThi(DateTime::today());
    formNgayKetThuc_ = formNgayBatDau_;

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
    danhSachKhachHang_ = facade.getDanhSachKhachHangCuaChuSan();
    danhSachSan_ = facade.getDanhSachSanBongCuaChuSan();
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
    // LÝ DO CHỌN: Tab Bar chuẩn Modern SaaS hiển thị TextPrimary tương phản cao trên nền Surface
    if (ImGui::BeginTabBar("##GiaiTabBar", ImGuiTabBarFlags_None)) {
        if (ImGui::BeginTabItem(u8"Danh sách giải đấu")) {
            renderTabDanhSachGiai();
            ImGui::EndTabItem();
        }

        // Tab có hiển thị số lượng yêu cầu chờ duyệt
        std::string tabDuyetTitle = u8"Yêu cầu chờ duyệt (" + std::to_string(danhSachChoDuyet_.size()) + u8")";
        if (ImGui::BeginTabItem(tabDuyetTitle.c_str())) {
            renderTabYeuCauChoDuyet();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem(u8"Thêm giải thủ công")) {
            renderTabThemGiaiThuCong();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem(u8"Lịch sự kiện giải đấu")) {
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
                        Widgets::BadgeBaoTri(tt.c_str());
                    } else if (tt == u8"Đã duyệt") {
                        Widgets::BadgeTrong(tt.c_str());
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

}  // namespace GUI::Screens::Owner
