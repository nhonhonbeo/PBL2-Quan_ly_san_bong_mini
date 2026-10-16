#include "GUI/Widgets/Badge.h"

#include "GUI/Theme/Theme.h"

namespace Widgets {

void Badge(const char* text, const ImVec4& bgMau, const ImVec4& textMau) {
    if (text == nullptr || text[0] == '\0') {
        return;
    }

    ImVec2 txtSize = ImGui::CalcTextSize(text);
    float padX = 10.0f;
    float padY = 4.0f;
    ImVec2 badgeSize(txtSize.x + padX * 2.0f, txtSize.y + padY * 2.0f);

    ImVec2 p0 = ImGui::GetCursorScreenPos();
    ImVec2 p1 = ImVec2(p0.x + badgeSize.x, p0.y + badgeSize.y);

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    // LÝ DO CHỌN: Bo tròn nửa chiều cao (rounding = height / 2) tạo phong cách Pill Badge hiện đại
    float rounding = badgeSize.y * 0.5f;

    drawList->AddRectFilled(p0, p1, GUI::Theme::Colors::toU32(bgMau), rounding);
    drawList->AddText(ImVec2(p0.x + padX, p0.y + padY), GUI::Theme::Colors::toU32(textMau), text);

    ImGui::Dummy(badgeSize);
}

void BadgeTrong(const char* text) {
    // Nền xanh lá nhạt, chữ xanh cỏ chính
    Badge(text, ImVec4(0.88f, 0.96f, 0.90f, 1.0f), GUI::Theme::Colors::GrassGreen);
}

void BadgeDaDat(const char* text) {
    // Nền xanh cỏ đậm, chữ trắng
    Badge(text, GUI::Theme::Colors::GrassGreen, GUI::Theme::Colors::WhiteCard);
}

void BadgeBaoTri(const char* text) {
    // Nền vàng cam nhạt, chữ cam sẫm
    Badge(text, ImVec4(0.99f, 0.93f, 0.80f, 1.0f), ImVec4(0.78f, 0.45f, 0.05f, 1.0f));
}

void BadgeVIP(const char* text) {
    // Nền vàng ánh kim rực rỡ, chữ trắng
    Badge(text, GUI::Theme::Colors::GoldAccent, GUI::Theme::Colors::WhiteCard);
}

void BadgeDanhSachDen(const char* text) {
    // Nền đỏ nhạt, chữ đỏ nguy hiểm
    Badge(text, ImVec4(0.99f, 0.88f, 0.89f, 1.0f), GUI::Theme::Colors::RedDanger);
}

}  // namespace Widgets
