#include "GUI/Widgets/Modal.h"

#include <string>
#include <unordered_set>
#include <imgui.h>
#include "GUI/Theme/Fonts.h"
#include "GUI/Theme/Theme.h"
#include "GUI/Widgets/Button.h"

namespace Widgets {

static std::unordered_set<std::string> s_PendingModals;

void MoModalXacNhan(const char* tieuDe) {
    if (tieuDe != nullptr && tieuDe[0] != '\0') {
        s_PendingModals.insert(tieuDe);
    }
    ImGui::OpenPopup(tieuDe);
}

bool KiemTraVaMoModal(const char* tieuDe) {
    if (tieuDe != nullptr && tieuDe[0] != '\0') {
        auto it = s_PendingModals.find(tieuDe);
        if (it != s_PendingModals.end()) {
            ImGui::OpenPopup(tieuDe);
            s_PendingModals.erase(it);
            return true;
        }
    }
    return false;
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

    KiemTraVaMoModal(tieuDe);

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(460.0f, 0.0f), ImGuiCond_Appearing);

    ImGuiWindowFlags flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(24.0f, 22.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);

    if (ImGui::BeginPopupModal(tieuDe, nullptr, flags)) {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 iconPos = ImGui::GetCursorScreenPos();
        float radius = 16.0f;
        ImVec2 centerIcon(iconPos.x + radius, iconPos.y + radius);

        ImU32 badgeBg = laNguyHiem
            ? IM_COL32(254, 226, 226, 255)   // Đỏ pastel
            : IM_COL32(254, 243, 199, 255);  // Vàng pastel

        ImU32 iconColor = laNguyHiem
            ? GUI::Theme::Colors::toU32(GUI::Theme::Colors::Danger)
            : GUI::Theme::Colors::toU32(GUI::Theme::Colors::Warning);

        drawList->AddCircleFilled(centerIcon, radius, badgeBg);
        drawList->AddText(ImVec2(centerIcon.x - 4.0f, centerIcon.y - 10.0f), iconColor, "!");

        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + radius * 2.0f + 16.0f);
        ImGui::BeginGroup();

        // Tiêu đề modal
        ImFont* titleFont = GUI::Theme::getFonts().title;
        if (titleFont != nullptr) ImGui::PushFont(titleFont);
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::TextPrimary);
        ImGui::TextUnformatted(tieuDe);
        ImGui::PopStyleColor();
        if (titleFont != nullptr) ImGui::PopFont();

        ImGui::Spacing();

        // Nội dung mô tả
        ImGui::PushTextWrapPos(ImGui::GetWindowWidth() - 32.0f);
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::TextSecondary);
        ImGui::TextUnformatted(noiDung);
        ImGui::PopStyleColor();
        ImGui::PopTextWrapPos();

        ImGui::EndGroup();

        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Separator, GUI::Theme::Colors::Border);
        ImGui::Separator();
        ImGui::PopStyleColor();
        ImGui::Spacing();

        // Căn nút bấm sang bên phải
        float buttonWidth = 115.0f;
        float totalButtonsWidth = buttonWidth * 2.0f + ImGui::GetStyle().ItemSpacing.x;
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - totalButtonsWidth - ImGui::GetStyle().WindowPadding.x);

        if (NutPhu(labelHuy, ImVec2(buttonWidth, 36.0f))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        bool clickedConfirm = laNguyHiem
            ? NutNguyHiem(labelDongY, ImVec2(buttonWidth, 36.0f))
            : NutChinh(labelDongY, ImVec2(buttonWidth, 36.0f));

        if (clickedConfirm) {
            if (onDongY) {
                onDongY();
            }
            daDongY = true;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    ImGui::PopStyleVar(2);
    return daDongY;
}

}  // namespace Widgets
