#include "GUI/Widgets/Modal.h"

#include <imgui.h>
#include "GUI/Theme/Theme.h"
#include "GUI/Widgets/Button.h"

namespace Widgets {

void MoModalXacNhan(const char* tieuDe) {
    ImGui::OpenPopup(tieuDe);
}

bool ModalXacNhan(
    const char* tieuDe,
    const char* noiDung,
    const std::function<void()>& onDongY,
    const char* labelDongY,
    const char* labelHuy,
    bool laNguyHiem
) {
    bool daDongY = false;

    // Căn giữa màn hình hiển thị
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(440.0f, 0.0f), ImGuiCond_Appearing);

    ImGuiWindowFlags flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;

    if (ImGui::BeginPopupModal(tieuDe, nullptr, flags)) {
        ImGui::Spacing();

        // Biểu tượng cảnh báo hoặc thông báo
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 iconPos = ImGui::GetCursorScreenPos();
        float radius = 14.0f;
        ImVec2 centerIcon(iconPos.x + radius + 4.0f, iconPos.y + radius);

        ImU32 badgeCol = laNguyHiem
            ? GUI::Theme::Colors::toU32(GUI::Theme::Colors::RedDanger)
            : GUI::Theme::Colors::toU32(GUI::Theme::Colors::GoldAccent);

        drawList->AddCircleFilled(centerIcon, radius, badgeCol);
        drawList->AddText(ImVec2(centerIcon.x - 4.0f, centerIcon.y - 10.0f), IM_COL32_WHITE, "!");

        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + radius * 2.0f + 16.0f);
        ImGui::PushTextWrapPos(ImGui::GetWindowWidth() - 32.0f);
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::DarkText);
        ImGui::TextUnformatted(noiDung);
        ImGui::PopStyleColor();
        ImGui::PopTextWrapPos();

        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Căn nút bấm sang bên phải
        float buttonWidth = 110.0f;
        float totalButtonsWidth = buttonWidth * 2.0f + ImGui::GetStyle().ItemSpacing.x;
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - totalButtonsWidth - ImGui::GetStyle().WindowPadding.x);

        if (NutPhu(labelHuy, ImVec2(buttonWidth, 34.0f))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        bool clickedConfirm = laNguyHiem
            ? NutNguyHiem(labelDongY, ImVec2(buttonWidth, 34.0f))
            : NutChinh(labelDongY, ImVec2(buttonWidth, 34.0f));

        if (clickedConfirm) {
            if (onDongY) {
                onDongY();
            }
            daDongY = true;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    return daDongY;
}

}  // namespace Widgets
