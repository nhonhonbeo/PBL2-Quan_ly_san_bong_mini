#include "GUI/Screens/Renter/UuDaiScreen.h"

#include <cstdio>
#include <string>
#include <vector>
#include <imgui.h>

#include "GUI/App/Session.h"
#include "GUI/Theme/Theme.h"
#include "GUI/Theme/Fonts.h"
#include "GUI/Widgets/Badge.h"
#include "GUI/Widgets/Card.h"
#include "GUI/Widgets/ProgressBar.h"

namespace GUI::Screens::Renter {

void UuDaiScreen::onEnter() {
    reloadViewModel();
}

void UuDaiScreen::onExit() {
}

void UuDaiScreen::reloadViewModel() {
    auto& facade = GUI::App::Facade::getInstance();
    User* curUser = GUI::App::Session::getInstance().getUser();
    std::string sdt = curUser ? curUser->get_phone_number() : "0909876543";

    soLanDat_ = facade.getSoLanDatCuaKhach(sdt);
    mucUuDaiHienTai_ = facade.getMucUuDaiCuaKhach(sdt);
    danhSachNguong_ = facade.getNguongUuDai();

    // Xác định hạng hiện tại
    tenHangHienTai_ = u8"Khách hàng tiêu chuẩn";
    for (auto it = danhSachNguong_.rbegin(); it != danhSachNguong_.rend(); ++it) {
        if (soLanDat_ >= it->soLanToiThieu) {
            tenHangHienTai_ = it->tenHang;
            break;
        }
    }

    // Xác định mốc kế tiếp
    mocKeTiep_ = 0;
    phanTramKeTiep_ = 0;
    tenHangKeTiep_.clear();
    soLuotConLai_ = 0;
    tienDoKeTiep_ = 1.0f;

    for (const auto& n : danhSachNguong_) {
        if (soLanDat_ < n.soLanToiThieu) {
            mocKeTiep_ = n.soLanToiThieu;
            phanTramKeTiep_ = n.phanTramGiam;
            tenHangKeTiep_ = n.tenHang;
            soLuotConLai_ = n.soLanToiThieu - soLanDat_;
            tienDoKeTiep_ = static_cast<float>(soLanDat_) / static_cast<float>(n.soLanToiThieu);
            break;
        }
    }
}

void UuDaiScreen::render() {
    renderThongKeUuDai();
    ImGui::Spacing();
    ImGui::Spacing();
    renderBangCacMoc();
}

void UuDaiScreen::renderThongKeUuDai() {
    float availW = ImGui::GetContentRegionAvail().x;
    float cardSpacing = 16.0f;
    float cardW = (availW - cardSpacing * 2.0f) / 3.0f;

    auto veTheKPI = [cardW](const char* id, const char* tieuDe, const char* giaTri, const char* moTa, ImVec4 accentColor) {
        ImGui::PushID(id);
        ImVec2 p0 = ImGui::GetCursorScreenPos();
        float cardH = 110.0f;
        ImVec2 p1 = ImVec2(p0.x + cardW, p0.y + cardH);

        ImDrawList* dl = ImGui::GetWindowDrawList();

        // 1. Bóng đổ khuếch tán nhẹ
        dl->AddRectFilled(ImVec2(p0.x, p0.y + 2.0f), ImVec2(p1.x, p1.y + 3.0f), IM_COL32(0, 0, 0, 8), 8.0f);

        // 2. Nền trắng và viền chuẩn
        dl->AddRectFilled(p0, p1, GUI::Theme::Colors::toU32(GUI::Theme::Colors::Surface), 8.0f);
        dl->AddRect(p0, p1, GUI::Theme::Colors::toU32(GUI::Theme::Colors::Border), 8.0f, 0, 1.0f);

        // 3. Vạch màu chỉ báo trên đỉnh
        dl->AddRectFilled(p0, ImVec2(p1.x, p0.y + 3.5f), GUI::Theme::Colors::toU32(accentColor), 8.0f, ImDrawFlags_RoundCornersTop);

        // 4. Nội dung text
        ImGui::SetCursorScreenPos(ImVec2(p0.x + 16.0f, p0.y + 14.0f));
        ImGui::BeginGroup();

        // Tiêu đề nhỏ viết hoa
        if (GUI::Theme::getFonts().small) ImGui::PushFont(GUI::Theme::getFonts().small);
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::TextSecondary);
        ImGui::TextUnformatted(tieuDe);
        ImGui::PopStyleColor();
        if (GUI::Theme::getFonts().small) ImGui::PopFont();

        ImGui::Spacing();

        // Giá trị số nổi bật
        if (GUI::Theme::getFonts().title) ImGui::PushFont(GUI::Theme::getFonts().title);
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::TextPrimary);
        ImGui::TextUnformatted(giaTri);
        ImGui::PopStyleColor();
        if (GUI::Theme::getFonts().title) ImGui::PopFont();

        ImGui::Spacing();

