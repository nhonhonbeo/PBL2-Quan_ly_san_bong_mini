#include "GUI/Screens/Owner/KhachHangScreen.h"

#include <algorithm>
#include <cstdio>
#include <iomanip>
#include <sstream>
#include <imgui.h>

#include "GUI/Theme/Theme.h"
#include "GUI/Theme/Fonts.h"
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
    danhSachKhach_ = facade.getDanhSachKhachHangCuaChuSan();
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

        if (ImGui::BeginTabItem(u8"Ngưỡng ưu đãi & Khách VIP")) {
            renderTabUuDai();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem(u8"Danh sách đen & Đề xuất")) {
            renderTabDanhSachDen();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem(u8"Đánh giá & Nhận xét")) {
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


void KhachHangScreen::renderModalXacNhanChan() {
    std::string noiDung = u8"Bạn có chắc chắn muốn đưa khách hàng có SĐT " + sdtCanChan_ +
                          u8" vào danh sách đen.\nKhách hàng này sẽ bị khóa tài khoản đặt sân.";

    Widgets::ModalXacNhan(
        u8"Xác nhận đưa vào danh sách đen",
        noiDung.c_str(),
        [this]() {
            // LÝ DO CHỌN: LinkedList quản lý danh sách đen, chèn khách hàng bị chặn với O(1)
            auto kq = GUI::App::Facade::getInstance().themVaoDanhSachDen(sdtCanChan_, lyDoCanChan_);
            if (kq.ok) {
                reloadViewModel();
            } else {
            }
        },
        u8"Xác nhận chặn",
        u8"Hủy bỏ",
        true // Nút đỏ nguy hiểm
    );
}

void KhachHangScreen::renderModalXacNhanGoChan() {
    std::string noiDung = u8"Bạn có chắc chắn muốn gỡ bỏ khách hàng có SĐT " + sdtCanGoChan_ +
                          u8" khỏi danh sách đen.\nKhách hàng sẽ được khôi phục quyền đặt sân.";

    Widgets::ModalXacNhan(
        u8"Xác nhận gỡ khỏi danh sách đen",
        noiDung.c_str(),
        [this]() {
            // LÝ DO CHỌN: LinkedList gỡ bỏ phần tử khách hàng khỏi danh sách đen với O(1)
            auto kq = GUI::App::Facade::getInstance().goKhoiDanhSachDen(sdtCanGoChan_);
            if (kq.ok) {
                reloadViewModel();
            } else {
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
        u8"Hệ thống sẽ tự động quét toàn bộ lịch sử đặt sân của tất cả khách hàng và áp dụng mức giảm giá VIP tương ứng.\nVui lòng xác nhận để tiếp tục.",
        [this]() {
            auto kq = GUI::App::Facade::getInstance().apDungUuDaiChoKhach();
            if (kq.ok) {
                reloadViewModel();
            } else {
            }
        },
        u8"Đồng ý áp dụng",
        u8"Hủy bỏ",
        false
    );
}

}  // namespace GUI::Screens::Owner
