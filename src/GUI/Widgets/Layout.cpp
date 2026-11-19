#include "GUI/Widgets/Layout.h"

#include <algorithm>

#include <imgui.h>

namespace Widgets {

void BeginNoiDungHep(const char* id, float beRongToiDa) {
    const float availableWidth = ImGui::GetContentRegionAvail().x;
    const float width = std::min(availableWidth, std::max(0.0f, beRongToiDa));
    const float offset = std::max(0.0f, (availableWidth - width) * 0.5f);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);
    ImGui::BeginChild(id, ImVec2(width, 0.0f), false);
}

void EndNoiDungHep() {
    ImGui::EndChild();
}

void BeginCot(const char* id, float beRong) {
    ImGui::BeginChild(id, ImVec2(std::max(0.0f, beRong), 0.0f), false);
}

void EndCot() {
    ImGui::EndChild();
}

}