        // Mô tả phụ
        if (GUI::Theme::getFonts().small) ImGui::PushFont(GUI::Theme::getFonts().small);
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::TextSecondary);
        ImGui::TextUnformatted(moTa);
        ImGui::PopStyleColor();
        if (GUI::Theme::getFonts().small) ImGui::PopFont();

        ImGui::EndGroup();

        ImGui::SetCursorScreenPos(ImVec2(p1.x, p0.y));
        ImGui::PopID();
    };

    // Hàng 3 thẻ KPI
    float startX = ImGui::GetCursorPosX();
    float startY = ImGui::GetCursorPosY();

    std::string valDat = std::to_string(soLanDat_) + u8" lượt";
    std::string valGiam = std::to_string(mucUuDaiHienTai_) + "%";

    ImGui::SetCursorPos(ImVec2(startX, startY));
    veTheKPI("kpi_renter_bookings", u8"TỔNG LƯỢT ĐẶT THÀNH CÔNG", valDat.c_str(), u8"Tích lũy từ khi đăng ký tài khoản", GUI::Theme::Colors::Primary);

    ImGui::SetCursorPos(ImVec2(startX + cardW + cardSpacing, startY));
    veTheKPI("kpi_renter_discount", u8"MỨC CHIẾT KHẤU HIỆN CÓ", valGiam.c_str(), u8"Trừ trực tiếp trên hóa đơn mỗi giờ", GUI::Theme::Colors::GoldAccent);

    ImGui::SetCursorPos(ImVec2(startX + (cardW + cardSpacing) * 2.0f, startY));
    veTheKPI("kpi_renter_tier", u8"HẠNG THÀNH VIÊN HIỆN TẠI", tenHangHienTai_.c_str(),
             (mucUuDaiHienTai_ >= 10 ? u8"Đặc quyền thành viên VIP" : u8"Thành viên thân thiết"), GUI::Theme::Colors::Info);

    ImGui::SetCursorPos(ImVec2(startX, startY + 116.0f));

    // Card tiến trình nâng hạng
    Widgets::Card("card_renter_tier_progress", u8"Tiến trình nâng hạng thành viên kế tiếp", [this]() {
        if (mocKeTiep_ > 0) {
            ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::DarkText);
            ImGui::Text(u8"Tiến trình nâng lên hạng %s (Giảm %d%%):", tenHangKeTiep_.c_str(), phanTramKeTiep_);
            ImGui::PopStyleColor();
            ImGui::Spacing();

            char barText[128];
            std::snprintf(barText, sizeof(barText), u8"%d / %d lượt (Còn %d lượt đặt nữa)",
                          soLanDat_, mocKeTiep_, soLuotConLai_);

            Widgets::ProgressBar(tienDoKeTiep_, barText, ImVec2(-1.0f, 32.0f), GUI::Theme::Colors::GrassGreen);
        } else {
            ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::GrassGreen);
            ImGui::TextUnformatted(u8"Chúc mừng! Bạn đã đạt hạng thành viên cao nhất (VIP Vàng) của cụm sân.");
            ImGui::PopStyleColor();
            Widgets::ProgressBar(1.0f, u8"100% Hoàn thành", ImVec2(-1.0f, 32.0f), GUI::Theme::Colors::GoldAccent);
        }
    });
}

void UuDaiScreen::renderBangCacMoc() {
    Widgets::Card("card_renter_discount_tiers", u8"Bảng các mốc ưu đãi thành viên của cụm sân bóng", [this]() {
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
        ImGui::TextWrapped(u8"Hệ thống tự động tích lũy số lần đặt giữ chỗ trực tuyến và tự động chiết khấu trực tiếp trên mỗi đơn đặt sân khi đạt mốc quy định.");
        ImGui::PopStyleColor();
        ImGui::Spacing();

        if (ImGui::BeginTable("##tbl_renter_tier_rules", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn(u8"Hạng thành viên", ImGuiTableColumnFlags_WidthFixed, 220.0f);
            ImGui::TableSetupColumn(u8"Số lần đặt tối thiểu", ImGuiTableColumnFlags_WidthFixed, 180.0f);
            ImGui::TableSetupColumn(u8"Mức giảm giá", ImGuiTableColumnFlags_WidthFixed, 180.0f);
            ImGui::TableSetupColumn(u8"Tình trạng của bạn", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableHeadersRow();

            for (const auto& n : danhSachNguong_) {
                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                if (n.phanTramGiam >= 10) {
                    Widgets::BadgeVIP(n.tenHang.c_str());
                } else {
                    Widgets::Badge(n.tenHang.c_str(), GUI::Theme::Colors::GrassGreen, GUI::Theme::Colors::WhiteCard);
                }

                ImGui::TableSetColumnIndex(1);
                ImGui::Text(u8"Từ %d lần đặt", n.soLanToiThieu);

                ImGui::TableSetColumnIndex(2);
                ImGui::Text(u8"Giảm %d%% / mỗi giờ", n.phanTramGiam);

                ImGui::TableSetColumnIndex(3);
                if (soLanDat_ >= n.soLanToiThieu) {
                    if (mucUuDaiHienTai_ == n.phanTramGiam) {
                        Widgets::Badge(u8"Đang áp dụng", GUI::Theme::Colors::GoldAccent, GUI::Theme::Colors::WhiteCard);
                    } else {
                        Widgets::Badge(u8"Đã mở khóa", GUI::Theme::Colors::GrayHover, GUI::Theme::Colors::DarkText);
                    }
                } else {
                    ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
                    ImGui::Text(u8"Cần thêm %d lượt đặt nữa", n.soLanToiThieu - soLanDat_);
                    ImGui::PopStyleColor();
                }
            }

            ImGui::EndTable();
        }
    });
}

}  // namespace GUI::Screens::Renter
