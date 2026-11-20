#include "GUI/Screens/Renter/DatSanScreen.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <string>
#include <utility>

#include <imgui.h>

#include "Core/DateTime.h"
#include "GUI/App/Facade.h"
#include "GUI/App/Session.h"
#include "GUI/Theme/Fonts.h"
#include "GUI/Theme/Theme.h"
#include "GUI/Widgets/Badge.h"
#include "GUI/Widgets/Button.h"
#include "GUI/Widgets/Card.h"
#include "GUI/Widgets/DatePicker.h"
#include "GUI/Widgets/ProgressBar.h"
#include "GUI/Widgets/Toast.h"
#include "features/Account/User.h"
#include "GUI/App/ViewFormat.h"

namespace GUI::Screens::Renter {

using namespace GUI::App::ViewFormat;

void DatSanScreen::renderBuoc1ChonCoSo() {
    Widgets::Card("choose_facility", u8"Ch\u1ECDn c\u1EE5m s\u00E2n th\u1EC3 thao", [this]() {
        ImGui::PushStyleColor(ImGuiCol_FrameBg, GUI::Theme::Colors::Surface);
        ImGui::PushStyleColor(ImGuiCol_Border, GUI::Theme::Colors::InputBorder);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.5f);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 7.0f));

        ImGui::AlignTextToFramePadding();
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::Primary);
        ImDrawList* searchDl = ImGui::GetWindowDrawList(); ImVec2 searchIconPos = ImGui::GetCursorScreenPos(); ImU32 searchIconCol = GUI::Theme::Colors::toU32(GUI::Theme::Colors::Primary); searchDl->AddCircle(ImVec2(searchIconPos.x + 10.0f, searchIconPos.y + 12.0f), 5.5f, searchIconCol, 16, 1.8f); searchDl->AddLine(ImVec2(searchIconPos.x + 14.0f, searchIconPos.y + 16.0f), ImVec2(searchIconPos.x + 19.0f, searchIconPos.y + 21.0f), searchIconCol, 2.0f); ImGui::Dummy(ImVec2(22.0f, 24.0f));
        ImGui::PopStyleColor();
        ImGui::SameLine(0.0f, 8.0f);

        char search[256]{};
        const std::size_t count = tuKhoaCoSo_.size() < sizeof(search) - 1 ? tuKhoaCoSo_.size() : sizeof(search) - 1;
        std::memcpy(search, tuKhoaCoSo_.data(), count);
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x < 520.0f ? ImGui::GetContentRegionAvail().x : 380.0f);
        if (ImGui::InputTextWithHint("##search_facility", u8"T\u00ECm ki\u1EBFm c\u1EE5m s\u00E2n theo t\u00EAn ho\u1EB7c \u0111\u1ECBa ch\u1EC9...", search, sizeof(search))) {
            tuKhoaCoSo_ = search;
        }

        if (!tuKhoaCoSo_.empty()) {
            if (ImGui::GetContentRegionAvail().x >= 140.0f) ImGui::SameLine(0.0f, 8.0f);
            if (ImGui::Button(u8"\u2715 X\u00F3a l\u1ECDc##clear_facility_search")) {
                tuKhoaCoSo_.clear();
            }
        }

        ImGui::PopStyleVar(3);
        ImGui::PopStyleColor(2);

        int matchingCount = 0;
        for (auto* node = danhSachCoSo_.get_head(); node != nullptr; node = node->next) {
            const VenueFacilityRecord& facility = node->data;
            if (!tuKhoaCoSo_.empty() &&
                facility.name.find(tuKhoaCoSo_) == std::string::npos &&
                facility.address.find(tuKhoaCoSo_) == std::string::npos) {
                continue;
            }
            ++matchingCount;
        }

        ImGui::AlignTextToFramePadding();
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::TextSecondary);
        ImGui::Text(u8"Hi\u1EC3n th\u1ECB %d c\u1EE5m s\u00E2n s\u1EB5n c\u00F3", matchingCount);
        ImGui::PopStyleColor();

        ImGui::PushStyleColor(ImGuiCol_Separator, GUI::Theme::Colors::Border);
        ImGui::Separator();
        ImGui::PopStyleColor();

        if (matchingCount == 0) {
            ImGui::Spacing();
            ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::TextSecondary);
            ImGui::TextUnformatted(u8"Kh\u00F4ng t\u00ECm th\u1EA5y c\u1EE5m s\u00E2n n\u00E0o ph\u00F9 h\u1EE3p v\u1EDBi t\u1EEB kh\u00F3a t\u00ECm ki\u1EBFm. Vui l\u00F2ng th\u1EED l\u1EA1i!");
            ImGui::PopStyleColor();
            ImGui::Spacing();
            return;
        }

        float availableWidth = ImGui::GetContentRegionAvail().x - 18.0f;
        int numCols = static_cast<int>(availableWidth / 320.0f);
        if (numCols < 1) numCols = 1;
        if (numCols > 3) numCols = 3;
        numCols = std::min(numCols, matchingCount);

        if (ImGui::BeginTable("##cum_san_grid_table", numCols, ImGuiTableFlags_SizingStretchSame, ImVec2(availableWidth,0))) {
            // Active facilities first; keep original relative order in each group.
            for (int pass = 0; pass < 2; ++pass) {
                for (auto* node = danhSachCoSo_.get_head(); node != nullptr; node = node->next) {
                    const VenueFacilityRecord& facility = node->data;
                    if ((soSanTheoCoSo_[facility.ownerId] > 0) != (pass == 0)) continue;
                    if (!tuKhoaCoSo_.empty() &&
                        facility.name.find(tuKhoaCoSo_) == std::string::npos &&
                        facility.address.find(tuKhoaCoSo_) == std::string::npos) {
                        continue;
                    }

                    ImGui::TableNextColumn();
                    renderTheCumSan(facility, soSanTheoCoSo_[facility.ownerId], giaThapNhatTheoCoSo_[facility.ownerId]);
                }
            }
            ImGui::EndTable();
        }
    });
}

