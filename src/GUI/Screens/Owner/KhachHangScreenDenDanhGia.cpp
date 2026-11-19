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
#include "GUI/Widgets/InputFields.h"
#include "GUI/Widgets/Modal.h"
#include "GUI/Widgets/ProgressBar.h"
#include "GUI/Widgets/StarRating.h"
#include "GUI/Widgets/Toast.h"

namespace GUI::Screens::Owner {

namespace {

// Hàm che một phần số điện thoại để hiển thị công khai (ví dụ: 090***5678)
std::string maHoaSDT(const std::string& sdt) {
    if (sdt.length() >= 7) {
        return sdt.substr(0, 3) + "***" + sdt.substr(sdt.length() - 4);
    }
    return sdt;
}

}  // namespace

void KhachHangScreen::renderTabDanhGia() {
    // Khối 1: Thống kê điểm trung bình toàn cơ sở & từng sân
    Widgets::Card("card_tab_review_stats", u8"Thống kê điểm đánh giá chất lượng dịch vụ", [this]() {
        float availW = ImGui::GetContentRegionAvail().x;
        float leftW = 260.0f;
        float rightW = availW - leftW - 20.0f;

        // Cột trái: Điểm trung bình toàn cơ sở
        ImGui::BeginGroup();
        ImGui::PushItemWidth(leftW);

        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
        ImGui::TextUnformatted(u8"Điểm trung bình toàn cụm sân:");
        ImGui::PopStyleColor();

        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::GoldAccent);
        if (GUI::Theme::getFonts().heading) ImGui::PushFont(GUI::Theme::getFonts().heading);
        char scoreBuf[32];
        std::snprintf(scoreBuf, sizeof(scoreBuf), "%.1f / 5.0", diemTBToanCoSo_);
        ImGui::TextUnformatted(scoreBuf);
        if (GUI::Theme::getFonts().heading) ImGui::PopFont();
        ImGui::PopStyleColor();

        int starScore = static_cast<int>(diemTBToanCoSo_ + 0.5f);
        Widgets::StarRating(nullptr, "overall_stars", starScore, true, 22.0f);

        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
        ImGui::Text(u8"Tổng số: %zu đánh giá", danhSachDanhGia_.size());
        ImGui::PopStyleColor();

        ImGui::PopItemWidth();
        ImGui::EndGroup();

        ImGui::SameLine(0.0f, 20.0f);

        // Cột phải: Điểm trung bình theo từng sân bằng thanh ngang ProgressBar có số
        ImGui::BeginGroup();
        ImGui::PushItemWidth(rightW);

        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::DarkText);
        ImGui::TextUnformatted(u8"Điểm đánh giá trung bình theo từng sân bóng:");
        ImGui::PopStyleColor();
        ImGui::Spacing();

        for (const auto& item : diemTBTungSan_) {
            ImGui::Text(u8"Sân %s:", item.first.c_str());
            ImGui::SameLine(90.0f);

            float ratio = item.second / 5.0f;
            char barLabel[32];
            std::snprintf(barLabel, sizeof(barLabel), "%.1f / 5.0", item.second);

            Widgets::ProgressBar(ratio, barLabel, ImVec2(rightW - 100.0f, 30.0f), GUI::Theme::Colors::GoldAccent);
            ImGui::Spacing();
        }

        ImGui::PopItemWidth();
        ImGui::EndGroup();
    });

    ImGui::Spacing();

    // Khối 2: Bộ lọc theo số sao & Danh sách đánh giá nhận xét
    Widgets::Card("card_tab_review_list", u8"Danh sách đánh giá & Nhận xét của khách hàng", [this]() {
        // Bộ lọc số sao
        ImGui::TextUnformatted(u8"Lọc theo số sao:");
        ImGui::SameLine();
        const char* filterLabels[] = {
            u8"Tất cả số sao",
            u8"5 sao",
            u8"4 sao",
            u8"3 sao",
            u8"2 sao",
            u8"1 sao"
        };
        ImGui::SetNextItemWidth(180.0f);
        if (ImGui::BeginCombo("##combo_filter_stars", filterLabels[locSoSao_])) {
            for (int i = 0; i < 6; ++i) {
                bool isSelected = (locSoSao_ == i);
                if (ImGui::Selectable(filterLabels[i], isSelected)) {
                    locSoSao_ = i;
                }
            }
            ImGui::EndCombo();
        }
        ImGui::SameLine(0.0f, 20.0f);
        ImGui::TextUnformatted(u8"Lọc theo sân:");
        ImGui::SameLine();
        std::string selectedFieldLabel = locSanIndex_ == 0
            ? u8"Tất cả sân"
            : danhSachSan_[static_cast<size_t>(locSanIndex_ - 1)].maSan + " - " +
                  danhSachSan_[static_cast<size_t>(locSanIndex_ - 1)].tenSan;
        ImGui::SetNextItemWidth(280.0f);
        if (ImGui::BeginCombo("##combo_filter_field", selectedFieldLabel.c_str())) {
            if (ImGui::Selectable(u8"Tất cả sân", locSanIndex_ == 0)) {
                locSanIndex_ = 0;
            }
            for (size_t i = 0; i < danhSachSan_.size(); ++i) {
                const auto& san = danhSachSan_[i];
                std::string fieldLabel = san.maSan + " - " + san.tenSan;
                bool isSelected = locSanIndex_ == static_cast<int>(i + 1);
                if (ImGui::Selectable(fieldLabel.c_str(), isSelected)) {
                    locSanIndex_ = static_cast<int>(i + 1);
                }
            }
            ImGui::EndCombo();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Danh sách đánh giá
        bool coDanhGia = false;
        for (const auto& dg : danhSachDanhGia_) {
            if (locSanIndex_ > 0 &&
                dg.maSan != danhSachSan_[static_cast<size_t>(locSanIndex_ - 1)].maSan) {
                continue;
            }
            // Lọc số sao: nếu locSoSao_ > 0 thì so sánh (6 - locSoSao_)
            if (locSoSao_ > 0) {
                int expectedStar = (locSoSao_ == 1) ? 5 : ((locSoSao_ == 2) ? 4 : ((locSoSao_ == 3) ? 3 : ((locSoSao_ == 4) ? 2 : 1)));
                if (dg.soSao != expectedStar) {
                    continue;
                }
            }

            coDanhGia = true;

            // Khối thẻ nhận xét
            ImGui::BeginGroup();

            ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::DarkText);
            ImGui::Text("%s (%s)", dg.tenKhach.c_str(), maHoaSDT(dg.sdt).c_str());
            ImGui::PopStyleColor();

            ImGui::SameLine();
            Widgets::BadgeTrong((u8"Sân " + dg.maSan).c_str());

            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
            ImGui::Text(u8"| Ngày: %s", dg.ngayGui.c_str());
            ImGui::PopStyleColor();

            // Vẽ số sao
            Widgets::StarRating(nullptr, ("dg_star_" + dg.id).c_str(), const_cast<int&>(dg.soSao), true, 16.0f);

            // Nội dung nhận xét
            ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::DarkText);
            ImGui::TextWrapped(u8"\"%s\"", dg.noiDung.c_str());
            ImGui::PopStyleColor();

            ImGui::EndGroup();

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
        }

        if (!coDanhGia) {
            ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
            ImGui::TextUnformatted(u8"Không có đánh giá nào phù hợp với bộ lọc đã chọn.");
            ImGui::PopStyleColor();
        }
    });
}

}  // namespace GUI::Screens::Owner
