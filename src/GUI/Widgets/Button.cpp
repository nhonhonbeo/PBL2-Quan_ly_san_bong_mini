#include "GUI/Widgets/Button.h"

#include "GUI/Theme/Theme.h"

namespace Widgets {

bool NutChinh(const char* label, const ImVec2& kichThuoc) {
    ImGui::PushStyleColor(ImGuiCol_Button,        GUI::Theme::Colors::GrassGreen);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.13f, 0.62f, 0.40f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  GUI::Theme::Colors::DarkGreen);
    ImGui::PushStyleColor(ImGuiCol_Text,          GUI::Theme::Colors::WhiteCard);

    bool clicked = ImGui::Button(label, kichThuoc);

    ImGui::PopStyleColor(4);
    return clicked;
}

bool NutPhu(const char* label, const ImVec2& kichThuoc) {
    ImGui::PushStyleColor(ImGuiCol_Button,        GUI::Theme::Colors::WhiteCard);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.95f, 0.96f, 0.97f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.90f, 0.92f, 0.94f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Text,          GUI::Theme::Colors::DarkText);
    ImGui::PushStyleColor(ImGuiCol_Border,        GUI::Theme::Colors::BorderLight);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);

    bool clicked = ImGui::Button(label, kichThuoc);

    ImGui::PopStyleVar();
    ImGui::PopStyleColor(5);
    return clicked;
}

bool NutNguyHiem(const char* label, const ImVec2& kichThuoc) {
    ImGui::PushStyleColor(ImGuiCol_Button,        GUI::Theme::Colors::RedDanger);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.93f, 0.28f, 0.34f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.74f, 0.13f, 0.19f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Text,          GUI::Theme::Colors::WhiteCard);

    bool clicked = ImGui::Button(label, kichThuoc);

    ImGui::PopStyleColor(4);
    return clicked;
}

}  // namespace Widgets
