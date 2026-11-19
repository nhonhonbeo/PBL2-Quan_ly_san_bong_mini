#include "GUI/Screens/Owner/KhachHangScreen.h"

#include <cstdio>
#include <string>
#include <imgui.h>

#include "GUI/Theme/Theme.h"
#include "GUI/Theme/Fonts.h"
#include "GUI/Widgets/Button.h"
#include "GUI/Widgets/Card.h"
#include "GUI/Widgets/StarRating.h"

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

        ImGui::EndGroup();

        ImGui::Spacing();
        ImGui::Spacing();

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

    });
}

}  // namespace GUI::Screens::Owner
