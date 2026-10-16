#include "GUI/Screens/Owner/FieldManagementScreen.h"

#include <algorithm>
#include <cstdio>
#include <imgui.h>

#include "GUI/Theme/Theme.h"
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
    // Thiết lập cấu hình cột cho Bảng danh sách sân
    tableColsSan_ = {
        {u8"Mã sân", 90.0f, ImGuiTableColumnFlags_WidthFixed},
        {u8"Tên sân bóng", 200.0f, ImGuiTableColumnFlags_None},
        {u8"Loại sân", 100.0f, ImGuiTableColumnFlags_WidthFixed},
        {u8"Giá thuê (VNĐ/h)", 130.0f, ImGuiTableColumnFlags_WidthFixed},
        {u8"Trạng thái", 120.0f, ImGuiTableColumnFlags_WidthFixed},
        {u8"Hành động", 220.0f, ImGuiTableColumnFlags_WidthFixed}
    };

    reloadViewModel();
}

void FieldManagementScreen::onExit() {
}

void FieldManagementScreen::reloadViewModel() {
    auto& facade = GUI::App::Facade::getInstance();

    // 1. Nạp danh sách sân
    danhSachSan_ = facade.getDanhSachSanBong();
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
    if (ImGui::BeginTabBar("##FieldManagementTabBar", ImGuiTabBarFlags_None)) {
        if (ImGui::BeginTabItem(u8"a. Danh sách sân bóng")) {
            renderTabDanhSachSan();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem(u8"b. Khung giờ hoạt động")) {
            renderTabKhungGio();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem(u8"c. Bảng giá thuê ma trận")) {
            renderTabGiaThueMaTran();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem(u8"d. Tình trạng theo ngày (SlotGrid)")) {
            renderTabTinhTrangTheoNgay();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    // Render các Modal xác nhận thao tác
    renderModalXoaSan();
    renderModalSuaSan();
    renderModalXoaKhungGio();
}

void FieldManagementScreen::renderTabDanhSachSan() {
    Widgets::Card("card_tab_field_list", u8"Danh sách sân bóng trong cơ sở", [this]() {
        // Nút mở nhanh Wizard tạo sân
        if (Widgets::NutChinh(u8"+ Khởi tạo sân mới (Wizard 4 bước)", ImVec2(280.0f, 36.0f))) {
            if (onChuyenSangWizardCallback_) {
                onChuyenSangWizardCallback_();
            }
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
                            giaThueDangSua_ = san->giaThueGio;
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
                            Widgets::ToastThanhCong(u8"Đã chuyển trạng thái sân " + maSan);
                            reloadViewModel();
                        } else {
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

void FieldManagementScreen::renderTabKhungGio() {
    Widgets::Card("card_tab_timeslot", u8"Thiết lập khung giờ hoạt động của cơ sở", [this]() {
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
        ImGui::TextWrapped(u8"Chọn giờ mở/đóng cửa và bước thời lượng để tự động tính toán và sinh toàn bộ các khung giờ phục vụ trong ngày.");
        ImGui::PopStyleColor();
        ImGui::Spacing();

        // 1. TimePicker chọn Giờ mở & Giờ đóng
        ImGui::BeginGroup();
        ImGui::SetNextItemWidth(160.0f);
        Widgets::TimePicker(u8"Giờ mở cửa:", "tp_open_time", gioMoCua_);
        ImGui::EndGroup();

        ImGui::SameLine(0.0f, 24.0f);

        ImGui::BeginGroup();
        ImGui::SetNextItemWidth(160.0f);
        Widgets::TimePicker(u8"Giờ đóng cửa:", "tp_close_time", gioDongCua_);
        ImGui::EndGroup();

        ImGui::SameLine(0.0f, 24.0f);

        // 2. Bước thời lượng (60p / 90p / 120p)
        ImGui::BeginGroup();
        std::vector<std::string> dsBuoc = {u8"60 phút / trận", u8"90 phút / trận", u8"120 phút / trận"};
        ImGui::SetNextItemWidth(180.0f);
        Widgets::Combo(u8"Thời lượng mỗi trận:", "combo_buoc_gio", buocThoiLuongIdx_, dsBuoc);
        ImGui::EndGroup();

        ImGui::SameLine(0.0f, 24.0f);

        // Nút sinh tự động
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 20.0f);
        if (Widgets::NutChinh(u8"⚡ Sinh khung giờ tự động", ImVec2(200.0f, 36.0f))) {
            int buoc = (buocThoiLuongIdx_ == 0) ? 60 : ((buocThoiLuongIdx_ == 1) ? 90 : 120);
            auto kq = GUI::App::Facade::getInstance().sinhKhungGioTuDong(gioMoCua_, gioDongCua_, buoc);
            if (kq.ok) {
                Widgets::ToastThanhCong(u8"Đã tự động tính toán và cập nhật danh sách khung giờ mới!");
                reloadViewModel();
            } else {
                Widgets::ToastLoi(kq.loi);
            }
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // 3. Danh sách khung giờ hiện tại
        ImGui::TextUnformatted(u8"Danh sách khung giờ đang áp dụng:");
        ImGui::Spacing();

        if (ImGui::BeginTable("##tbl_khung_gio", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn(u8"Mã KG", ImGuiTableColumnFlags_WidthFixed, 80.0f);
            ImGui::TableSetupColumn(u8"Giờ bắt đầu", ImGuiTableColumnFlags_WidthFixed, 120.0f);
            ImGui::TableSetupColumn(u8"Giờ kết thúc", ImGuiTableColumnFlags_WidthFixed, 120.0f);
            ImGui::TableSetupColumn(u8"Đặc tính", ImGuiTableColumnFlags_WidthFixed, 140.0f);
            ImGui::TableSetupColumn(u8"Thao tác", ImGuiTableColumnFlags_WidthFixed, 100.0f);
            ImGui::TableHeadersRow();

            for (const auto& kg : danhSachKhungGio_) {
                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted(kg.id.c_str());

                ImGui::TableSetColumnIndex(1);
                ImGui::TextUnformatted(kg.batDau.c_str());

                ImGui::TableSetColumnIndex(2);
                ImGui::TextUnformatted(kg.ketThuc.c_str());

                ImGui::TableSetColumnIndex(3);
                if (kg.laGioVang) {
                    Widgets::Badge(u8"Giờ vàng", GUI::Theme::Colors::GoldAccent, GUI::Theme::Colors::WhiteCard);
                } else {
                    Widgets::Badge(u8"Giờ thường", GUI::Theme::Colors::GrayHover, GUI::Theme::Colors::DarkText);
                }

                ImGui::TableSetColumnIndex(4);
                std::string btnXoaKgId = u8"Xóa##kg_" + kg.id;
                if (Widgets::NutNguyHiem(btnXoaKgId.c_str(), ImVec2(60.0f, 24.0f))) {
                    idKhungGioCanXoa_ = kg.id;
                    Widgets::MoModalXacNhan(u8"Xác nhận xóa khung giờ");
                }
            }

            ImGui::EndTable();
        }
    });
}

void FieldManagementScreen::renderTabGiaThueMaTran() {
    Widgets::Card("card_tab_pricing_matrix", u8"Bảng giá thuê ma trận (Khung giờ x Loại sân)", [this]() {
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
        ImGui::TextWrapped(u8"Cấu hình biểu giá thuê trực tiếp tại từng ô. Các khung giờ vàng được tô nền vàng nhạt để dễ phân biệt.");
        ImGui::PopStyleColor();
        ImGui::Spacing();

        // Checkbox áp dụng cho tất cả sân cùng loại
        ImGui::Checkbox(u8"Áp dụng mức giá thay đổi cho TẤT CẢ khung giờ của cùng loại sân", &apDungTatCaCungLoai_);
        ImGui::Spacing();
        ImGui::Spacing();

        auto& facade = GUI::App::Facade::getInstance();
        ImGuiTableFlags flags = ImGuiTableFlags_Borders |
                                ImGuiTableFlags_RowBg |
                                ImGuiTableFlags_SizingStretchProp;

        if (ImGui::BeginTable("##tbl_pricing_matrix", 4, flags)) {
            ImGui::TableSetupColumn(u8"Khung giờ", ImGuiTableColumnFlags_WidthFixed, 150.0f);
            ImGui::TableSetupColumn(u8"Sân 5 người (VNĐ)", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn(u8"Sân 7 người (VNĐ)", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn(u8"Sân 11 người (VNĐ)", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableHeadersRow();

            for (size_t r = 0; r < danhSachKhungGio_.size(); ++r) {
                const auto& kg = danhSachKhungGio_[r];
                std::string kgStr = kg.batDau + " - " + kg.ketThuc;

                ImGui::TableNextRow();

                // Cột Khung giờ
                ImGui::TableSetColumnIndex(0);
                if (kg.laGioVang) {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.78f, 0.45f, 0.05f, 1.0f));
                    ImGui::Text("%s [Vàng]", kgStr.c_str());
                    ImGui::PopStyleColor();
                } else {
                    ImGui::TextUnformatted(kgStr.c_str());
                }

                // Cột giá sân 5, 7, 11
                int loaiSans[] = {5, 7, 11};
                for (int c = 0; c < 3; ++c) {
                    ImGui::TableSetColumnIndex(c + 1);

                    int loai = loaiSans[c];
                    int giaHienTai = facade.getGiaMaTran(kgStr, loai);
                    int giaTam = giaHienTai;

                    std::string inputId = "##gia_" + std::to_string(r) + "_" + std::to_string(loai);
                    ImGui::SetNextItemWidth(-1.0f);

                    if (kg.laGioVang) {
                        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(1.0f, 0.96f, 0.85f, 1.0f));
                    }

                    if (ImGui::InputInt(inputId.c_str(), &giaTam, 10000, 50000)) {
                        if (giaTam >= 50000) {
                            facade.capNhatGiaMaTran(kgStr, loai, giaTam, apDungTatCaCungLoai_);
                        }
                    }

                    if (kg.laGioVang) {
                        ImGui::PopStyleColor();
                    }
                }
            }

            ImGui::EndTable();
        }

        ImGui::Spacing();
        if (Widgets::NutChinh(u8"Lưu cấu hình biểu giá", ImVec2(200.0f, 36.0f))) {
            Widgets::ToastThanhCong(u8"Đã lưu thành công biểu giá ma trận thuê sân!");
        }
    });
}

void FieldManagementScreen::renderTabTinhTrangTheoNgay() {
    Widgets::Card("card_tab_slot_grid", u8"Tình trạng sân bóng theo ngày (SlotGrid)", [this]() {
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
        ImGui::TextWrapped(u8"Xem trực quan tình trạng Trống / Đã đặt / Bảo trì của mọi sân bóng trong cụm theo từng khung giờ trong ngày.");
        ImGui::PopStyleColor();
        ImGui::Spacing();

        // Render widget dùng chung SlotGrid
        bool coDoiNgay = Widgets::SlotGrid::Render(
            "owner_slot_grid",
            ngayXemSlot_,
            dsMaSanSlot_,
            dsKhungGioSlot_,
            maTranSlot_,
            false // Chế độ Chủ sân chỉ xem tình trạng
        );

        if (coDoiNgay) {
            reloadViewModel();
            Widgets::ToastThongTin(u8"Đã nạp tình trạng sân ngày " + ngayXemSlot_);
        }
    });
}

void FieldManagementScreen::renderModalXoaSan() {
    Widgets::ModalXacNhan(
        u8"Xác nhận xóa sân bóng",
        (u8"Bạn có chắc chắn muốn xóa vĩnh viễn sân bóng '" + maSanCanXoa_ + u8"' khỏi cơ sở không?\nMọi dữ liệu liên quan đến sân này sẽ bị xóa bỏ.").c_str(),
        [this]() {
            auto kq = GUI::App::Facade::getInstance().xoaSanBong(maSanCanXoa_);
            if (kq.ok) {
                Widgets::ToastThanhCong(u8"Đã xóa sân bóng " + maSanCanXoa_ + u8" thành công!");
                reloadViewModel();
            } else {
                Widgets::ToastLoi(kq.loi);
            }
        },
        u8"Xóa vĩnh viễn",
        u8"Hủy bỏ",
        true // Nút đỏ nguy hiểm
    );
}

void FieldManagementScreen::renderModalSuaSan() {
    // Modal tùy biến chỉnh sửa thông tin sân bóng
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(440.0f, 0.0f), ImGuiCond_Appearing);

    if (ImGui::BeginPopupModal(u8"Chỉnh sửa thông tin sân bóng", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Spacing();
        ImGui::Text(u8"Mã sân định danh: %s", maSanDangSua_.c_str());
        ImGui::Spacing();

        Widgets::InputText(u8"Tên sân bóng:", "edit_field_name", tenSanDangSua_);
        ImGui::Spacing();

        Widgets::InputInt(u8"Giá thuê niêm yết (VNĐ/h):", "edit_field_price", giaThueDangSua_, nullptr, 10000, 50000);
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (Widgets::NutChinh(u8"Lưu thay đổi", ImVec2(130.0f, 34.0f))) {
            auto kq = GUI::App::Facade::getInstance().suaSanBong(maSanDangSua_, tenSanDangSua_, loaiSanDangSua_, giaThueDangSua_);
            if (kq.ok) {
                Widgets::ToastThanhCong(u8"Đã cập nhật thông tin sân " + maSanDangSua_);
                reloadViewModel();
                ImGui::CloseCurrentPopup();
            } else {
                Widgets::ToastLoi(kq.loi);
            }
        }

        ImGui::SameLine();
        if (Widgets::NutPhu(u8"Hủy bỏ", ImVec2(100.0f, 34.0f))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void FieldManagementScreen::renderModalXoaKhungGio() {
    Widgets::ModalXacNhan(
        u8"Xác nhận xóa khung giờ",
        (u8"Bạn có chắc chắn muốn xóa khung giờ '" + idKhungGioCanXoa_ + u8"' không?").c_str(),
        [this]() {
            auto kq = GUI::App::Facade::getInstance().xoaKhungGio(idKhungGioCanXoa_);
            if (kq.ok) {
                Widgets::ToastThanhCong(u8"Đã xóa khung giờ " + idKhungGioCanXoa_);
                reloadViewModel();
            } else {
                Widgets::ToastLoi(kq.loi);
            }
        },
        u8"Xóa",
        u8"Hủy",
        true
    );
}

}  // namespace GUI::Screens::Owner
