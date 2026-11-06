#include "GUI/Widgets/Card.h"

#include <imgui.h>
#include "GUI/Theme/Fonts.h"
#include "GUI/Theme/Theme.h"

namespace Widgets {

void Card(const char* id, const char* tieuDe, const std::function<void()>& noiDung) {
    Card(id, tieuDe, 18.0f, 16.0f, noiDung);
}

void Card(const char* id, const char* tieuDe, float paddingX, float paddingY, const std::function<void()>& noiDung) {
    ImGui::PushID(id);

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    float availableWidth = ImGui::GetContentRegionAvail().x;
    ImVec2 startCursor = ImGui::GetCursorScreenPos();

    // LÝ DO CHỌN: Dùng ImDrawListSplitter riêng cục bộ cho mỗi Card thay vì drawList->ChannelsSplit(),
    // cho phép các thẻ Card lồng nhau tùy ý mà không vi phạm ràng buộc phân kênh của ImGui.
    ImDrawListSplitter splitter;
    splitter.Split(drawList, 2);

    // Kênh 1: Nội dung bên trong thẻ
    splitter.SetCurrentChannel(drawList, 1);
    ImGui::SetCursorScreenPos(ImVec2(startCursor.x + paddingX, startCursor.y + paddingY));
    ImGui::BeginGroup();

    // Tiêu đề thẻ (nếu có)
    if (tieuDe != nullptr && tieuDe[0] != '\0') {
        ImVec2 titlePos = ImGui::GetCursorScreenPos();
        // Vạch màu xanh lá thương hiệu bo tròn bên trái tiêu đề tạo điểm nhấn chuyên nghiệp
        ImU32 accentColor = GUI::Theme::Colors::toU32(GUI::Theme::Colors::Primary);
        float lineHeight = ImGui::GetTextLineHeight();
        drawList->AddRectFilled(
            ImVec2(titlePos.x - 8.0f, titlePos.y + 2.0f),
            ImVec2(titlePos.x - 4.5f, titlePos.y + lineHeight - 1.0f),
            accentColor,
            2.0f
        );

        ImFont* titleFont = GUI::Theme::getFonts().title;
        if (titleFont != nullptr) ImGui::PushFont(titleFont);

        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::TextPrimary);
        ImGui::TextUnformatted(tieuDe);
        ImGui::PopStyleColor();

        if (titleFont != nullptr) ImGui::PopFont();

        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Separator, GUI::Theme::Colors::Border);
        ImGui::Separator();
        ImGui::PopStyleColor();
        ImGui::Spacing();
    }

    // Thực thi lambda vẽ nội dung
    noiDung();

    ImGui::EndGroup();

    ImVec2 contentMax = ImGui::GetItemRectMax();
    float finalHeight = (contentMax.y - startCursor.y) + paddingY;
    ImVec2 cardMin = startCursor;
    ImVec2 cardMax = ImVec2(startCursor.x + availableWidth, startCursor.y + finalHeight);

    // Kênh 0: Nền thẻ và đổ bóng mềm
    splitter.SetCurrentChannel(drawList, 0);

    // LÝ DO CHỌN: Đổ bóng khuếch tán đa tầng (ambient soft shadow) tạo cảm giác chiều sâu nổi khối SaaS
    drawList->AddRectFilled(
        ImVec2(cardMin.x - 1.0f, cardMin.y + 2.0f),
        ImVec2(cardMax.x + 1.0f, cardMax.y + 5.0f),
        IM_COL32(0, 0, 0, 10),
        8.0f
    );
    drawList->AddRectFilled(
        ImVec2(cardMin.x, cardMin.y + 1.0f),
        ImVec2(cardMax.x, cardMax.y + 2.0f),
        IM_COL32(0, 0, 0, 16),
        8.0f
    );

    // Nền trắng thẻ bo góc 8px
    ImU32 cardBgColor = GUI::Theme::Colors::toU32(GUI::Theme::Colors::Surface);
    drawList->AddRectFilled(cardMin, cardMax, cardBgColor, 8.0f);

    // Viền nhẹ tinh tế #E2E8E5
    ImU32 borderColor = GUI::Theme::Colors::toU32(GUI::Theme::Colors::Border);
    drawList->AddRect(cardMin, cardMax, borderColor, 8.0f, 0, 1.0f);

    splitter.Merge(drawList);

    // Đẩy con trỏ layout xuống dưới thẻ vừa vẽ kèm khoảng cách 14px
    ImGui::SetCursorScreenPos(ImVec2(startCursor.x, cardMax.y + 14.0f));

    ImGui::PopID();
}

}  // namespace Widgets
