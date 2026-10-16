#include "GUI/Widgets/StarRating.h"

#include <cmath>
#include <string>
#include <imgui.h>

#include "GUI/Theme/Theme.h"

namespace Widgets {

namespace {

// LÝ DO CHỌN: Vẽ ngôi sao 5 cánh hoàn chỉnh bằng 1 ngũ giác tâm + 5 tam giác cánh,
// đảm bảo lấp đầy (fill) tuyệt đối chuẩn xác trên mọi GPU mà không lo lỗi đa giác lõm.
void VeNgoiSao(ImDrawList* drawList, ImVec2 center, float rOuter, ImU32 fillColor, ImU32 borderColor) {
    float rInner = rOuter * 0.42f;
    ImVec2 pts[10];
    const float pi = 3.14159265358979323846f;

    for (int i = 0; i < 10; ++i) {
        float angle = -pi * 0.5f + i * (pi / 5.0f);
        float r = (i % 2 == 0) ? rOuter : rInner;
        pts[i] = ImVec2(center.x + r * std::cos(angle), center.y + r * std::sin(angle));
    }

    // 1. Ngũ giác tâm (các đỉnh chẵn là đỉnh ngoài, đỉnh lẻ là đáy cánh)
    ImVec2 pentagon[5] = {pts[1], pts[3], pts[5], pts[7], pts[9]};
    drawList->AddConvexPolyFilled(pentagon, 5, fillColor);

    // 2. 5 tam giác cánh
    drawList->AddTriangleFilled(pts[0], pts[1], pts[9], fillColor);
    drawList->AddTriangleFilled(pts[2], pts[3], pts[1], fillColor);
    drawList->AddTriangleFilled(pts[4], pts[5], pts[3], fillColor);
    drawList->AddTriangleFilled(pts[6], pts[7], pts[5], fillColor);
    drawList->AddTriangleFilled(pts[8], pts[9], pts[7], fillColor);

    // 3. Đường viền bao quanh ngôi sao
    drawList->AddPolyline(pts, 10, borderColor, ImDrawFlags_Closed, 1.2f);
}

}  // namespace

bool StarRating(
    const char* label,
    const char* id,
    int& giaTri,
    bool chiXem,
    float kichThuocSao
) {
    ImGui::PushID(id);

    if (label != nullptr && label[0] != '\0') {
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::DarkText);
        ImGui::TextUnformatted(label);
        ImGui::PopStyleColor();
        ImGui::Spacing();
    }

    if (giaTri < 0) giaTri = 0;
    if (giaTri > 5) giaTri = 5;

    bool changed = false;
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 startPos = ImGui::GetCursorScreenPos();
    float spacing = 6.0f;
    float rOuter = kichThuocSao * 0.5f;

    ImU32 goldCol    = GUI::Theme::Colors::toU32(GUI::Theme::Colors::GoldAccent);
    ImU32 emptyCol   = GUI::Theme::Colors::toU32(ImVec4(0.92f, 0.93f, 0.94f, 1.0f));
    ImU32 borderCol  = GUI::Theme::Colors::toU32(GUI::Theme::Colors::Gray);
    ImU32 borderGold = GUI::Theme::Colors::toU32(ImVec4(0.85f, 0.60f, 0.00f, 1.0f));

    for (int i = 1; i <= 5; ++i) {
        ImVec2 starCenter(startPos.x + (i - 1) * (kichThuocSao + spacing) + rOuter, startPos.y + rOuter);
        ImVec2 starMin(starCenter.x - rOuter, starCenter.y - rOuter);

        std::string btnId = "##star_" + std::to_string(i);
        ImGui::SetCursorScreenPos(starMin);

        bool hovered = false;
        if (!chiXem) {
            if (ImGui::InvisibleButton(btnId.c_str(), ImVec2(kichThuocSao, kichThuocSao))) {
                giaTri = i;
                changed = true;
            }
            hovered = ImGui::IsItemHovered();
        } else {
            ImGui::Dummy(ImVec2(kichThuocSao, kichThuocSao));
        }

        bool isActive = (i <= giaTri);
        ImU32 curFill = (isActive || hovered) ? goldCol : emptyCol;
        ImU32 curBorder = (isActive || hovered) ? borderGold : borderCol;

        VeNgoiSao(drawList, starCenter, rOuter, curFill, curBorder);
    }

    // Hiển thị text điểm đánh giá
    ImGui::SetCursorScreenPos(ImVec2(startPos.x + 5 * (kichThuocSao + spacing) + 8.0f, startPos.y + 2.0f));
    std::string textGiaTri = std::to_string(giaTri) + "/5 sao";
    ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::DarkText);
    ImGui::TextUnformatted(textGiaTri.c_str());
    ImGui::PopStyleColor();

    ImGui::Dummy(ImVec2(0.0f, 4.0f));
    ImGui::PopID();
    return changed;
}

}  // namespace Widgets
