#include "GUI/Screens/Owner/KhachHangScreen.h"

#include <algorithm>
#include <cstdio>
#include <iomanip>
#include <sstream>
#include <imgui.h>

#include "GUI/Theme/Theme.h"
#include "GUI/Widgets/Badge.h"
#include "GUI/Widgets/Button.h"
#include "GUI/Widgets/Card.h"
#include "GUI/Widgets/ComboRadio.h"
#include "GUI/Widgets/InputFields.h"
#include "GUI/Widgets/Modal.h"
#include "GUI/Widgets/ProgressBar.h"
#include "GUI/Widgets/StarRating.h"
#include "GUI/Widgets/Toast.h"

namespace GUI::Screens::Owner {

namespace {

// Hàm che một phần số điện thoại để hiển thị công khai (ví dụ: 090***5678)
std::string maHoaSDT(const std::string& sdt) {
    if (sdt.length() >= 7) {
        return sdt.substr(0, 3) + "***" + sdt.substr(sdt.length() - 4);
    }
    return sdt;
}

}  // namespace

void KhachHangScreen::onEnter() {
    // Cấu hình bảng danh bạ khách hàng
    tableColsDanhBa_ = {
        {u8"STT", 60.0f, ImGuiTableColumnFlags_WidthFixed},
        {u8"Họ và tên", 180.0f, ImGuiTableColumnFlags_None},
        {u8"Số điện thoại", 130.0f, ImGuiTableColumnFlags_WidthFixed},
        {u8"Số lần đặt", 110.0f, ImGuiTableColumnFlags_WidthFixed},
        {u8"Số lần hủy", 110.0f, ImGuiTableColumnFlags_WidthFixed},
        {u8"Trạng thái", 140.0f, ImGuiTableColumnFlags_WidthFixed}
    };

    reloadViewModel();
}

void KhachHangScreen::onExit() {
}

void KhachHangScreen::reloadViewModel() {
    auto& facade = GUI::App::Facade::getInstance();

    // 1. Nạp danh bạ khách hàng
    danhSachKhach_ = facade.getDanhSachKhachHang();
    tableRowsDanhBa_.clear();

    for (size_t i = 0; i < danhSachKhach_.size(); ++i) {
        const auto& kh = danhSachKhach_[i];
        Widgets::TableRow row;
        row.id = kh.sdt; // Dùng SĐT làm định danh duy nhất

        std::string trangThaiStr = kh.biChan ? u8"Bị chặn" : (kh.mucUuDaiPhanTram > 0 ? (u8"VIP (" + std::to_string(kh.mucUuDaiPhanTram) + "%)") : u8"Bình thường");

        row.oDuLieu = {
            std::to_string(i + 1),
            kh.hoTen,
            kh.sdt,
            std::to_string(kh.soLanDat),
            std::to_string(kh.soLanHuy),
            trangThaiStr
        };
        tableRowsDanhBa_.push_back(row);
    }

    // 2. Nạp chi tiết khách hàng đang chọn (nếu có)
    // LÝ DO CHỌN: HashTable tra cứu theo SĐT với thời gian O(1) trung bình,
    // nạp ngay thông tin khách hàng vào ViewModel mà không cần duyệt tuyến tính toàn bộ danh bạ.
    if (!sdtDangChon_.empty()) {
        auto khPtr = facade.timKhachHangTheoSDT(sdtDangChon_);
        if (khPtr) {
            khachDangChon_ = *khPtr;
            lichSuDatCuaKhach_ = facade.getLichSuDatCuaKhach(sdtDangChon_);
            danhGiaCuaKhach_ = facade.getDanhGiaCuaKhach(sdtDangChon_);
        } else {
            sdtDangChon_.clear();
        }
    }

    // 3. Nạp danh sách ngưỡng ưu đãi
    danhSachNguongUuDai_ = facade.getNguongUuDai();

    // 4. Nạp danh sách đen & đề xuất chặn
    // LÝ DO CHỌN: LinkedList quản lý danh sách đen và đề xuất chặn,
    // hỗ trợ chèn / xóa các khách hàng vi phạm ở bất kỳ vị trí nào với O(1).
    danhSachDen_ = facade.getDanhSachDen();
    danhSachDeXuatChan_ = facade.getDeXuatChan(nguongHuyDeXuat_);

    // 5. Nạp danh sách đánh giá nhận xét
    // LÝ DO CHỌN: LinkedList quản lý danh sách đánh giá nhận xét,
    // các đánh giá mới nhất được chèn vào đầu danh sách với độ phức tạp O(1).
    danhSachDanhGia_ = facade.getDanhSachDanhGia();
    diemTBToanCoSo_ = facade.getDiemTrungBinhToanCoSo();
    diemTBTungSan_ = facade.getDiemTrungBinhTungSan();
}

void KhachHangScreen::render() {
    if (ImGui::BeginTabBar("##KhachHangTabBar", ImGuiTabBarFlags_None)) {
        if (ImGui::BeginTabItem(u8"1. Danh bạ khách hàng")) {
            renderTabDanhBa();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem(u8"2. Ngưỡng ưu đãi & Khách VIP")) {
            renderTabUuDai();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem(u8"3. Danh sách đen & Đề xuất")) {
            renderTabDanhSachDen();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem(u8"4. Đánh giá nhận xét")) {
            renderTabDanhGia();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    // Render các Modal xác nhận thao tác
    renderModalXacNhanChan();
    renderModalXacNhanGoChan();
    renderModalApDungUuDai();
}

void KhachHangScreen::renderTabDanhBa() {
    bool coKhachDangChon = !sdtDangChon_.empty();
    float availW = ImGui::GetContentRegionAvail().x;
    float panelW = 380.0f;
    float tableW = coKhachDangChon ? (availW - panelW - 16.0f) : availW;

    // Cột trái: Bảng danh bạ khách hàng
    ImGui::BeginGroup();
    ImGui::PushItemWidth(tableW);

    Widgets::Card("card_cust_contact_list", u8"Danh bạ khách hàng cơ sở sân", [this]() {
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
        ImGui::TextWrapped(u8"Tìm kiếm theo SĐT hoặc Tên khách (không phân biệt hoa thường). Bấm vào dòng bất kỳ để mở panel xem thông tin chi tiết và lịch sử bên phải.");
        ImGui::PopStyleColor();
        ImGui::Spacing();

        // Bảng danh bạ
        Widgets::Table(
            "tbl_cust_contacts",
            tableColsDanhBa_,
            tableRowsDanhBa_,
            tableStateDanhBa_,
            [this](const Widgets::TableRow& row, int col) {
                // LÝ DO CHỌN: Khách bị chặn tô nền đỏ nhạt ở MỌI bảng khách hàng của ứng dụng để cảnh báo thị giác
                bool isBlocked = (row.oDuLieu[5] == u8"Bị chặn");

                if (isBlocked) {
                    ImDrawList* dl = ImGui::GetWindowDrawList();
                    ImVec2 cellMin = ImGui::GetCursorScreenPos();
                    float colWidth = ImGui::GetContentRegionAvail().x;
                    ImVec2 cellMax(cellMin.x + colWidth + 8.0f, cellMin.y + ImGui::GetTextLineHeightWithSpacing() + 6.0f);
                    dl->AddRectFilled(cellMin, cellMax, IM_COL32(254, 226, 226, 160)); // Đỏ nhạt
                }

                if (col == 5) {
                    // Cột trạng thái
                    if (isBlocked) {
                        Widgets::BadgeDanhSachDen(u8"Bị chặn");
                    } else if (row.oDuLieu[5].find("VIP") != std::string::npos) {
                        Widgets::BadgeVIP(row.oDuLieu[5].c_str());
                    } else {
                        Widgets::Badge(u8"Bình thường", GUI::Theme::Colors::GrayHover, GUI::Theme::Colors::DarkText);
                    }
                } else {
                    ImGui::TextUnformatted(row.oDuLieu[col].c_str());
                }
            },
            [this](const Widgets::TableRow& clickedRow) {
                // Click dòng -> chọn khách hàng và nạp chi tiết
                sdtDangChon_ = clickedRow.id;
                reloadViewModel();
                Widgets::ToastThongTin(u8"Đã chọn khách hàng: " + clickedRow.oDuLieu[1]);
            }
        );
    });

    ImGui::PopItemWidth();
    ImGui::EndGroup();

    // Cột phải: Panel chi tiết khách hàng
    if (coKhachDangChon) {
        ImGui::SameLine(0.0f, 16.0f);
        ImGui::BeginGroup();
        ImGui::PushItemWidth(panelW);

        renderPanelChiTietKhachHang();

        ImGui::PopItemWidth();
        ImGui::EndGroup();
    }
}

void KhachHangScreen::renderPanelChiTietKhachHang() {
    Widgets::Card("card_cust_detail_panel", u8"Chi tiết hồ sơ khách hàng", [this]() {
        // Nút đóng panel
        float availW = ImGui::GetContentRegionAvail().x;
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + availW - 32.0f);
        if (Widgets::NutPhu("X##close_panel", ImVec2(28.0f, 26.0f))) {
            sdtDangChon_.clear();
            return;
        }

        ImGui::Spacing();

        // Tên và SĐT khách
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::DarkText);
        ImGui::TextUnformatted(khachDangChon_.hoTen.c_str());
        ImGui::PopStyleColor();

        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::GrassGreen);
        ImGui::Text(u8"📞 %s", khachDangChon_.sdt.c_str());
        ImGui::PopStyleColor();
        ImGui::Spacing();

        // Badge trạng thái
        if (khachDangChon_.biChan) {
            Widgets::BadgeDanhSachDen(u8"Đang bị chặn (Blacklist)");
            ImGui::Spacing();
            ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::RedDanger);
            ImGui::TextWrapped(u8"Lý do: %s", khachDangChon_.lyDoChan.c_str());
            ImGui::Text(u8"Ngày chặn: %s", khachDangChon_.ngayChan.c_str());
            ImGui::PopStyleColor();
        } else if (khachDangChon_.mucUuDaiPhanTram > 0) {
            Widgets::BadgeVIP((u8"Khách hàng VIP (Giảm " + std::to_string(khachDangChon_.mucUuDaiPhanTram) + "%)").c_str());
        } else {
            Widgets::Badge(u8"Khách hàng tiêu chuẩn", GUI::Theme::Colors::GrayHover, GUI::Theme::Colors::DarkText);
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Thống kê số lần đặt / hủy
        ImGui::Text(u8"• Số lần đã đặt sân: %d lần", khachDangChon_.soLanDat);
        ImGui::Text(u8"• Số lần đã hủy sân: %d lần", khachDangChon_.soLanHuy);

        float tyLeHuy = 0.0f;
        int tongLuot = khachDangChon_.soLanDat + khachDangChon_.soLanHuy;
        if (tongLuot > 0) {
            tyLeHuy = (static_cast<float>(khachDangChon_.soLanHuy) / tongLuot) * 100.0f;
        }
        ImGui::Text(u8"• Tỷ lệ hủy đặt: %.1f%%", tyLeHuy);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Lịch sử đặt sân gần đây của khách
        ImGui::TextUnformatted(u8"Lịch sử đặt sân gần nhất:");
        ImGui::Spacing();

        if (lichSuDatCuaKhach_.empty()) {
            ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
            ImGui::TextUnformatted(u8"Chưa có đơn đặt sân nào gần đây.");
            ImGui::PopStyleColor();
        } else {
            for (const auto& don : lichSuDatCuaKhach_) {
                ImGui::BulletText(u8"%s | Sân %s | %s", don.ngay.c_str(), don.maSan.c_str(), don.khungGio.c_str());
            }
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Đánh giá đã gửi
        ImGui::TextUnformatted(u8"Đánh giá & Nhận xét đã gửi:");
        ImGui::Spacing();

        if (danhGiaCuaKhach_.empty()) {
            ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
            ImGui::TextUnformatted(u8"Khách hàng chưa gửi đánh giá nào.");
            ImGui::PopStyleColor();
        } else {
            for (const auto& dg : danhGiaCuaKhach_) {
                Widgets::StarRating(nullptr, ("panel_star_" + dg.id).c_str(), const_cast<int&>(dg.soSao), true, 16.0f);
                ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::DarkText);
                ImGui::TextWrapped(u8"\"%s\" (Sân %s)", dg.noiDung.c_str(), dg.maSan.c_str());
                ImGui::PopStyleColor();
                ImGui::Spacing();
            }
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Thao tác chặn / gỡ chặn nhanh
        if (khachDangChon_.biChan) {
            if (Widgets::NutChinh(u8"Gỡ khỏi danh sách đen", ImVec2(-1.0f, 34.0f))) {
                sdtCanGoChan_ = khachDangChon_.sdt;
                Widgets::MoModalXacNhan(u8"Xác nhận gỡ khỏi danh sách đen");
            }
        } else {
            if (Widgets::NutNguyHiem(u8"Thêm vào danh sách đen (Chặn)", ImVec2(-1.0f, 34.0f))) {
                sdtCanChan_ = khachDangChon_.sdt;
                lyDoCanChan_ = u8"Hủy sân nhiều lần hoặc gây mất trật tự";
                Widgets::MoModalXacNhan(u8"Xác nhận đưa vào danh sách đen");
            }
        }
    });
}

void KhachHangScreen::renderTabUuDai() {
    // Khối 1: Bảng cấu hình ngưỡng ưu đãi
    Widgets::Card("card_tab_tier_config", u8"Cấu hình các ngưỡng ưu đãi thành viên", [this]() {
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
        ImGui::TextWrapped(u8"Chỉnh sửa trực tiếp số lần đặt tối thiểu và mức phần trăm giảm giá cho từng hạng thành viên. Nhấn 'Lưu thay đổi' để cập nhật.");
        ImGui::PopStyleColor();
        ImGui::Spacing();

        if (ImGui::BeginTable("##tbl_nguong_uu_dai", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn(u8"Hạng thành viên", ImGuiTableColumnFlags_WidthFixed, 180.0f);
            ImGui::TableSetupColumn(u8"Số lần đặt tối thiểu", ImGuiTableColumnFlags_WidthFixed, 180.0f);
            ImGui::TableSetupColumn(u8"Mức giảm giá (%)", ImGuiTableColumnFlags_WidthFixed, 180.0f);
            ImGui::TableSetupColumn(u8"Thao tác", ImGuiTableColumnFlags_WidthFixed, 140.0f);
            ImGui::TableHeadersRow();

            for (auto& n : danhSachNguongUuDai_) {
                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                if (n.phanTramGiam >= 10) {
                    Widgets::BadgeVIP(n.tenHang.c_str());
                } else {
                    Widgets::Badge(n.tenHang.c_str(), GUI::Theme::Colors::GrayHover, GUI::Theme::Colors::DarkText);
                }

                ImGui::TableSetColumnIndex(1);
                std::string inputSlId = "##min_book_" + std::to_string(n.id);
                ImGui::SetNextItemWidth(120.0f);
                ImGui::InputInt(inputSlId.c_str(), &n.soLanToiThieu, 1, 5);

                ImGui::TableSetColumnIndex(2);
                std::string inputPctId = "##pct_" + std::to_string(n.id);
                ImGui::SetNextItemWidth(120.0f);
                ImGui::InputInt(inputPctId.c_str(), &n.phanTramGiam, 1, 5);

                ImGui::TableSetColumnIndex(3);
                std::string btnSaveId = u8"Lưu##tier_" + std::to_string(n.id);
                if (Widgets::NutPhu(btnSaveId.c_str(), ImVec2(80.0f, 26.0f))) {
                    auto kq = GUI::App::Facade::getInstance().capNhatNguongUuDai(n.id, n.soLanToiThieu, n.phanTramGiam);
                    if (kq.ok) {
                        Widgets::ToastThanhCong(u8"Đã cập nhật ngưỡng " + n.tenHang);
                        reloadViewModel();
                    } else {
                        Widgets::ToastLoi(kq.loi);
                    }
                }
            }

            ImGui::EndTable();
        }

        ImGui::Spacing();
        ImGui::Spacing();

        if (Widgets::NutChinh(u8"⭐ Áp dụng ưu đãi cho toàn bộ khách hàng", ImVec2(340.0f, 38.0f))) {
            Widgets::MoModalXacNhan(u8"Xác nhận áp dụng ưu đãi");
        }
    });

    ImGui::Spacing();

    // Khối 2: Danh sách khách hàng đủ điều kiện nhận ưu đãi
    Widgets::Card("card_tab_qualified_vip", u8"Danh sách khách hàng đạt chuẩn nhận ưu đãi", [this]() {
        if (ImGui::BeginTable("##tbl_qualified_vip", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp)) {
            ImGui::TableSetupColumn(u8"STT", ImGuiTableColumnFlags_WidthFixed, 60.0f);
            ImGui::TableSetupColumn(u8"Họ và tên", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn(u8"Số điện thoại", ImGuiTableColumnFlags_WidthFixed, 130.0f);
            ImGui::TableSetupColumn(u8"Số lần đã đặt", ImGuiTableColumnFlags_WidthFixed, 120.0f);
            ImGui::TableSetupColumn(u8"Hạng đạt chuẩn", ImGuiTableColumnFlags_WidthFixed, 150.0f);
            ImGui::TableSetupColumn(u8"Ưu đãi hiện tại", ImGuiTableColumnFlags_WidthFixed, 130.0f);
            ImGui::TableHeadersRow();

            int stt = 1;
            for (const auto& kh : danhSachKhach_) {
                if (kh.soLanDat >= 5) {
                    ImGui::TableNextRow();

                    // Tô nền đỏ nhạt nếu đang bị chặn
                    if (kh.biChan) {
                        ImDrawList* dl = ImGui::GetWindowDrawList();
                        ImVec2 cellMin = ImGui::GetCursorScreenPos();
                        float colWidth = ImGui::GetContentRegionAvail().x;
                        ImVec2 cellMax(cellMin.x + colWidth + 8.0f, cellMin.y + ImGui::GetTextLineHeightWithSpacing() + 6.0f);
                        dl->AddRectFilled(cellMin, cellMax, IM_COL32(254, 226, 226, 160));
                    }

                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%d", stt++);

                    ImGui::TableSetColumnIndex(1);
                    ImGui::TextUnformatted(kh.hoTen.c_str());

                    ImGui::TableSetColumnIndex(2);
                    ImGui::TextUnformatted(kh.sdt.c_str());

                    ImGui::TableSetColumnIndex(3);
                    ImGui::Text(u8"%d lần", kh.soLanDat);

                    ImGui::TableSetColumnIndex(4);
                    if (kh.soLanDat >= 20) {
                        Widgets::BadgeVIP(u8"VIP Vàng (15%)");
                    } else if (kh.soLanDat >= 10) {
                        Widgets::BadgeVIP(u8"VIP Bạc (10%)");
                    } else {
                        Widgets::Badge(u8"Thân thiết (5%)", GUI::Theme::Colors::GrayHover, GUI::Theme::Colors::DarkText);
                    }

                    ImGui::TableSetColumnIndex(5);
                    if (kh.biChan) {
                        Widgets::BadgeDanhSachDen(u8"Tước quyền (Bị chặn)");
                    } else {
                        ImGui::Text(u8"Giảm %d%%", kh.mucUuDaiPhanTram);
                    }
                }
            }

            ImGui::EndTable();
        }
    });
}

void KhachHangScreen::renderTabDanhSachDen() {
    // Khối 1: Bảng khách hàng đang bị chặn (Blacklist)
    Widgets::Card("card_tab_blacklist", u8"Danh sách khách hàng đang bị chặn (Blacklist)", [this]() {
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
        ImGui::TextWrapped(u8"Khách hàng thuộc danh sách đen sẽ bị khóa quyền đặt giữ chỗ trực tuyến và tước toàn bộ quyền lợi ưu đãi.");
        ImGui::PopStyleColor();
        ImGui::Spacing();

        if (danhSachDen_.empty()) {
            ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::GrassGreen);
            ImGui::TextUnformatted(u8"Hiện tại không có khách hàng nào trong danh sách đen.");
            ImGui::PopStyleColor();
        } else {
            if (ImGui::BeginTable("##tbl_blacklist", 7, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp)) {
                ImGui::TableSetupColumn(u8"Mã KH", ImGuiTableColumnFlags_WidthFixed, 80.0f);
                ImGui::TableSetupColumn(u8"Họ và tên", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn(u8"Số điện thoại", ImGuiTableColumnFlags_WidthFixed, 120.0f);
                ImGui::TableSetupColumn(u8"Số lần hủy", ImGuiTableColumnFlags_WidthFixed, 100.0f);
                ImGui::TableSetupColumn(u8"Ngày chặn", ImGuiTableColumnFlags_WidthFixed, 110.0f);
                ImGui::TableSetupColumn(u8"Lý do", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn(u8"Hành động", ImGuiTableColumnFlags_WidthFixed, 110.0f);
                ImGui::TableHeadersRow();

                for (const auto& kh : danhSachDen_) {
                    ImGui::TableNextRow();

                    // Tô nền đỏ nhạt
                    ImDrawList* dl = ImGui::GetWindowDrawList();
                    ImVec2 cellMin = ImGui::GetCursorScreenPos();
                    float colWidth = ImGui::GetContentRegionAvail().x;
                    ImVec2 cellMax(cellMin.x + colWidth + 8.0f, cellMin.y + ImGui::GetTextLineHeightWithSpacing() + 6.0f);
                    dl->AddRectFilled(cellMin, cellMax, IM_COL32(254, 226, 226, 160));

                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted(kh.id.c_str());

                    ImGui::TableSetColumnIndex(1);
                    ImGui::TextUnformatted(kh.hoTen.c_str());

                    ImGui::TableSetColumnIndex(2);
                    ImGui::TextUnformatted(kh.sdt.c_str());

                    ImGui::TableSetColumnIndex(3);
                    ImGui::Text("%d", kh.soLanHuy);

                    ImGui::TableSetColumnIndex(4);
                    ImGui::TextUnformatted(kh.ngayChan.c_str());

                    ImGui::TableSetColumnIndex(5);
                    ImGui::TextUnformatted(kh.lyDoChan.c_str());

                    ImGui::TableSetColumnIndex(6);
                    std::string btnUnblockId = u8"Gỡ chặn##" + kh.sdt;
                    if (Widgets::NutPhu(btnUnblockId.c_str(), ImVec2(90.0f, 26.0f))) {
                        sdtCanGoChan_ = kh.sdt;
                        Widgets::MoModalXacNhan(u8"Xác nhận gỡ khỏi danh sách đen");
                    }
                }

                ImGui::EndTable();
            }
        }
    });

    ImGui::Spacing();

    // Khối 2: Khung "Đề xuất khách hàng hủy quá N lần"
    Widgets::Card("card_tab_blacklist_suggest", u8"Đề xuất đưa vào danh sách đen (Khách hủy sân nhiều lần)", [this]() {
        ImGui::TextUnformatted(u8"Ngưỡng số lần hủy để đưa vào đề xuất chặn:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(80.0f);
        if (ImGui::InputInt("##input_nguong_huy", &nguongHuyDeXuat_, 1, 1)) {
            if (nguongHuyDeXuat_ < 1) nguongHuyDeXuat_ = 1;
            reloadViewModel();
        }

        ImGui::Spacing();
        ImGui::Spacing();

        if (danhSachDeXuatChan_.empty()) {
            ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
            ImGui::Text(u8"Không có khách hàng nào hủy từ %d lần trở lên.", nguongHuyDeXuat_);
            ImGui::PopStyleColor();
        } else {
            if (ImGui::BeginTable("##tbl_de_xuat_chan", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp)) {
                ImGui::TableSetupColumn(u8"Mã KH", ImGuiTableColumnFlags_WidthFixed, 80.0f);
                ImGui::TableSetupColumn(u8"Họ và tên", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn(u8"Số điện thoại", ImGuiTableColumnFlags_WidthFixed, 130.0f);
                ImGui::TableSetupColumn(u8"Số lần đã đặt", ImGuiTableColumnFlags_WidthFixed, 110.0f);
                ImGui::TableSetupColumn(u8"Số lần đã hủy", ImGuiTableColumnFlags_WidthFixed, 110.0f);
                ImGui::TableSetupColumn(u8"Hành động", ImGuiTableColumnFlags_WidthFixed, 150.0f);
                ImGui::TableHeadersRow();

                for (const auto& kh : danhSachDeXuatChan_) {
                    ImGui::TableNextRow();

                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted(kh.id.c_str());

                    ImGui::TableSetColumnIndex(1);
                    ImGui::TextUnformatted(kh.hoTen.c_str());

                    ImGui::TableSetColumnIndex(2);
                    ImGui::TextUnformatted(kh.sdt.c_str());

                    ImGui::TableSetColumnIndex(3);
                    ImGui::Text("%d", kh.soLanDat);

                    ImGui::TableSetColumnIndex(4);
                    ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::RedDanger);
                    ImGui::Text(u8"%d lần", kh.soLanHuy);
                    ImGui::PopStyleColor();

                    ImGui::TableSetColumnIndex(5);
                    std::string btnBlockId = u8"Chặn khách##" + kh.sdt;
                    if (Widgets::NutNguyHiem(btnBlockId.c_str(), ImVec2(130.0f, 26.0f))) {
                        sdtCanChan_ = kh.sdt;
                        lyDoCanChan_ = u8"Hủy sân quá " + std::to_string(kh.soLanHuy) + u8" lần quy định";
                        Widgets::MoModalXacNhan(u8"Xác nhận đưa vào danh sách đen");
                    }
                }

                ImGui::EndTable();
            }
        }
    });
}

void KhachHangScreen::renderTabDanhGia() {
    // Khối 1: Thống kê điểm trung bình toàn cơ sở & từng sân
    Widgets::Card("card_tab_review_stats", u8"Thống kê điểm đánh giá chất lượng dịch vụ", [this]() {
        float availW = ImGui::GetContentRegionAvail().x;
        float leftW = 260.0f;
        float rightW = availW - leftW - 20.0f;

        // Cột trái: Điểm trung bình toàn cơ sở
        ImGui::BeginGroup();
        ImGui::PushItemWidth(leftW);

        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
        ImGui::TextUnformatted(u8"Điểm trung bình toàn cụm sân:");
        ImGui::PopStyleColor();

        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::GoldAccent);
        char scoreBuf[32];
        std::snprintf(scoreBuf, sizeof(scoreBuf), "%.1f / 5.0", diemTBToanCoSo_);
        ImGui::TextUnformatted(scoreBuf);
        ImGui::PopStyleColor();

        int starScore = static_cast<int>(diemTBToanCoSo_ + 0.5f);
        Widgets::StarRating(nullptr, "overall_stars", starScore, true, 22.0f);

        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
        ImGui::Text(u8"Tổng số: %zu đánh giá", danhSachDanhGia_.size());
        ImGui::PopStyleColor();

        ImGui::PopItemWidth();
        ImGui::EndGroup();

        ImGui::SameLine(0.0f, 20.0f);

        // Cột phải: Điểm trung bình theo từng sân bằng thanh ngang ProgressBar có số
        ImGui::BeginGroup();
        ImGui::PushItemWidth(rightW);

        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::DarkText);
        ImGui::TextUnformatted(u8"Điểm đánh giá trung bình theo từng sân bóng:");
        ImGui::PopStyleColor();
        ImGui::Spacing();

        for (const auto& item : diemTBTungSan_) {
            ImGui::Text(u8"Sân %s:", item.first.c_str());
            ImGui::SameLine(90.0f);

            float ratio = item.second / 5.0f;
            char barLabel[32];
            std::snprintf(barLabel, sizeof(barLabel), "%.1f / 5.0", item.second);

            Widgets::ProgressBar(ratio, barLabel, ImVec2(rightW - 100.0f, 18.0f), GUI::Theme::Colors::GoldAccent);
            ImGui::Spacing();
        }

        ImGui::PopItemWidth();
        ImGui::EndGroup();
    });

    ImGui::Spacing();

    // Khối 2: Bộ lọc theo số sao & Danh sách đánh giá nhận xét
    Widgets::Card("card_tab_review_list", u8"Danh sách đánh giá & Nhận xét của khách hàng", [this]() {
        // Bộ lọc số sao
        ImGui::TextUnformatted(u8"Lọc theo số sao:");
        ImGui::SameLine();
        const char* filterLabels[] = {
            u8"Tất cả số sao",
            u8"⭐ 5 sao",
            u8"⭐ 4 sao",
            u8"⭐ 3 sao",
            u8"⭐ 2 sao",
            u8"⭐ 1 sao"
        };
        ImGui::SetNextItemWidth(180.0f);
        if (ImGui::BeginCombo("##combo_filter_stars", filterLabels[locSoSao_])) {
            for (int i = 0; i < 6; ++i) {
                bool isSelected = (locSoSao_ == i);
                if (ImGui::Selectable(filterLabels[i], isSelected)) {
                    locSoSao_ = i;
                }
            }
            ImGui::EndCombo();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Danh sách đánh giá
        bool coDanhGia = false;
        for (const auto& dg : danhSachDanhGia_) {
            // Lọc số sao: nếu locSoSao_ > 0 thì so sánh (6 - locSoSao_)
            if (locSoSao_ > 0) {
                int expectedStar = (locSoSao_ == 1) ? 5 : ((locSoSao_ == 2) ? 4 : ((locSoSao_ == 3) ? 3 : ((locSoSao_ == 4) ? 2 : 1)));
                if (dg.soSao != expectedStar) {
                    continue;
                }
            }

            coDanhGia = true;

            // Khối thẻ nhận xét
            ImGui::BeginGroup();

            ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::DarkText);
            ImGui::Text("%s (%s)", dg.tenKhach.c_str(), maHoaSDT(dg.sdt).c_str());
            ImGui::PopStyleColor();

            ImGui::SameLine();
            Widgets::BadgeTrong((u8"Sân " + dg.maSan).c_str());

            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
            ImGui::Text(u8"| Ngày: %s", dg.ngayGui.c_str());
            ImGui::PopStyleColor();

            // Vẽ số sao
            Widgets::StarRating(nullptr, ("dg_star_" + dg.id).c_str(), const_cast<int&>(dg.soSao), true, 16.0f);

            // Nội dung nhận xét
            ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::DarkText);
            ImGui::TextWrapped(u8"\"%s\"", dg.noiDung.c_str());
            ImGui::PopStyleColor();

            ImGui::EndGroup();

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
        }

        if (!coDanhGia) {
            ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
            ImGui::TextUnformatted(u8"Không có đánh giá nào phù hợp với bộ lọc đã chọn.");
            ImGui::PopStyleColor();
        }
    });
}

void KhachHangScreen::renderModalXacNhanChan() {
    std::string noiDung = u8"Bạn có chắc chắn muốn đưa khách hàng có SĐT " + sdtCanChan_ +
                          u8" vào danh sách đen không?\nKhách hàng này sẽ bị khóa tài khoản đặt sân.";

    Widgets::ModalXacNhan(
        u8"Xác nhận đưa vào danh sách đen",
        noiDung.c_str(),
        [this]() {
            // LÝ DO CHỌN: LinkedList quản lý danh sách đen, chèn khách hàng bị chặn với O(1)
            auto kq = GUI::App::Facade::getInstance().themVaoDanhSachDen(sdtCanChan_, lyDoCanChan_);
            if (kq.ok) {
                Widgets::ToastThanhCong(u8"Đã đưa SĐT " + sdtCanChan_ + u8" vào danh sách đen!");
                reloadViewModel();
            } else {
                Widgets::ToastLoi(kq.loi);
            }
        },
        u8"Xác nhận chặn",
        u8"Hủy bỏ",
        true // Nút đỏ nguy hiểm
    );
}

void KhachHangScreen::renderModalXacNhanGoChan() {
    std::string noiDung = u8"Bạn có chắc chắn muốn gỡ bỏ khách hàng có SĐT " + sdtCanGoChan_ +
                          u8" khỏi danh sách đen không?\nKhách hàng sẽ được khôi phục quyền đặt sân.";

    Widgets::ModalXacNhan(
        u8"Xác nhận gỡ khỏi danh sách đen",
        noiDung.c_str(),
        [this]() {
            // LÝ DO CHỌN: LinkedList gỡ bỏ phần tử khách hàng khỏi danh sách đen với O(1)
            auto kq = GUI::App::Facade::getInstance().goKhoiDanhSachDen(sdtCanGoChan_);
            if (kq.ok) {
                Widgets::ToastThanhCong(u8"Đã gỡ SĐT " + sdtCanGoChan_ + u8" khỏi danh sách đen!");
                reloadViewModel();
            } else {
                Widgets::ToastLoi(kq.loi);
            }
        },
        u8"Gỡ chặn",
        u8"Hủy bỏ",
        false
    );
}

void KhachHangScreen::renderModalApDungUuDai() {
    Widgets::ModalXacNhan(
        u8"Xác nhận áp dụng ưu đãi",
        u8"Hệ thống sẽ tự động quét toàn bộ lịch sử đặt sân của tất cả khách hàng và áp dụng mức giảm giá VIP tương ứng.\nBạn có muốn tiếp tục không?",
        [this]() {
            auto kq = GUI::App::Facade::getInstance().apDungUuDaiChoKhach();
            if (kq.ok) {
                Widgets::ToastThanhCong(u8"Đã cập nhật mức ưu đãi VIP cho toàn bộ khách hàng!");
                reloadViewModel();
            } else {
                Widgets::ToastLoi(kq.loi);
            }
        },
        u8"Đồng ý áp dụng",
        u8"Hủy bỏ",
        false
    );
}

}  // namespace GUI::Screens::Owner
