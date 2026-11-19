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
        Widgets::InputTextValidated(
            u8"Tên giải đấu (*)",
            "##form_t_name",
            formTenGiai_,
            u8"Ví dụ: Cúp Tứ Hùng Doanh Nghiệp IT 2026",
            formTenGiaiDaCham_,
            daGuiYeuCau_,
            Widgets::FormValidator::kiemTraTenGiaiDau,
            Widgets::FormValidator::chuanHoaTenGiaiDau
        );
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
            Widgets::InputText(u8"Họ và tên khách (*)", "##form_t_cust_name", formTenKhachMoi_, nullptr, u8"Nguyễn Văn A");
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
        const std::string startIso = DateTime::toIsoDate(formNgayBatDau_);
        const std::string endIso = DateTime::toIsoDate(formNgayKetThuc_);
        const bool startDateValid = !startIso.empty() &&
                                    !DateTime::isPastDate(startIso);
        const bool endDateValid = !endIso.empty() && !startIso.empty() &&
                                  DateTime::compareDate(endIso, startIso) >= 0;
        if (!startDateValid) {
            ImGui::TextColored(
                GUI::Theme::Colors::RedDanger,
                u8"Ngày bắt đầu không được ở trong quá khứ."
            );
        }
        if (!endDateValid) {
            ImGui::TextColored(
                GUI::Theme::Colors::RedDanger,
                u8"Ngày kết thúc phải bằng hoặc sau ngày bắt đầu."
            );
        }
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
        ImGui::BeginDisabled(!startDateValid || !endDateValid);
        if (Widgets::NutChinh(u8"+ Xác nhận tạo giải đấu", ImVec2(-1.0f, 40.0f))) {
            daGuiYeuCau_ = true;
            // Kiểm tra tính hợp lệ của Form
            if (!Widgets::FormValidator::kiemTraTenGiaiDau(formTenGiai_).empty()) {
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
                formTenGiaiDaCham_ = false;
                daGuiYeuCau_ = false;
                formMoTa_.clear();
                reloadViewModel();
            } else {
                // Hiển thị Toast đỏ nội dung lỗi do nghiệp vụ trả về
            }
        }
        ImGui::EndDisabled();

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

}  // namespace GUI::Screens::Owner
