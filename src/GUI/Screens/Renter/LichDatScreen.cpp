#include "GUI/Screens/Renter/LichDatScreen.h"

#include <chrono>
#include <ctime>
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
#include "GUI/Widgets/Modal.h"
#include "GUI/Widgets/Toast.h"

namespace GUI::Screens::Renter {

namespace {

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

std::string layNgayHomNayStr() {
    auto now = std::chrono::system_clock::now();
    std::time_t tt = std::chrono::system_clock::to_time_t(now);
    std::tm localTm{};
#if defined(_WIN32)
    localtime_s(&localTm, &tt);
#else
    localtime_r(&tt, &localTm);
#endif
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << localTm.tm_mday << "/"
        << std::setfill('0') << std::setw(2) << (localTm.tm_mon + 1) << "/"
        << (localTm.tm_year + 1900);
    return oss.str();
}

std::string dinhDangTien(int soTien) {
    std::string s = std::to_string(soTien);
    int n = s.length() - 3;
    while (n > 0) {
        s.insert(n, ".");
        n -= 3;
    }
    return s + u8" đ";
}

}  // namespace

void LichDatScreen::onEnter() {
    tableCols_ = {
        {u8"Mã đơn", 100.0f, ImGuiTableColumnFlags_WidthFixed},
        {u8"Sân bóng", 110.0f, ImGuiTableColumnFlags_WidthFixed},
        {u8"Ngày đá", 110.0f, ImGuiTableColumnFlags_WidthFixed},
        {u8"Khung giờ", 140.0f, ImGuiTableColumnFlags_WidthFixed},
        {u8"Giá tiền", 120.0f, ImGuiTableColumnFlags_WidthFixed},
        {u8"Trạng thái", 140.0f, ImGuiTableColumnFlags_WidthFixed},
        {u8"Hành động", 112.0f, ImGuiTableColumnFlags_WidthFixed}
    };

    locTrangThai_ = 0;
    reloadViewModel();
}

void LichDatScreen::onExit() {
}

void LichDatScreen::reloadViewModel() {
    auto& facade = GUI::App::Facade::getInstance();
    User* curUser = GUI::App::Session::getInstance().getUser();
    std::string sdt = curUser ? curUser->get_phone_number() : "0909876543";

    soLanDaHuy_ = facade.getSoLanHuyCuaKhach(sdt);
    danhSachDonGoc_ = facade.getLichSuDatCuaKhach(sdt);

    // Lọc theo bộ lọc
    std::string todayStr = layNgayHomNayStr();
    int todayInt = parseNgaySangSo(todayStr);

    danhSachDonHienThi_.clear();
    for (const auto& don : danhSachDonGoc_) {
        int donNgayInt = parseNgaySangSo(don.ngay);

        if (locTrangThai_ == 1) {
            // Sắp tới: ngày >= hôm nay và chưa hủy
            if (donNgayInt >= todayInt && don.trangThai != u8"Đã hủy") {
                danhSachDonHienThi_.push_back(don);
            }
        } else if (locTrangThai_ == 2) {
            // Đã qua: ngày < hôm nay hoặc đã hoàn thành
            if (donNgayInt < todayInt || don.trangThai == u8"Đã hoàn thành") {
                danhSachDonHienThi_.push_back(don);
            }
        } else if (locTrangThai_ == 3) {
            // Đã hủy
            if (don.trangThai == u8"Đã hủy") {
                danhSachDonHienThi_.push_back(don);
            }
        } else {
            // Tất cả
            danhSachDonHienThi_.push_back(don);
        }
    }

    // Chuyển sang TableRows
    tableRows_.clear();
    for (const auto& don : danhSachDonHienThi_) {
        Widgets::TableRow row;
        row.id = don.maDon;
        row.oDuLieu = {
            don.maDon,
            u8"Sân " + don.maSan,
            don.ngay,
            don.khungGio,
            dinhDangTien(don.giaTien),
            don.trangThai,
            "" // Cột nút hủy
        };
        tableRows_.push_back(row);
    }
}

void LichDatScreen::render() {
    Widgets::Card("card_renter_history_table", u8"Lịch sử các đơn đặt sân của tôi", [this]() {
        renderBoLoc();
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        renderTableDonDat();
    });

    renderModalXacNhanHuy();
}

void LichDatScreen::renderBoLoc() {
    ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
    ImGui::TextUnformatted(u8"Lọc trạng thái đơn đặt:");
    ImGui::PopStyleColor();
    ImGui::SameLine();

    const char* boLocLabels[] = {
        u8"Tất cả đơn",
        u8"Lịch sắp tới",
        u8"Lịch đã qua",
        u8"Đã hủy"
    };

    for (int i = 0; i < 4; ++i) {
        if (i > 0) ImGui::SameLine(0.0f, 8.0f);

        bool isSelected = (locTrangThai_ == i);
        std::string btnFilterId = std::string(boLocLabels[i]) + "##filter";
        if (isSelected) {
            if (Widgets::NutChinh(btnFilterId.c_str(), ImVec2(0.0f, 32.0f))) {
                locTrangThai_ = i;
                reloadViewModel();
            }
        } else if (Widgets::NutPhu(btnFilterId.c_str(), ImVec2(0.0f, 32.0f))) {
            locTrangThai_ = i;
            reloadViewModel();
        }
    }
}

void LichDatScreen::renderTableDonDat() {
    if (tableRows_.empty()) {
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
        ImGui::TextUnformatted(u8"Không có đơn đặt sân nào trong mục này.");
        ImGui::PopStyleColor();
        return;
    }

    Widgets::Table(
        "tbl_renter_booking_history",
        tableCols_,
        tableRows_,
        tableState_,
        [this](const Widgets::TableRow& row, int col) {
            if (col == 5) {
                // Trạng thái Badge
                const std::string& tt = row.oDuLieu[5];
                if (tt == u8"Đã xác nhận") {
                    Widgets::Badge(tt.c_str(), GUI::Theme::Colors::GrassGreen, GUI::Theme::Colors::WhiteCard);
                } else if (tt == u8"Đã hoàn thành" || tt == u8"Đã thanh toán") {
                    Widgets::Badge(tt.c_str(), GUI::Theme::Colors::BlueInfo, GUI::Theme::Colors::WhiteCard);
                } else if (tt == u8"Đã hủy") {
                    Widgets::BadgeDanhSachDen(tt.c_str());
                } else {
                    Widgets::Badge(tt.c_str(), GUI::Theme::Colors::GrayHover, GUI::Theme::Colors::DarkText);
                }
            } else if (col == 6) {
                // Cột Thao tác: Nút Hủy
                bool daHuy = (row.oDuLieu[5] == u8"Đã hủy" || row.oDuLieu[5] == u8"Đã hoàn thành");
                if (!daHuy) {
                    std::string btnCancelId = u8"Hủy đơn##" + row.id;
                    if (Widgets::NutNguyHiem(btnCancelId.c_str(), ImVec2(80.0f, 24.0f))) {
                        maDonCanHuy_ = row.id;
                        Widgets::MoModalXacNhan(u8"Xác nhận hủy đặt sân");
                    }
                } else {
                    ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
                    ImGui::TextUnformatted(u8"--");
                    ImGui::PopStyleColor();
                }
            } else {
                ImGui::TextUnformatted(row.oDuLieu[col].c_str());
            }
        }
    );
}

void LichDatScreen::renderModalXacNhanHuy() {
    std::string noiDung = u8"Hủy nhiều lần sẽ vào danh sách đen, bạn đã hủy " +
                          std::to_string(soLanDaHuy_) +
                          u8" lần.\nBạn có chắc chắn muốn hủy đơn đặt sân mã " + maDonCanHuy_ + u8" không?";

    Widgets::ModalXacNhan(
        u8"Xác nhận hủy đặt sân",
        noiDung.c_str(),
        [this]() {
            User* curUser = GUI::App::Session::getInstance().getUser();
            std::string sdt = curUser ? curUser->get_phone_number() : "0909876543";

            auto kq = GUI::App::Facade::getInstance().huyDonDat(maDonCanHuy_, sdt);
            if (kq.ok) {
                reloadViewModel();
            } else {
            }
        },
        u8"Xác nhận hủy sân",
        u8"Không hủy",
        true // Nút đỏ nguy hiểm
    );
}

}  // namespace GUI::Screens::Renter