void DatSanScreen::renderTheCumSan(const VenueFacilityRecord& facility, int fieldCount, long long lowestPrice) {
    ImGui::PushID(facility.ownerId.c_str());
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(14.0f, 12.0f));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, GUI::Theme::Colors::Surface);
    ImGui::PushStyleColor(ImGuiCol_Border, GUI::Theme::Colors::Border);

    std::string childId = "##card_" + facility.ownerId;
    if (ImGui::BeginChild(childId.c_str(), ImVec2(0.0f, 310.0f), true, ImGuiWindowFlags_None)) {
        ImDrawList* drawList = ImGui::GetWindowDrawList();

        ImVec2 bannerP0 = ImGui::GetCursorScreenPos();
        float bannerWidth = ImGui::GetContentRegionAvail().x;
        float bannerHeight = 60.0f;
        ImVec2 bannerP1(bannerP0.x + bannerWidth, bannerP0.y + bannerHeight);

        drawList->AddRectFilledMultiColor(bannerP0, bannerP1, IM_COL32(15, 95, 63, 255), IM_COL32(35, 143, 85, 255), IM_COL32(23, 116, 70, 255), IM_COL32(10, 83, 54, 255));

        float midX = bannerP0.x + bannerWidth * 0.5f;
        float midY = bannerP0.y + bannerHeight * 0.5f;
        drawList->AddLine(ImVec2(midX, bannerP0.y), ImVec2(midX, bannerP1.y), IM_COL32(255, 255, 255, 80), 1.2f);
        drawList->AddCircle(ImVec2(midX, midY), 13.0f, IM_COL32(255, 255, 255, 80), 24, 1.2f);
        drawList->AddRect(bannerP0, bannerP1, IM_COL32(255, 255, 255, 60), 4.0f, 0, 1.0f);

        drawList->AddRect(ImVec2(bannerP0.x+10,bannerP0.y+8), ImVec2(bannerP1.x-10,bannerP1.y-8), IM_COL32(255,255,255,120), 2);
        drawList->AddRect(ImVec2(bannerP0.x+10,midY-18),ImVec2(bannerP0.x+34,midY+18),IM_COL32(255,255,255,120));
        drawList->AddRect(ImVec2(bannerP1.x-34,midY-18),ImVec2(bannerP1.x-10,midY+18),IM_COL32(255,255,255,120));

        ImGui::SetCursorScreenPos(ImVec2(bannerP0.x, bannerP1.y + 8.0f));
        ImFont* titleFont = GUI::Theme::getFonts().title;
        if (titleFont != nullptr) ImGui::PushFont(titleFont);
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::TextPrimary);
        const auto oneLine = [](const std::string& value) {
            const auto p = ImGui::GetCursorScreenPos();
            const float width = ImGui::GetContentRegionAvail().x;
            const float height = ImGui::GetTextLineHeight();
            auto* dl = ImGui::GetWindowDrawList();
            dl->PushClipRect(p, ImVec2(p.x+width,p.y+height), true);
            dl->AddText(p, ImGui::GetColorU32(ImGuiCol_Text), value.c_str());
            dl->PopClipRect();
            ImGui::Dummy(ImVec2(width,height));
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s",value.c_str());
        };
        oneLine(facility.name);
        ImGui::PopStyleColor();
        if (titleFont != nullptr) ImGui::PopFont();

        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::TextSecondary);
        oneLine(facility.address);

        if (!facility.openTime.empty() && !facility.closeTime.empty())
            ImGui::Text(u8"• Giờ mở cửa: %s - %s", facility.openTime.c_str(), facility.closeTime.c_str());
        else ImGui::TextUnformatted(u8"• Giờ mở cửa: Chưa cập nhật");
        if (!facility.phone.empty()) ImGui::Text(u8"• Liên hệ: %s", facility.phone.c_str());
        ImGui::PopStyleColor();

        ImGui::PushStyleColor(ImGuiCol_Separator, GUI::Theme::Colors::Border);
        ImGui::Separator();
        ImGui::PopStyleColor();

        if (fieldCount == 0) {
            Widgets::BadgeBaoTri(u8"Chưa có sân sẵn sàng");
            ImGui::TextDisabled(u8"Chưa có giá thuê");
        } else {
            Widgets::BadgeTrong((std::to_string(fieldCount) + u8" s\u00E2n s\u1EB5n s\u00E0ng").c_str());
            ImGui::TextColored(GUI::Theme::Colors::PrimaryDark, u8"Giá từ %s", dinhDangTien(lowestPrice).c_str());
        }

        ImGui::Spacing();

        const float bottom = ImGui::GetWindowHeight() - 56.0f;
        ImGui::SetCursorPosY(std::max(ImGui::GetCursorPosY(), bottom));
        if (fieldCount == 0) {
            ImGui::BeginDisabled(true);
            Widgets::NutPhu(u8"T\u1EA1m ng\u01B0ng \u0111\u00F3n kh\u00E1ch", ImVec2(ImGui::GetContentRegionAvail().x, 42.0f));
            ImGui::EndDisabled();
        } else {
            if (Widgets::NutChinh(u8"Chọn cụm sân", ImVec2(ImGui::GetContentRegionAvail().x, 42.0f))) {
                chonCoSo(facility);
            }
        }
    }
    ImGui::EndChild();

    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(2);
    ImGui::PopID();
}

