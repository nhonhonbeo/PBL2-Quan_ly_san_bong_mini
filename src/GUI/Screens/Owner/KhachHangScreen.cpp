#include "GUI/Screens/Owner/KhachHangScreen.h"

#include <algorithm>

#include <imgui.h>

#include "GUI/Theme/Theme.h"
#include "GUI/Widgets/Card.h"

namespace GUI::Screens::Owner {

void KhachHangScreen::onEnter() {
    // Cấu hình bảng danh bạ khách hàng
    tableColsDanhBa_ = {
        {u8"STT", 60.0f, ImGuiTableColumnFlags_WidthFixed},
        {u8"Họ và tên", 180.0f, ImGuiTableColumnFlags_None},
        {u8"Số điện thoại", 130.0f, ImGuiTableColumnFlags_WidthFixed},
        {u8"Số lần đặt", 110.0f, ImGuiTableColumnFlags_WidthFixed},
        {u8"Số lần hủy", 110.0f, ImGuiTableColumnFlags_WidthFixed}
    };

    reloadViewModel();
}

void KhachHangScreen::onExit() {
}

void KhachHangScreen::reloadViewModel() {
    auto& facade = GUI::App::Facade::getInstance();

    // 1. Nạp danh bạ khách hàng
    danhSachKhach_ = facade.getDanhSachKhachHangCuaChuSan();
    tableRowsDanhBa_.clear();

    for (size_t i = 0; i < danhSachKhach_.size(); ++i) {
        const auto& kh = danhSachKhach_[i];
        Widgets::TableRow row;
        row.id = kh.sdt; // Dùng SĐT làm định danh duy nhất

        row.oDuLieu = {
            std::to_string(i + 1),
            kh.hoTen,
            kh.sdt,
            std::to_string(kh.soLanDat),
            std::to_string(kh.soLanHuy)
        };
        tableRowsDanhBa_.push_back(row);
    }

    // 2. Nạp chi tiết khách hàng đang chọn (nếu có)
    // LÝ DO CHỌN: Nạp chi tiết từ danh sách đã lọc theo chủ sân, không tra cứu dữ liệu toàn cục.
    if (!sdtDangChon_.empty()) {
        auto khach = std::find_if(
            danhSachKhach_.begin(),
            danhSachKhach_.end(),
            [this](const GUI::App::KhachHangInfo& item) {
                return item.sdt == sdtDangChon_;
            }
        );
        if (khach != danhSachKhach_.end()) {
            khachDangChon_ = *khach;
            lichSuDatCuaKhach_ =
                facade.getLichSuDatCuaKhachCuaChuSan(sdtDangChon_);
            danhGiaCuaKhach_ =
                facade.getDanhGiaCuaKhachCuaChuSan(sdtDangChon_);
        } else {
            sdtDangChon_.clear();
        }
    }

    // 3. Nạp danh sách đánh giá nhận xét
    // LÝ DO CHỌN: LinkedList quản lý danh sách đánh giá nhận xét,
    // các đánh giá mới nhất được chèn vào đầu danh sách với độ phức tạp O(1).
    danhSachSan_ = facade.getDanhSachSanBongCuaChuSan();
    danhSachDanhGia_ = facade.getDanhSachDanhGia();
    diemTBToanCoSo_ = facade.getDiemTrungBinhToanCoSo();
    diemTBTungSan_ = facade.getDiemTrungBinhTungSan();
}

void KhachHangScreen::render() {
    // LÝ DO CHỌN: Tab Bar chuẩn Modern SaaS hiển thị TextPrimary tương phản cao trên nền Surface
    if (ImGui::BeginTabBar("##KhachHangTabBar", ImGuiTabBarFlags_None)) {
        if (ImGui::BeginTabItem(u8"Danh bạ khách hàng")) {
            renderTabDanhBa();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem(u8"Đánh giá & Nhận xét")) {
            renderTabDanhGia();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

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
                (void)col;
                ImGui::TextUnformatted(row.oDuLieu[col].c_str());
            },
            [this](const Widgets::TableRow& clickedRow) {
                // Click dòng -> chọn khách hàng và nạp chi tiết
                sdtDangChon_ = clickedRow.id;
                reloadViewModel();
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


}  // namespace GUI::Screens::Owner
