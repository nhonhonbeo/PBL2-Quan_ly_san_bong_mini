#include "GUI/Widgets/ProgressBar.h"

#include <algorithm>
#include <cstdio>
#include <string>

namespace Widgets {

void ProgressBar(
    float tyLe,
    const char* nhanTuyChon,
    const ImVec2& kichThuoc,
    const ImVec4& mauThanh
) {
    float clampedRatio = std::clamp(tyLe, 0.0f, 1.0f);

    float width = kichThuoc.x;
    if (width <= 0.0f) {
        width = ImGui::GetContentRegionAvail().x;
    }
    float height = kichThuoc.y;

    ImVec2 p0 = ImGui::GetCursorScreenPos();
    ImVec2 p1 = ImVec2(p0.x + width, p0.y + height);

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    float rounding = height * 0.4f;

    // 1. Nền rãnh thanh tiến trình
    ImU32 bgCol = GUI::Theme::Colors::toU32(ImVec4(0.92f, 0.94f, 0.93f, 1.0f));
    drawList->AddRectFilled(p0, p1, bgCol, rounding);

    // 2. Thanh tiến trình đã nạp
    if (clampedRatio > 0.005f) {
        float fillW = width * clampedRatio;
        ImVec2 fillP1 = ImVec2(p0.x + fillW, p1.y);
        ImU32 fillCol = GUI::Theme::Colors::toU32(mauThanh);
        drawList->AddRectFilled(p0, fillP1, fillCol, rounding);
    }

    // 3. Nhãn phần trăm ở giữa thanh
    char buffer[64];
    if (nhanTuyChon != nullptr && nhanTuyChon[0] != '\0') {
        std::snprintf(buffer, sizeof(buffer), "%s (%.0f%%)", nhanTuyChon, clampedRatio * 100.0f);
    } else {
        std::snprintf(buffer, sizeof(buffer), "%.0f%%", clampedRatio * 100.0f);
    }

    ImVec2 txtSize = ImGui::CalcTextSize(buffer);
    ImVec2 txtPos(p0.x + (width - txtSize.x) * 0.5f, p0.y + (height - txtSize.y) * 0.5f);

    // Chữ màu trắng nếu thanh đã đầy qua vị trí chữ, hoặc chữ màu đậm nếu nằm ngoài
    ImU32 txtCol = (clampedRatio > 0.55f)
        ? IM_COL32_WHITE
        : GUI::Theme::Colors::toU32(GUI::Theme::Colors::DarkText);

    drawList->AddText(txtPos, txtCol, buffer);

    ImGui::Dummy(ImVec2(width, height));
}

}  // namespace Widgets
