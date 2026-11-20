#include "GUI/Widgets/Card.h"

#include <imgui.h>
#include "GUI/Theme/Fonts.h"
#include "GUI/Theme/Theme.h"

namespace Widgets {

void Card(const char* id, const char* tieuDe, const std::function<void()>& noiDung) {
    Card(id, tieuDe, 14.0f, 12.0f, noiDung);
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
    ImGui::SetCursorScreenPos(ImVec2(startCursor.x, startCursor.y + paddingY));
    ImGui::Indent(paddingX);
    ImGui::BeginGroup();

    // Tiêu đề thẻ (nếu có)
    if (tieuDe != nullptr && tieuDe[0] != '\0') {
        ImFont* titleFont = GUI::Theme::getFonts().title;
        if (titleFont != nullptr) ImGui::PushFont(titleFont);

        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::TextPrimary);
        ImGui::TextWrapped("%s", tieuDe);
        ImGui::PopStyleColor();

        if (titleFont != nullptr) ImGui::PopFont();

        ImGui::Spacing();
    }

    // Thực thi lambda vẽ nội dung
    noiDung();

    ImGui::EndGroup();

    ImVec2 contentMax = ImGui::GetItemRectMax();
    ImGui::Unindent(paddingX);
    float finalHeight = (contentMax.y - startCursor.y) + paddingY;
    ImVec2 cardMin = startCursor;
    ImVec2 cardMax = ImVec2(startCursor.x + availableWidth, startCursor.y + finalHeight);

    // Kênh 0: Nền thẻ và đổ bóng mềm
    splitter.SetCurrentChannel(drawList, 0);

    // Nền trắng thẻ bo góc 8px
    ImU32 cardBgColor = GUI::Theme::Colors::toU32(GUI::Theme::Colors::Surface);
    drawList->AddRectFilled(cardMin, cardMax, cardBgColor, 8.0f);

    // Viền nhẹ tinh tế #E2E8E5
    ImU32 borderColor = GUI::Theme::Colors::toU32(GUI::Theme::Colors::Border);
    drawList->AddRect(cardMin, cardMax, borderColor, 8.0f, 0, 1.0f);

    splitter.Merge(drawList);

    // Đẩy con trỏ layout xuống dưới thẻ vừa vẽ kèm khoảng cách 14px
    ImGui::SetCursorScreenPos(ImVec2(startCursor.x, cardMax.y + 10.0f));

    ImGui::PopID();
}

}  // namespace Widgets
