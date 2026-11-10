#include "GUI/Widgets/Badge.h"

#include "GUI/Theme/Theme.h"

namespace Widgets {

void Badge(const char* text, const ImVec4& bgMau, const ImVec4& textMau) {
    if (text == nullptr || text[0] == '\0') {
        return;
    }

    ImVec2 txtSize = ImGui::CalcTextSize(text);
    float padX = 10.0f;
    float padY = 3.0f;
    ImVec2 badgeSize(txtSize.x + padX * 2.0f, txtSize.y + padY * 2.0f);

    ImVec2 p0 = ImGui::GetCursorScreenPos();
    ImVec2 p1 = ImVec2(p0.x + badgeSize.x, p0.y + badgeSize.y);

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    // LÝ DO CHỌN: Phong cách Modern Soft Pill Badge (bo tròn nửa chiều cao, nền pastel dịu nhẹ),
    // giúp người dùng nhận diện ngay trạng thái mà không gây nhức mắt.
    float rounding = badgeSize.y * 0.5f;

    drawList->AddRectFilled(p0, p1, GUI::Theme::Colors::toU32(bgMau), rounding);
    // Viền nhẹ cùng tông chữ để đảm bảo phân tách rõ ràng trên mọi màn hình
    drawList->AddRect(p0, p1, GUI::Theme::Colors::toU32(textMau, 0.35f), rounding, 0, 1.0f);
    drawList->AddText(ImVec2(p0.x + padX, p0.y + padY), GUI::Theme::Colors::toU32(textMau), text);

    ImGui::Dummy(badgeSize);
}

void BadgeTrong(const char* text) {
    // Nền xanh lá pastel dịu, chữ xanh thương hiệu #16865B
    Badge(text, ImVec4(0.902f, 0.969f, 0.937f, 1.0f), GUI::Theme::Colors::Primary);
}

void BadgeDaDat(const char* text) {
    // Nền xanh dương pastel, chữ xanh đậm #2563EB
    Badge(text, ImVec4(0.910f, 0.949f, 0.980f, 1.0f), ImVec4(0.145f, 0.388f, 0.922f, 1.0f));
}

void BadgeBaoTri(const char* text) {
    // LÝ DO CHỌN: Đạt chuẩn tương phản tiếp cận WCAG AAA (>7:1):
    // Nền hổ phách ấm #FEF3C7, chữ nâu đen sẫm #78350F giúp đọc rõ nét tuyệt đối trên mọi màn hình.
    Badge(text, ImVec4(0.996f, 0.953f, 0.780f, 1.0f), ImVec4(0.471f, 0.208f, 0.059f, 1.0f));
}

void BadgeVIP(const char* text) {
    // LÝ DO CHỌN: Nền vàng nhạt sáng #FEF9C3, chữ nâu vàng đậm #713F12 tương phản cao đạt chuẩn WCAG
    Badge(text, ImVec4(0.996f, 0.976f, 0.765f, 1.0f), ImVec4(0.443f, 0.247f, 0.071f, 1.0f));
}

void BadgeDanhSachDen(const char* text) {
    // Nền đỏ pastel, chữ đỏ đậm #DC2626
    Badge(text, ImVec4(0.996f, 0.886f, 0.886f, 1.0f), GUI::Theme::Colors::Danger);
}

}  // namespace Widgets