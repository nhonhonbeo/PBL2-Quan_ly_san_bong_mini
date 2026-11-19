#include "GUI/Screens/Owner/FieldManagementScreen.h"

#include <algorithm>
#include <cstdio>
#include <imgui.h>

#include "Core/DateTime.h"
#include "GUI/Theme/Theme.h"
#include "GUI/App/Session.h"
#include "GUI/Widgets/Badge.h"
#include "GUI/Widgets/Button.h"
#include "GUI/Widgets/Card.h"
#include "GUI/Widgets/ComboRadio.h"
#include "GUI/Widgets/InputFields.h"
#include "GUI/Widgets/Modal.h"
#include "GUI/Widgets/TimePicker.h"
#include "GUI/Widgets/Toast.h"

namespace GUI::Screens::Owner {

FieldManagementScreen::FieldManagementScreen(std::function<void()> onChuyenSangWizard)
    : onChuyenSangWizardCallback_(std::move(onChuyenSangWizard)) {}

void FieldManagementScreen::onEnter() {
    const std::string today = DateTime::today();
    ngayXemSlot_ = today.substr(8, 2) + "/" + today.substr(5, 2) + "/" +
                   today.substr(0, 4);
    // Thiết lập cấu hình cột cho Bảng danh sách sân
    tableColsSan_ = {
        {u8"Mã sân", 90.0f, ImGuiTableColumnFlags_WidthFixed},
        {u8"Tên sân bóng", 200.0f, ImGuiTableColumnFlags_None},
        {u8"Loại sân", 100.0f, ImGuiTableColumnFlags_WidthFixed},
        {u8"Giá thuê (VNĐ/h)", 130.0f, ImGuiTableColumnFlags_WidthFixed},
        {u8"Trạng thái", 120.0f, ImGuiTableColumnFlags_WidthFixed},
        {u8"Hành động", 310.0f, ImGuiTableColumnFlags_WidthFixed}
    };

    reloadViewModel();
}

void FieldManagementScreen::onExit() {
}

void FieldManagementScreen::reloadViewModel() {
    auto& facade = GUI::App::Facade::getInstance();
    giaChoLuu_.clear();
    loiGiaThue_.clear();
    User* owner = GUI::App::Session::getInstance().getUser();
    LinkedList<VenueFacilityRecord> facilities;
    std::string facilityError;
    if (owner != nullptr &&
        facade.getDanhSachCoSo(facilities, facilityError)) {
        for (auto* node = facilities.get_head(); node != nullptr;
             node = node->next) {
            if (node->data.ownerId == owner->get_user_id()) {
                gioMoCua_ = node->data.openTime;
                gioDongCua_ = node->data.closeTime;
                buocThoiLuongIdx_ =
                    node->data.allows60Minutes &&
                    !node->data.allows90Minutes ? 0 : 1;
                break;
            }
        }
    }

    // 1. Nạp danh sách sân
    danhSachSan_ = facade.getDanhSachSanBongCuaChuSan();
    tableRowsSan_.clear();
    dsMaSanSlot_.clear();

    for (const auto& san : danhSachSan_) {
        Widgets::TableRow row;
        row.id = san.maSan;
        row.oDuLieu = {
            san.maSan,
            san.tenSan,
            u8"Sân " + std::to_string(san.loaiSan) + u8" người",
            std::to_string(san.giaThueGio),
            san.dangBaoTri ? u8"Bảo trì" : u8"Sẵn sàng",
            "" // Cột hành động
        };
        tableRowsSan_.push_back(row);
        dsMaSanSlot_.push_back(san.maSan);
    }

    // 2. Nạp danh sách khung giờ
    danhSachKhungGio_ = facade.getDanhSachKhungGio();
    dsKhungGioSlot_.clear();
    for (const auto& kg : danhSachKhungGio_) {
        dsKhungGioSlot_.push_back(kg.batDau + " - " + kg.ketThuc);
    }

    // 3. Nạp tình trạng Slot theo ngày
    maTranSlot_ = facade.getTinhTrangSlotTheoNgay(ngayXemSlot_);
}

void FieldManagementScreen::render() {
    // LÝ DO CHỌN: Tab Bar chuẩn Modern SaaS không cần ép màu chữ WhiteCard,
    // sử dụng TextPrimary trên nền Surface trắng và chỉ báo TabSelectedOverline xanh lá thương hiệu.
    if (ImGui::BeginTabBar("##FieldManagementTabBar", ImGuiTabBarFlags_None)) {
        if (ImGui::BeginTabItem(u8"Danh sách sân bóng")) {
            renderTabDanhSachSan();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem(u8"Khung giờ hoạt động")) {
            renderTabKhungGio();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem(u8"Bảng giá thuê")) {
            renderTabGiaThueMaTran();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem(u8"Tình trạng theo ngày")) {
            renderTabTinhTrangTheoNgay();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    // Render các Modal xác nhận thao tác
    renderModalXoaSan();
    renderModalSuaSan();
    renderModalThemNhanh();
    renderModalUploadAnhSan();
}

void FieldManagementScreen::renderTabDanhSachSan() {
    Widgets::Card("card_tab_field_list", u8"Danh sách sân bóng trong cơ sở", [this]() {
        if (Widgets::NutChinh(u8"+ Thêm nhanh sân mới", ImVec2(220.0f, 36.0f))) {
            loaiSanThemNhanh_ = 5;
            soLuongSanThemNhanh_ = 1;
            loiThemNhanh_.clear();
            Widgets::MoModalXacNhan(u8"Thêm nhanh sân bóng");
        }

        ImGui::Spacing();
        ImGui::Spacing();

        // Render Table danh sách sân bóng
        Widgets::Table(
            "tbl_field_list",
            tableColsSan_,
            tableRowsSan_,
            tableStateSan_,
            [this](const Widgets::TableRow& row, int col) {
                if (col == 4) {
                    // Trạng thái: hiển thị Badge
                    bool isBaoTri = (row.oDuLieu[4] == u8"Bảo trì");
                    if (isBaoTri) {
                        Widgets::BadgeBaoTri(u8"Bảo trì");
                    } else {
                        Widgets::BadgeTrong(u8"Sẵn sàng");
                    }
                } else if (col == 5) {
                    // Hành động: Sửa, Khóa/Mở, Xóa
                    std::string maSan = row.id;

                    // Nút Sửa
                    std::string btnSuaId = u8"Sửa##" + maSan;
                    if (Widgets::NutPhu(btnSuaId.c_str(), ImVec2(52.0f, 26.0f))) {
                        auto san = GUI::App::Facade::getInstance().getSanBong(maSan);
                        if (san) {
                            maSanDangSua_ = san->maSan;
                            tenSanDangSua_ = san->tenSan;
                            loaiSanDangSua_ = san->loaiSan;
                            Widgets::MoModalXacNhan(u8"Chỉnh sửa thông tin sân bóng");
                        }
                    }

                    ImGui::SameLine(0.0f, 6.0f);

                    // Nút Khóa/Mở bảo trì
                    bool isBaoTri = (row.oDuLieu[4] == u8"Bảo trì");
                    const char* btnBtLabel = isBaoTri ? u8"Mở##" : u8"Khóa##";
                    if (Widgets::NutPhu((std::string(btnBtLabel) + maSan).c_str(), ImVec2(56.0f, 26.0f))) {
                        auto kq = GUI::App::Facade::getInstance().toggleBaoTriSan(maSan);
                        if (kq.ok) {
                            reloadViewModel();
                        } else if (!kq.loi.empty()) {
                            Widgets::ToastLoi(kq.loi);
                        }
                    }

                    ImGui::SameLine(0.0f, 6.0f);

                    // Nút Xóa (yêu cầu ModalXacNhan)
                    std::string btnXoaId = u8"Xóa##" + maSan;
                    if (Widgets::NutNguyHiem(btnXoaId.c_str(), ImVec2(52.0f, 26.0f))) {
                        maSanCanXoa_ = maSan;
                        Widgets::MoModalXacNhan(u8"Xác nhận xóa sân bóng");
                    }
                } else {
                    ImGui::TextUnformatted(row.oDuLieu[col].c_str());
                }
            }
        );
    });
}

}  // namespace GUI::Screens::Owner
