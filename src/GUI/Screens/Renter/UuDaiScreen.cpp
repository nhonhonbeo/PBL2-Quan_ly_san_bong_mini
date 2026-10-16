#include "GUI/Screens/Renter/UuDaiScreen.h"

#include <cstdio>
#include <string>
#include <vector>
#include <imgui.h>

#include "GUI/App/Session.h"
#include "GUI/Theme/Theme.h"
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
    Widgets::Card("card_renter_discount_stats", u8"Quyền lợi & Tiến trình nâng hạng thành viên", [this]() {
        ImGui::Columns(3, nullptr, false);

        // 1. Số lần đã đặt
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
        ImGui::TextUnformatted(u8"Số lần đã đặt thành công:");
        ImGui::PopStyleColor();
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::DarkText);
        ImGui::Text(u8"⚽ %d lần", soLanDat_);
        ImGui::PopStyleColor();

        ImGui::NextColumn();

        // 2. Mức ưu đãi hiện có
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
        ImGui::TextUnformatted(u8"Mức giảm giá hiện tại:");
        ImGui::PopStyleColor();
        ImGui::Spacing();
        if (mucUuDaiHienTai_ > 0) {
            Widgets::BadgeVIP((u8"Giảm " + std::to_string(mucUuDaiHienTai_) + "% / giờ").c_str());
        } else {
            Widgets::Badge(u8"0% (Chưa có ưu đãi)", GUI::Theme::Colors::GrayHover, GUI::Theme::Colors::DarkText);
        }

        ImGui::NextColumn();

        // 3. Hạng thành viên
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
        ImGui::TextUnformatted(u8"Hạng thành viên hiện tại:");
        ImGui::PopStyleColor();
        ImGui::Spacing();
        if (mucUuDaiHienTai_ >= 10) {
            Widgets::BadgeVIP(tenHangHienTai_.c_str());
        } else if (mucUuDaiHienTai_ > 0) {
            Widgets::Badge(tenHangHienTai_.c_str(), GUI::Theme::Colors::GrassGreen, GUI::Theme::Colors::WhiteCard);
        } else {
            Widgets::Badge(tenHangHienTai_.c_str(), GUI::Theme::Colors::GrayHover, GUI::Theme::Colors::DarkText);
        }

        ImGui::Columns(1);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Tiến trình nâng hạng
        if (mocKeTiep_ > 0) {
            ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::DarkText);
            ImGui::Text(u8"Tiến trình nâng lên hạng %s (Giảm %d%%):", tenHangKeTiep_.c_str(), phanTramKeTiep_);
            ImGui::PopStyleColor();
            ImGui::Spacing();

            char barText[128];
            std::snprintf(barText, sizeof(barText), u8"%d / %d lượt (Còn %d lượt đặt nữa)",
                          soLanDat_, mocKeTiep_, soLuotConLai_);

            Widgets::ProgressBar(tienDoKeTiep_, barText, ImVec2(-1.0f, 22.0f), GUI::Theme::Colors::GrassGreen);
        } else {
            ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::GrassGreen);
            ImGui::TextUnformatted(u8"🎉 Chúc mừng! Bạn đã đạt hạng thành viên cao nhất (VIP Vàng) của cụm sân.");
            ImGui::PopStyleColor();
            Widgets::ProgressBar(1.0f, u8"100% Hoàn thành", ImVec2(-1.0f, 22.0f), GUI::Theme::Colors::GoldAccent);
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
                        Widgets::Badge(u8"✓ Đang áp dụng", GUI::Theme::Colors::GoldAccent, GUI::Theme::Colors::WhiteCard);
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
