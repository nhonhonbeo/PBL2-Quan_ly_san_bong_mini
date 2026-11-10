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
    // LÝ DO CHỌN: Chia thanh tiến độ thành 4 thanh nhỏ đại diện cho 4 bước,
    // loại bỏ hoàn toàn hiển thị % để giao diện trực quan và chuẩn hóa theo từng giai đoạn.
    float clampedRatio = std::clamp(tyLe, 0.0f, 1.0f);

    float width = kichThuoc.x;
    if (width <= 0.0f) {
        width = ImGui::GetContentRegionAvail().x;
    }
    float height = kichThuoc.y > 0.0f ? kichThuoc.y : 28.0f;

    ImVec2 p0 = ImGui::GetCursorScreenPos();
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    constexpr int NUM_SEGMENTS = 4;
    constexpr float SEGMENT_GAP = 6.0f;
    const float totalGaps = SEGMENT_GAP * static_cast<float>(NUM_SEGMENTS - 1);
    const float segW = std::max(4.0f, (width - totalGaps) / static_cast<float>(NUM_SEGMENTS));
    const float rounding = std::min(4.0f, height * 0.5f);

    const ImU32 bgCol = GUI::Theme::Colors::toU32(GUI::Theme::Colors::Border);
    const ImU32 fillCol = GUI::Theme::Colors::toU32(mauThanh);

    // 1. Vẽ 4 thanh nhỏ đại diện cho 4 bước
    for (int i = 0; i < NUM_SEGMENTS; ++i) {
        float x0 = p0.x + static_cast<float>(i) * (segW + SEGMENT_GAP);
        float x1 = x0 + segW;
        float y0 = p0.y;
        float y1 = p0.y + height;

        drawList->AddRectFilled(ImVec2(x0, y0), ImVec2(x1, y1), bgCol, rounding);

        float segStart = static_cast<float>(i) / static_cast<float>(NUM_SEGMENTS);
        float segEnd = static_cast<float>(i + 1) / static_cast<float>(NUM_SEGMENTS);

        if (clampedRatio >= segEnd - 0.001f) {
            drawList->AddRectFilled(ImVec2(x0, y0), ImVec2(x1, y1), fillCol, rounding);
        } else if (clampedRatio > segStart) {
            float fillPart = (clampedRatio - segStart) / (segEnd - segStart);
            drawList->AddRectFilled(ImVec2(x0, y0), ImVec2(x0 + segW * fillPart, y1), fillCol, rounding);
        }
    }

    // 2. Nhãn tiến trình tùy chọn (chỉ hiển thị nhãn chữ, không hiển thị %)
    if (nhanTuyChon != nullptr && nhanTuyChon[0] != '\0' && height >= 18.0f) {
        ImVec2 txtSize = ImGui::CalcTextSize(nhanTuyChon);
        float textOffsetX = std::max(0.0f, (width - txtSize.x) * 0.5f);
        ImVec2 txtPos(p0.x + textOffsetX, p0.y + (height - txtSize.y) * 0.5f);

        ImVec2 pad(6.0f, 2.0f);
        drawList->AddRectFilled(
            ImVec2(txtPos.x - pad.x, txtPos.y - pad.y),
            ImVec2(txtPos.x + txtSize.x + pad.x, txtPos.y + txtSize.y + pad.y),
            IM_COL32(255, 255, 255, 210),
            4.0f
        );

        drawList->AddText(txtPos, GUI::Theme::Colors::toU32(GUI::Theme::Colors::TextPrimary), nhanTuyChon);
    }

    ImGui::Dummy(ImVec2(width, height));
}

}  // namespace Widgets