void DatSanScreen::renderBuoc2ChonSan() {
    Widgets::Card("choose_field", u8"Bước 2: Chọn loại sân và sân bóng cụ thể", [this]() {
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::TextPrimary);
        ImGui::Text(u8"Cơ sở đang chọn: %s", tenCoSoChon_.c_str());
        ImGui::PopStyleColor();

        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::TextSecondary);
        ImGui::Text(u8"Địa chỉ: %s", diaChiCoSoChon_.c_str());
        ImGui::PopStyleColor();

        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Separator, GUI::Theme::Colors::Border);
        ImGui::Separator();
        ImGui::PopStyleColor();
        ImGui::Spacing();

        ImGui::TextUnformatted(u8"Chọn loại sân bóng:");
        ImGui::Spacing();

        for (const int size : {5, 7, 11}) {
            if (!loaiSanCoSan_[size]) continue;

            bool isSelectedSize = (loaiSanChon_ == size);
            if (isSelectedSize) {
                ImGui::PushStyleColor(ImGuiCol_Button,        GUI::Theme::Colors::PrimaryLight);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, GUI::Theme::Colors::PrimaryLight);
                ImGui::PushStyleColor(ImGuiCol_Text,          GUI::Theme::Colors::PrimaryDark);
                ImGui::PushStyleColor(ImGuiCol_Border,        GUI::Theme::Colors::Primary);
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button,        GUI::Theme::Colors::Surface);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, GUI::Theme::Colors::GrayHover);
                ImGui::PushStyleColor(ImGuiCol_Text,          GUI::Theme::Colors::TextPrimary);
                ImGui::PushStyleColor(ImGuiCol_Border,        GUI::Theme::Colors::InputBorder);
            }
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, isSelectedSize ? 2.0f : 1.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);

            std::string btnSizeLabel = u8"Sân " + std::to_string(size) + u8" người";
            if (ImGui::Button(btnSizeLabel.c_str(), ImVec2(160.0f, 38.0f))) {
                loaiSanChon_ = size;
                maSanChon_.clear();
                tenSanChon_.clear();
            }
            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor(4);

            ImGui::SameLine(0.0f, 10.0f);
        }

        ImGui::NewLine();
        ImGui::Spacing();
        ImGui::TextUnformatted(u8"Danh sách sân bóng thuộc loại đã chọn:");
        ImGui::Spacing();

        int shown = 0;
        for (auto* node = danhSachSan_.get_head(); node != nullptr;
             node = node->next) {
            const VenueFieldRecord& field = node->data;
            if (field.status != 0 || field.size != loaiSanChon_) continue;
            ++shown;
            ImGui::PushID(field.fieldId.c_str());
            const bool selected = (maSanChon_ == field.fieldId);

            if (selected) {
                ImGui::PushStyleColor(ImGuiCol_Button,        GUI::Theme::Colors::Primary);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, GUI::Theme::Colors::PrimaryHover);
                ImGui::PushStyleColor(ImGuiCol_Text,          GUI::Theme::Colors::Surface);
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button,        GUI::Theme::Colors::Surface);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, GUI::Theme::Colors::PrimaryLight);
                ImGui::PushStyleColor(ImGuiCol_Text,          GUI::Theme::Colors::TextPrimary);
            }
            ImGui::PushStyleColor(ImGuiCol_Border, selected ? GUI::Theme::Colors::Primary : GUI::Theme::Colors::InputBorder);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);

            std::string fieldBtnLabel = field.name + " (" + field.fieldId + ")";
            if (ImGui::Button(fieldBtnLabel.c_str(), ImVec2(240.0f, 40.0f))) {
                maSanChon_ = field.fieldId;
                tenSanChon_ = field.name;
                loaiSanChon_ = field.size;
            }

            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor(4);
            ImGui::PopID();
            ImGui::SameLine(0.0f, 10.0f);
        }

        ImGui::NewLine();
        if (shown == 0) {
            ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::TextSecondary);
            ImGui::TextUnformatted(u8"Loại sân này hiện chưa có sân nào trống sẵn sàng phục vụ.");
            ImGui::PopStyleColor();
        }

        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Separator, GUI::Theme::Colors::Border);
        ImGui::Separator();
        ImGui::PopStyleColor();
        ImGui::Spacing();

        if (!maSanChon_.empty()) {
            ImGui::Spacing();
            std::string imgPath = GUI::App::Facade::getInstance().getAnhSan(maSanChon_);
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.96f, 0.98f, 0.96f, 1.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
            if (ImGui::BeginChild("##field_photo_preview_box", ImVec2(0.0f, 66.0f), true)) {
                ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::PrimaryDark);
                ImGui::Text(u8"Thông tin sân đã chọn: %s (%s)", tenSanChon_.c_str(), maSanChon_.c_str());
                ImGui::PopStyleColor();

                if (!imgPath.empty()) {
                    ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::Success);
                    ImGui::Text(u8"Ảnh thực tế: %s (Mặt sân cỏ đạt chuẩn chất lượng)", imgPath.c_str());
                    ImGui::PopStyleColor();
                } else {
                    ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::TextSecondary);
                    ImGui::TextUnformatted(u8"Chưa có ảnh thực tế riêng (áp dụng mặt cỏ tiêu chuẩn của cụm sân).");
                    ImGui::PopStyleColor();
                }
            }
            ImGui::EndChild();
            ImGui::PopStyleVar();
            ImGui::PopStyleColor();
        }

        ImGui::Spacing();

        if (Widgets::NutPhu(u8"Quay lại chọn cơ sở", ImVec2(180.0f, 38.0f))) {
            buocHienTai_ = 1;
        }

        if (!maSanChon_.empty()) {
            ImGui::SameLine(0.0f, 12.0f);
            if (Widgets::NutChinh(u8"Tiếp tục chọn ngày và ca đá", ImVec2(260.0f, 38.0f))) {
                buocHienTai_ = 3;
                reloadSlots();
            }
        }
    });
}

}  // namespace GUI::Screens::Renter
