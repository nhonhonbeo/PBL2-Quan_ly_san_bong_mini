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

void FieldManagementScreen::renderTabKhungGio() {
    Widgets::Card("card_tab_timeslot", u8"Thiết lập khung giờ hoạt động của cơ sở", [this]() {
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
        ImGui::TextWrapped(u8"Chọn giờ mở/đóng cửa và bước thời lượng để tự động tính toán và sinh toàn bộ các khung giờ phục vụ trong ngày.");
        ImGui::PopStyleColor();
        ImGui::Spacing();

        std::vector<std::string> dsBuoc = {u8"60 phút / trận", u8"90 phút / trận"};
        if (ImGui::BeginTable(
                "##operating_hours_layout",
                2,
                ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_NoBordersInBody
            )) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            Widgets::TimePicker(u8"Giờ mở cửa:", "tp_open_time", gioMoCua_);
            ImGui::TableSetColumnIndex(1);
            Widgets::TimePicker(u8"Giờ đóng cửa:", "tp_close_time", gioDongCua_);

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            Widgets::Combo(u8"Thời lượng mỗi ca:", "combo_buoc_gio", buocThoiLuongIdx_, dsBuoc);
            ImGui::TableSetColumnIndex(1);
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetTextLineHeightWithSpacing());
            if (Widgets::NutChinh(u8"Sinh khung giờ tự động", ImVec2(220.0f, 36.0f))) {
            int buoc = (buocThoiLuongIdx_ == 0) ? 60 : 90;
            auto kq = GUI::App::Facade::getInstance().sinhKhungGioTuDong(gioMoCua_, gioDongCua_, buoc);
            if (kq.ok) {
                reloadViewModel();
            } else if (!kq.loi.empty()) {
                Widgets::ToastLoi(kq.loi);
            }
            }
            ImGui::EndTable();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // 3. Danh sách khung giờ hiện tại
        ImGui::TextUnformatted(u8"Danh sách khung giờ đang áp dụng:");
        ImGui::Spacing();

        if (ImGui::BeginTable("##tbl_khung_gio", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn(u8"Mã KG", ImGuiTableColumnFlags_WidthFixed, 80.0f);
            ImGui::TableSetupColumn(u8"Giờ bắt đầu", ImGuiTableColumnFlags_WidthFixed, 120.0f);
            ImGui::TableSetupColumn(u8"Giờ kết thúc", ImGuiTableColumnFlags_WidthFixed, 120.0f);
            ImGui::TableSetupColumn(u8"Đặc tính", ImGuiTableColumnFlags_WidthFixed, 140.0f);
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
                    Widgets::BadgeNoiBat(u8"Giờ vàng");
                } else {
                    Widgets::Badge(u8"Giờ thường", GUI::Theme::Colors::GrayHover, GUI::Theme::Colors::DarkText);
                }
            }

            ImGui::EndTable();
        }
    });
}

void FieldManagementScreen::renderTabGiaThueMaTran() {
    Widgets::Card("card_tab_pricing_matrix", u8"Bảng giá thuê sân", [this]() {
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
                                ImGuiTableFlags_SizingStretchSame;

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
                    const auto priceKey = std::make_pair(kgStr, loai);
                    auto pending = giaChoLuu_.find(priceKey);
                    int giaTam = pending == giaChoLuu_.end()
                        ? giaHienTai
                        : pending->second;

                    std::string inputId = "##gia_" + std::to_string(r) + "_" + std::to_string(loai);

                    if (kg.laGioVang) {
                        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(1.0f, 0.96f, 0.85f, 1.0f));
                    }

                    if (Widgets::InputInt(nullptr, inputId.c_str(), giaTam, nullptr, 10000, 50000)) {
                        if (giaTam >= 50000) {
                            giaChoLuu_[priceKey] = giaTam;
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
        if (!loiGiaThue_.empty()) {
            ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::RedDanger);
            ImGui::TextWrapped("%s", loiGiaThue_.c_str());
            ImGui::PopStyleColor();
        }
        if (Widgets::NutChinh(u8"Lưu cấu hình biểu giá", ImVec2(200.0f, 36.0f))) {
            std::map<int, int> allTypePrices;
            for (const auto& pending : giaChoLuu_) {
                const bool applyAll = apDungTatCaCungLoai_;
                if (applyAll && allTypePrices.find(pending.first.second) != allTypePrices.end()) {
                    continue;
                }
                auto result = facade.capNhatGiaMaTran(
                    pending.first.first, pending.first.second,
                    pending.second, applyAll
                );
                if (!result.ok) {
                    loiGiaThue_ = result.loi;
                    break;
                }
                if (applyAll) {
                    allTypePrices[pending.first.second] = pending.second;
                }
            }
            if (loiGiaThue_.empty()) {
                reloadViewModel();
                Widgets::ToastThanhCong(u8"Đã lưu bảng giá của cơ sở.");
            }
        }
    });
}

void FieldManagementScreen::renderTabTinhTrangTheoNgay() {
    Widgets::Card("card_tab_slot_grid", u8"Tình trạng theo ngày", [this]() {
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
            true,
            "",
            "",
            [this](const std::string& maSan, const std::string& khungGio) {
                const auto slot = maTranSlot_.find({maSan, khungGio});
                if (slot == maTranSlot_.end()) return;
                if (slot->second.status == Widgets::SlotStatus::DaDat) {
                    const auto result = GUI::App::Facade::getInstance().xacNhanDaHoanThanhDon(slot->second.ghiChu);
                    if (result.ok) {
                        reloadViewModel();
                    } else if (!result.loi.empty()) {
                        Widgets::ToastLoi(result.loi);
                    }
                }
            }
        );

        if (coDoiNgay) {
            reloadViewModel();
        }
    });
}

}  // namespace GUI::Screens::Owner
