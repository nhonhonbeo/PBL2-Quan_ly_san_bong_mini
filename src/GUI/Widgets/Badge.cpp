#include "GUI/Widgets/Badge.h"
#include <string>

#include "GUI/Theme/Theme.h"
#include "GUI/Theme/Fonts.h"

namespace Widgets {

void Badge(const char* text, const ImVec4& bgMau, const ImVec4& textMau) {
    if (text == nullptr || text[0] == '\0') {
        return;
    }

    ImFont* font = GUI::Theme::getFonts().small;
    if (font) ImGui::PushFont(font);
    ImVec2 txtSize = ImGui::CalcTextSize(text);
    float padX = 9.0f;
    float padY = 4.0f;
    constexpr float dotSpace = 10.0f;
    ImVec2 badgeSize(txtSize.x + padX * 2.0f + dotSpace, txtSize.y + padY * 2.0f);

    ImVec2 p0 = ImGui::GetCursorScreenPos();
    ImVec2 p1 = ImVec2(p0.x + badgeSize.x, p0.y + badgeSize.y);

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    // LÝ DO CHỌN: Phong cách Modern Soft Pill Badge (bo tròn nửa chiều cao, nền pastel dịu nhẹ),
    // giúp người dùng nhận diện ngay trạng thái mà không gây nhức mắt.
    float rounding = badgeSize.y * 0.5f;

    drawList->AddRectFilled(p0, p1, GUI::Theme::Colors::toU32(bgMau), rounding);
    drawList->AddRect(p0, p1, GUI::Theme::Colors::toU32(textMau, 0.28f), rounding);
    drawList->AddCircleFilled(ImVec2(p0.x + padX + 2.0f, p0.y + badgeSize.y * 0.5f),
                             3.0f, GUI::Theme::Colors::toU32(textMau));
    drawList->AddText(ImVec2(p0.x + padX + dotSpace, p0.y + padY), GUI::Theme::Colors::toU32(textMau), text);

    ImGui::Dummy(badgeSize);
    if (font) ImGui::PopFont();
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

void BadgeNoiBat(const char* text) {
    // LÝ DO CHỌN: Nền vàng nhạt sáng #FEF9C3, chữ nâu vàng đậm #713F12 tương phản cao đạt chuẩn WCAG
    Badge(text, ImVec4(0.996f, 0.976f, 0.765f, 1.0f), ImVec4(0.443f, 0.247f, 0.071f, 1.0f));
}

void BadgeCanhBao(const char* text) {
    // Nền đỏ pastel, chữ đỏ đậm #DC2626
    Badge(text, ImVec4(0.996f, 0.886f, 0.886f, 1.0f), GUI::Theme::Colors::Danger);
}

void BookingStatusBadge(const char* status) {
    const std::string s = status ? status : "";
    if (s == "CONFIRMED" || s == u8"Đã xác nhận") BadgeDaDat(u8"Đã xác nhận");
    else if (s == "COMPLETED" || s == u8"Đã hoàn thành" || s == u8"Đã hoàn tất") BadgeTrong(u8"Hoàn thành");
    else if (s == "PAID" || s == u8"Đã thanh toán") BadgeTrong(u8"Đã thanh toán");
    else if (s == "CANCELLED" || s == u8"Đã hủy") BadgeCanhBao(u8"Đã hủy");
    else if (s == "PENDING" || s == u8"Chờ xác nhận" || s == u8"Chờ duyệt") BadgeBaoTri(u8"Chờ xác nhận");
    else if (s == "NO_SHOW") BadgeBaoTri(u8"Không đến");
    else Badge(s.empty() ? u8"Chưa rõ" : s.c_str(), GUI::Theme::Colors::GrayHover, GUI::Theme::Colors::TextSecondary);
}

}  // namespace Widgets