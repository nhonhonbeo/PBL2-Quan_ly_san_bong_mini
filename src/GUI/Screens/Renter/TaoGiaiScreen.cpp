#include "GUI/Screens/Renter/TaoGiaiScreen.h"

#include <ctime>
#include <cstdio>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <imgui.h>

#include "GUI/App/Session.h"
#include "GUI/Theme/Theme.h"
#include "GUI/Widgets/Badge.h"
#include "GUI/Widgets/Button.h"
#include "GUI/Widgets/Card.h"
#include "GUI/Widgets/ComboRadio.h"
#include "GUI/Widgets/DatePicker.h"
#include "GUI/Widgets/InputFields.h"
#include "GUI/Widgets/Toast.h"

namespace GUI::Screens::Renter {

namespace {

int ngayDangSo(const std::string& ngay) {
    if (ngay.size() != 10 || ngay[2] != '/' || ngay[5] != '/') {
        return 0;
    }
    try {
        int day = std::stoi(ngay.substr(0, 2));
        int month = std::stoi(ngay.substr(3, 2));
        int year = std::stoi(ngay.substr(6, 4));
        if (day < 1 || day > 31 || month < 1 || month > 12 || year < 1) {
            return 0;
        }
        return year * 10000 + month * 100 + day;
    } catch (const std::invalid_argument&) {
        return 0;
    } catch (const std::out_of_range&) {
        return 0;
    }
}

std::string ngayHienTai(int soNgayCongThem = 0) {
    std::time_t now = std::time(nullptr);
    std::tm* localTime = std::localtime(&now);
    if (localTime == nullptr) {
        return {};
    }
    std::tm date = *localTime;
    date.tm_mday += soNgayCongThem;
    if (std::mktime(&date) == static_cast<std::time_t>(-1)) {
        return {};
    }
    std::ostringstream result;
    result << std::setfill('0') << std::setw(2) << date.tm_mday << '/'
           << std::setw(2) << (date.tm_mon + 1) << '/'
           << (date.tm_year + 1900);
    return result.str();
}

const char* tenTrangThaiGiai(GUI::App::TrangThaiGiaiDau tt) {
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

void TaoGiaiScreen::onEnter() {
    formTenGiai_.clear();
    formNgayBatDau_ = ngayHienTai();
    formNgayKetThuc_ = ngayHienTai(1);
    formSoDoi_ = 8;
    formLoaiSanIndex_ = 0;
    formGhiChu_.clear();

    tableCols_ = {
        {u8"Mã giải", 90.0f, ImGuiTableColumnFlags_WidthFixed},
        {u8"Tên giải đấu", 220.0f, ImGuiTableColumnFlags_None},
        {u8"Lịch diễn ra", 190.0f, ImGuiTableColumnFlags_WidthFixed},
        {u8"Sân thi đấu", 110.0f, ImGuiTableColumnFlags_WidthFixed},
        {u8"Trạng thái", 130.0f, ImGuiTableColumnFlags_WidthFixed},
        {u8"Phản hồi / Lý do từ chủ sân", 250.0f, ImGuiTableColumnFlags_WidthStretch}
    };

    reloadViewModel();
}

void TaoGiaiScreen::onExit() {
}

void TaoGiaiScreen::reloadViewModel() {
    auto& facade = GUI::App::Facade::getInstance();
    User* curUser = GUI::App::Session::getInstance().getUser();
    std::string sdt = curUser ? curUser->get_phone_number() : "0909876543";

    danhSachGiaiCuaToi_ = facade.getGiaiDauCuaUser(sdt);

    tableRows_.clear();
    for (const auto& g : danhSachGiaiCuaToi_) {
        Widgets::TableRow row;
        row.id = g.maGiai;
        row.oDuLieu = {
            g.maGiai,
            g.tenGiai,
            g.ngayBatDau + " -> " + g.ngayKetThuc,
            u8"Sân " + g.maSan,
            tenTrangThaiGiai(g.trangThai),
            g.ghiChuTuChoi.empty() ? (g.trangThai == GUI::App::TrangThaiGiaiDau::DaDuyet ? u8"Chủ sân đã phê duyệt" : u8"Đang chờ chủ sân xem xét") : g.ghiChuTuChoi
        };
        tableRows_.push_back(row);
    }
}

void TaoGiaiScreen::render() {
    renderFormTaoGiai();
    ImGui::Spacing();
    ImGui::Spacing();
    renderBangTheoDoiTrangThai();
}

void TaoGiaiScreen::renderFormTaoGiai() {
    Widgets::Card("card_renter_request_tournament", u8"Đăng ký tổ chức giải đấu bóng đá phong trào", [this]() {
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
        ImGui::TextWrapped(u8"Gửi đề xuất tổ chức giải đấu hoặc cúp giao hữu tới Ban quản lý cụm sân bóng. Chủ sân sẽ kiểm tra xung đột lịch và phê duyệt yêu cầu của bạn.");
        ImGui::PopStyleColor();
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Columns(2, "cols_renter_create_tour", false);
        ImGui::SetColumnWidth(0, 480.0f);

        // 1. Tên giải đấu
        Widgets::InputText(u8"Tên giải đấu dự kiến (*)", "##r_t_name", formTenGiai_, u8"Ví dụ: Giải Vô Địch Khối Doanh Nghiệp Trẻ");
        ImGui::Spacing();

        // 2. Ngày bắt đầu và kết thúc bằng DatePicker
        Widgets::DatePicker(u8"Ngày bắt đầu giải (*)", "##r_dp_start", formNgayBatDau_);
        ImGui::Spacing();

        Widgets::DatePicker(u8"Ngày kết thúc giải (*)", "##r_dp_end", formNgayKetThuc_);
        int today = ngayDangSo(ngayHienTai());
        int startDate = ngayDangSo(formNgayBatDau_);
        int endDate = ngayDangSo(formNgayKetThuc_);
        bool startDateValid = startDate != 0 && today != 0 && startDate >= today;
        bool endDateValid = endDate != 0 && startDate != 0 && endDate >= startDate;
        if (!startDateValid) {
            ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::RedDanger);
            ImGui::TextUnformatted(u8"Ngày bắt đầu không được ở trong quá khứ.");
            ImGui::PopStyleColor();
        }
        if (!endDateValid) {
            ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::RedDanger);
            ImGui::TextUnformatted(u8"Ngày kết thúc phải cùng ngày hoặc sau ngày bắt đầu.");
            ImGui::PopStyleColor();
        }
        ImGui::Spacing();

        ImGui::NextColumn();

        // 3. Số đội và Loại sân
        ImGui::TextUnformatted(u8"Số lượng đội bóng tham dự (*):");
        ImGui::SetNextItemWidth(180.0f);
        ImGui::InputInt("##r_t_teams", &formSoDoi_, 2, 4);
        if (formSoDoi_ < 4) formSoDoi_ = 4;
        if (formSoDoi_ > 32) formSoDoi_ = 32;
        ImGui::Spacing();

        std::vector<std::string> dsLoaiSan = {u8"Sân 5 người", u8"Sân 7 người", u8"Sân 11 người"};
        Widgets::Combo(u8"Quy mô loại sân mong muốn (*)", "##r_combo_field_type", formLoaiSanIndex_, dsLoaiSan);
        ImGui::Spacing();

        // 4. Ghi chú và điều lệ
        ImGui::TextUnformatted(u8"Ghi chú / Điều lệ dự kiến:");
        char descBuf[1024];
        std::snprintf(descBuf, sizeof(descBuf), "%s", formGhiChu_.c_str());
        ImGui::PushStyleColor(ImGuiCol_FrameBg, GUI::Theme::Colors::WhiteCard);
        ImGui::PushStyleColor(ImGuiCol_Border, GUI::Theme::Colors::InputBorder);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.5f);
        if (ImGui::InputTextMultiline("##r_t_notes", descBuf, sizeof(descBuf), ImVec2(-1.0f, 90.0f))) {
            formGhiChu_ = descBuf;
        }
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(2);

        ImGui::Columns(1);

        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::BeginDisabled(!startDateValid || !endDateValid);
        if (Widgets::NutChinh(u8"Gửi yêu cầu tổ chức giải đấu tới Chủ sân", ImVec2(340.0f, 38.0f))) {
            if (formTenGiai_.empty()) {
                return;
            }

            User* curUser = GUI::App::Session::getInstance().getUser();
            std::string sdt = curUser ? curUser->get_phone_number() : "0909876543";
            std::string ten = curUser ? curUser->get_name() : u8"Người thuê";

            GUI::App::GiaiDauInfo gMoi;
            gMoi.tenGiai = formTenGiai_;
            gMoi.sdtKhach = sdt;
            gMoi.tenKhachDat = ten;
            gMoi.ngayBatDau = formNgayBatDau_;
            gMoi.ngayKetThuc = formNgayKetThuc_;

            int loaiSan = (formLoaiSanIndex_ == 0) ? 5 : ((formLoaiSanIndex_ == 1) ? 7 : 11);
            gMoi.maSan = (loaiSan == 5) ? "S5-01" : ((loaiSan == 7) ? "S7-01" : "S11-01");
            gMoi.khungGio = "18:00 - 21:00";
            gMoi.moTa = u8"Số đội: " + std::to_string(formSoDoi_) + u8" đội. " + formGhiChu_;

            auto kq = GUI::App::Facade::getInstance().guiYeuCauTaoGiai(gMoi);
            if (kq.ok) {
                formTenGiai_.clear();
                formGhiChu_.clear();
                reloadViewModel();
            } else {
            }
        }
        ImGui::EndDisabled();
    });
}

