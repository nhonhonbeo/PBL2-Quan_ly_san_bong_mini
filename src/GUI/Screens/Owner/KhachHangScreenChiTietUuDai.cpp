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

        // 1. Avatar tròn và Header thông tin khách hàng
        ImVec2 avatarPos = ImGui::GetCursorScreenPos();
        float avatarRadius = 22.0f;
        ImVec2 avatarCenter(avatarPos.x + avatarRadius, avatarPos.y + avatarRadius);
        ImDrawList* dl = ImGui::GetWindowDrawList();

        // Nền tròn avatar màu PrimaryLight (#E8F6EF), viền Primary (#16865B)
        dl->AddCircleFilled(avatarCenter, avatarRadius, GUI::Theme::Colors::toU32(GUI::Theme::Colors::PrimaryLight));
        dl->AddCircle(avatarCenter, avatarRadius, GUI::Theme::Colors::toU32(GUI::Theme::Colors::Primary), 0, 1.5f);

        // Ký tự đầu họ tên
        std::string initial = u8"K";
        if (!khachDangChon_.hoTen.empty()) {
            initial = khachDangChon_.hoTen.substr(0, 1);
        }
        if (GUI::Theme::getFonts().title) ImGui::PushFont(GUI::Theme::getFonts().title);
        ImVec2 initSize = ImGui::CalcTextSize(initial.c_str());
        dl->AddText(ImVec2(avatarCenter.x - initSize.x * 0.5f, avatarCenter.y - initSize.y * 0.5f),
                    GUI::Theme::Colors::toU32(GUI::Theme::Colors::Primary), initial.c_str());
        if (GUI::Theme::getFonts().title) ImGui::PopFont();

        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + avatarRadius * 2.0f + 14.0f);
        ImGui::BeginGroup();
        if (GUI::Theme::getFonts().title) ImGui::PushFont(GUI::Theme::getFonts().title);
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::DarkText);
        ImGui::TextUnformatted(khachDangChon_.hoTen.c_str());
        ImGui::PopStyleColor();
        if (GUI::Theme::getFonts().title) ImGui::PopFont();

        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::GrassGreen);
        ImGui::Text(u8"SĐT: %s", khachDangChon_.sdt.c_str());
        ImGui::PopStyleColor();

        // Badge trạng thái
        if (khachDangChon_.biChan) {
            Widgets::BadgeDanhSachDen(u8"Bị chặn");
        } else if (khachDangChon_.mucUuDaiPhanTram > 0) {
            Widgets::BadgeVIP((u8"VIP " + std::to_string(khachDangChon_.mucUuDaiPhanTram) + "%").c_str());
        } else {
            Widgets::Badge(u8"Tiêu chuẩn", GUI::Theme::Colors::GrayHover, GUI::Theme::Colors::DarkText);
        }
        ImGui::EndGroup();

        ImGui::Spacing();
        ImGui::Spacing();

        if (khachDangChon_.biChan) {
            ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::RedDanger);
            ImGui::TextWrapped(u8"Lý do chặn: %s", khachDangChon_.lyDoChan.c_str());
            ImGui::Text(u8"Ngày chặn: %s", khachDangChon_.ngayChan.c_str());
            ImGui::PopStyleColor();
            ImGui::Spacing();
        }

        // 2. Hai thẻ thống kê nhỏ (Metric Cards)
        float statCardW = (availW - 10.0f) * 0.5f;
        auto veMiniCard = [dl](ImVec2 pos, float w, const char* label, const char* val, ImVec4 accent) {
            ImVec2 p1(pos.x + w, pos.y + 54.0f);
            dl->AddRectFilled(pos, p1, GUI::Theme::Colors::toU32(GUI::Theme::Colors::GrayHover), 6.0f);
            dl->AddRect(pos, p1, GUI::Theme::Colors::toU32(GUI::Theme::Colors::Border), 6.0f);
            dl->AddRectFilled(pos, ImVec2(pos.x + 3.0f, p1.y), GUI::Theme::Colors::toU32(accent), 6.0f);

            dl->AddText(ImVec2(pos.x + 10.0f, pos.y + 6.0f), GUI::Theme::Colors::toU32(GUI::Theme::Colors::MutedText), label);
            dl->AddText(ImVec2(pos.x + 10.0f, pos.y + 26.0f), GUI::Theme::Colors::toU32(GUI::Theme::Colors::DarkText), val);
        };

        ImVec2 card1Pos = ImGui::GetCursorScreenPos();
        std::string valDat = std::to_string(khachDangChon_.soLanDat) + u8" lần";
        veMiniCard(card1Pos, statCardW, u8"Đã đặt thành công", valDat.c_str(), GUI::Theme::Colors::Primary);

        ImVec2 card2Pos(card1Pos.x + statCardW + 10.0f, card1Pos.y);
        float tyLeHuy = 0.0f;
        int tongLuot = khachDangChon_.soLanDat + khachDangChon_.soLanHuy;
        if (tongLuot > 0) {
            tyLeHuy = (static_cast<float>(khachDangChon_.soLanHuy) / tongLuot) * 100.0f;
        }
        char valHuy[64];
        std::snprintf(valHuy, sizeof(valHuy), u8"%d lần (%.0f%%)", khachDangChon_.soLanHuy, tyLeHuy);
        veMiniCard(card2Pos, statCardW, u8"Đã hủy lịch", valHuy, GUI::Theme::Colors::Danger);

        ImGui::Dummy(ImVec2(availW, 60.0f));

        ImGui::Separator();
        ImGui::Spacing();

        // 3. Lịch sử đặt sân gần đây của khách
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

        // 4. Đánh giá đã gửi
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

        // 5. Thao tác chặn / gỡ chặn nhanh
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
                        reloadViewModel();
                    } else {
                    }
                }
            }

            ImGui::EndTable();
        }

        ImGui::Spacing();
        ImGui::Spacing();

        if (Widgets::NutChinh(u8"Áp dụng ưu đãi cho toàn bộ khách hàng", ImVec2(340.0f, 38.0f))) {
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

}  // namespace GUI::Screens::Owner
