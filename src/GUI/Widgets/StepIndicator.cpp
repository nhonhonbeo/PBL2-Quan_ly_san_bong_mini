#include "GUI/Widgets/StepIndicator.h"

#include <algorithm>
#include <cstddef>
#include <string>

#include <imgui.h>

namespace Widgets {

void ThanhCacBuoc(const std::vector<std::string>& nhanCacBuoc,
                  int buocHienTai) {
    if (nhanCacBuoc.empty()) return;

    const ImVec2 start = ImGui::GetCursorScreenPos();
    const float width = ImGui::GetContentRegionAvail().x;
    const float segmentWidth = width / static_cast<float>(nhanCacBuoc.size());
    const float centerY = start.y + 12.0f;
    const float radius = 12.0f;
    const int current = std::clamp(buocHienTai, 1,
                                   static_cast<int>(nhanCacBuoc.size()));
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    for (std::size_t i = 0; i < nhanCacBuoc.size(); ++i) {
        const float centerX = start.x + segmentWidth *
            (static_cast<float>(i) + 0.5f);
        const int step = static_cast<int>(i) + 1;
        const bool completed = step < current;
        const bool active = step == current;
        const ImU32 color = completed ? IM_COL32(22, 134, 91, 255)
                           : active ? IM_COL32(43, 123, 92, 255)
                                    : IM_COL32(160, 170, 175, 255);

        if (i > 0) {
            const float previousCenterX = start.x + segmentWidth *
                (static_cast<float>(i) - 0.5f);
            drawList->AddLine(ImVec2(previousCenterX + radius, centerY),
                              ImVec2(centerX - radius, centerY),
                              completed ? IM_COL32(22, 134, 91, 255)
                                        : IM_COL32(190, 198, 202, 255),
                              2.0f);
        }

        drawList->AddCircleFilled(ImVec2(centerX, centerY), radius, color);
        if (completed) {
            drawList->AddLine(ImVec2(centerX - 5.0f, centerY),
                              ImVec2(centerX - 1.0f, centerY + 4.0f),
                              IM_COL32(255, 255, 255, 255), 2.0f);
            drawList->AddLine(ImVec2(centerX - 1.0f, centerY + 4.0f),
                              ImVec2(centerX + 6.0f, centerY - 5.0f),
                              IM_COL32(255, 255, 255, 255), 2.0f);
        } else {
            const std::string number = std::to_string(step);
            const ImVec2 numberSize = ImGui::CalcTextSize(number.c_str());
            drawList->AddText(
                ImVec2(centerX - numberSize.x * 0.5f,
                       centerY - numberSize.y * 0.5f),
                IM_COL32(255, 255, 255, 255), number.c_str());
        }

        const ImVec2 labelSize = ImGui::CalcTextSize(nhanCacBuoc[i].c_str());
        drawList->AddText(
            ImVec2(centerX - labelSize.x * 0.5f, centerY + radius + 5.0f),
            active ? IM_COL32(43, 123, 92, 255)
                   : IM_COL32(90, 100, 105, 255),
            nhanCacBuoc[i].c_str());
    }

    ImGui::Dummy(ImVec2(width, radius * 2.0f + 25.0f));
}

}