void TaoGiaiScreen::renderBangTheoDoiTrangThai() {
    Widgets::Card("card_renter_tour_status", u8"Danh sách giải đấu bạn đã đăng ký & Trạng thái phê duyệt", [this]() {
        if (tableRows_.empty()) {
            ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
            ImGui::TextUnformatted(u8"Bạn chưa gửi yêu cầu tổ chức giải đấu nào.");
            ImGui::PopStyleColor();
            return;
        }

        Widgets::Table(
            "tbl_renter_tournaments",
            tableCols_,
            tableRows_,
            tableState_,
            [](const Widgets::TableRow& row, int col) {
                if (col == 4) {
                    // Cột trạng thái
                    const std::string& tt = row.oDuLieu[4];
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
                } else if (col == 5) {
                    // Lý do phản hồi từ chủ sân
                    if (row.oDuLieu[4] == u8"Bị từ chối") {
                        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::RedDanger);
                        ImGui::TextWrapped(u8"%s", row.oDuLieu[5].c_str());
                        ImGui::PopStyleColor();
                    } else {
                        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::DarkText);
                        ImGui::TextWrapped(u8"%s", row.oDuLieu[5].c_str());
                        ImGui::PopStyleColor();
                    }
                } else {
                    ImGui::TextUnformatted(row.oDuLieu[col].c_str());
                }
            }
        );
    });
}

}  // namespace GUI::Screens::Renter
