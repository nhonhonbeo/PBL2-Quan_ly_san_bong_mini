#include "GUI/Widgets/Card.h"

#include <imgui.h>
#include "GUI/Theme/Theme.h"

namespace Widgets {

void Card(const char* id, const char* tieuDe, const std::function<void()>& noiDung) {
    Card(id, tieuDe, 16.0f, 16.0f, noiDung);
}

void Card(const char* id, const char* tieuDe, float paddingX, float paddingY, const std::function<void()>& noiDung) {
    ImGui::PushID(id);

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    float availableWidth = ImGui::GetContentRegionAvail().x;
    ImVec2 startCursor = ImGui::GetCursorScreenPos();

    // LÝ DO CHỌN: Tách 2 kênh vẽ (ChannelsSplit): kênh 1 vẽ nội dung trước để tính kích thước thực tế,
    // sau đó kênh 0 vẽ nền trắng và đổ bóng bên dưới mà không cần cố định chiều cao thẻ.
    drawList->ChannelsSplit(2);

    // Kênh 1: Nội dung bên trong thẻ
    drawList->ChannelsSetCurrent(1);
    ImGui::SetCursorScreenPos(ImVec2(startCursor.x + paddingX, startCursor.y + paddingY));
    ImGui::BeginGroup();

    // Tiêu đề thẻ (nếu có)
    if (tieuDe != nullptr && tieuDe[0] != '\0') {
        ImVec2 titlePos = ImGui::GetCursorScreenPos();
        // Vạch màu xanh cỏ nhỏ bên trái tiêu đề tạo điểm nhấn thương hiệu
        ImU32 accentColor = GUI::Theme::Colors::toU32(GUI::Theme::Colors::GrassGreen);
        drawList->AddRectFilled(
            ImVec2(titlePos.x - 6.0f, titlePos.y + 2.0f),
            ImVec2(titlePos.x - 2.0f, titlePos.y + ImGui::GetTextLineHeight() - 2.0f),
            accentColor,
            2.0f
        );

        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::DarkText);
        ImGui::TextUnformatted(tieuDe);
        ImGui::PopStyleColor();

        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Separator, GUI::Theme::Colors::BorderLight);
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
    drawList->ChannelsSetCurrent(0);

    // LÝ DO CHỌN: Vẽ 3 lớp bóng mờ xếp chồng để tạo hiệu ứng đổ bóng khuếch tán (soft ambient shadow)
    drawList->AddRectFilled(
        ImVec2(cardMin.x - 1.0f, cardMin.y + 2.0f),
        ImVec2(cardMax.x + 1.0f, cardMax.y + 4.0f),
        IM_COL32(0, 0, 0, 12),
        9.0f
    );
    drawList->AddRectFilled(
        ImVec2(cardMin.x, cardMin.y + 1.0f),
        ImVec2(cardMax.x, cardMax.y + 2.0f),
        IM_COL32(0, 0, 0, 18),
        8.0f
    );

    // Nền trắng thẻ bo góc 8px
    ImU32 cardBgColor = GUI::Theme::Colors::toU32(GUI::Theme::Colors::WhiteCard);
    drawList->AddRectFilled(cardMin, cardMax, cardBgColor, 8.0f);

    // Viền nhẹ tinh tế
    ImU32 borderColor = GUI::Theme::Colors::toU32(GUI::Theme::Colors::BorderLight);
    drawList->AddRect(cardMin, cardMax, borderColor, 8.0f, 0, 1.0f);

    drawList->ChannelsMerge();

    // Đẩy con trỏ layout xuống dưới thẻ vừa vẽ kèm khoảng cách 12px
    ImGui::SetCursorScreenPos(ImVec2(startCursor.x, cardMax.y + 12.0f));

    ImGui::PopID();
}

}  // namespace Widgets
