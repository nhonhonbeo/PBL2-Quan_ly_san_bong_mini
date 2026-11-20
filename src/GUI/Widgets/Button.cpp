#include "GUI/Widgets/Button.h"

#include "GUI/Theme/Theme.h"
#include "GUI/Theme/Fonts.h"

namespace Widgets {

bool NutChinh(const char* label, const ImVec2& kichThuoc) {
    ImGui::PushStyleColor(ImGuiCol_Button,        GUI::Theme::Colors::Primary);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, GUI::Theme::Colors::PrimaryHover);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  GUI::Theme::Colors::PrimaryDark);
    ImGui::PushStyleColor(ImGuiCol_Text,          GUI::Theme::Colors::Surface);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);

    if (GUI::Theme::getFonts().label) ImGui::PushFont(GUI::Theme::getFonts().label);
    bool clicked = ImGui::Button(label, kichThuoc);
    if (GUI::Theme::getFonts().label) ImGui::PopFont();
    if (ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

    ImGui::PopStyleVar();
    ImGui::PopStyleColor(4);
    return clicked;
}

bool NutPhu(const char* label, const ImVec2& kichThuoc) {
    ImGui::PushStyleColor(ImGuiCol_Button,        GUI::Theme::Colors::Surface);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, GUI::Theme::Colors::GrayHover);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.910f, 0.935f, 0.925f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Text,          GUI::Theme::Colors::TextPrimary);
    ImGui::PushStyleColor(ImGuiCol_Border,        GUI::Theme::Colors::InputBorder);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);

    if (GUI::Theme::getFonts().label) ImGui::PushFont(GUI::Theme::getFonts().label);
    bool clicked = ImGui::Button(label, kichThuoc);
    if (GUI::Theme::getFonts().label) ImGui::PopFont();
    if (ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(5);
    return clicked;
}

bool NutNguyHiem(const char* label, const ImVec2& kichThuoc) {
    ImGui::PushStyleColor(ImGuiCol_Button,        GUI::Theme::Colors::Danger);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.784f, 0.243f, 0.243f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.690f, 0.165f, 0.165f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Text,          GUI::Theme::Colors::Surface);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);

    if (GUI::Theme::getFonts().label) ImGui::PushFont(GUI::Theme::getFonts().label);
    bool clicked = ImGui::Button(label, kichThuoc);
    if (GUI::Theme::getFonts().label) ImGui::PopFont();
    if (ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

    ImGui::PopStyleVar();
    ImGui::PopStyleColor(4);
    return clicked;
}

}  // namespace Widgets
