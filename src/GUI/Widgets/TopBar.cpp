#include "GUI/Widgets/TopBar.h"

#include <imgui.h>

#include "GUI/Theme/Fonts.h"
#include "GUI/Theme/Theme.h"
#include "GUI/Widgets/Badge.h"

namespace Widgets {

void TopBar(
    const char* tieuDeManHinh,
    const std::vector<std::string>& breadcrumbs,
    const char* tenNguoiDung,
    const char* vaiTro,
    float chieuCao,
    const std::function<void()>& onDangXuat
) {
    const ImVec2 start = ImGui::GetCursorScreenPos();
    const float width = ImGui::GetContentRegionAvail().x;
    const auto& fonts = GUI::Theme::getFonts();
    using C = GUI::Theme::Colors;
    auto* dl = ImGui::GetWindowDrawList();
    dl->AddRectFilled(start, ImVec2(start.x + width, start.y + chieuCao), C::toU32(C::Surface));
    dl->AddLine(ImVec2(start.x, start.y + chieuCao), ImVec2(start.x + width, start.y + chieuCao), C::toU32(C::Border));
    ImGui::SetCursorScreenPos(ImVec2(start.x + 20, start.y + 10));
    if (fonts.small) ImGui::PushFont(fonts.small);
    ImGui::TextColored(C::TextSecondary,"%s", breadcrumbs.empty() ? "" : breadcrumbs.front().c_str());
    if (fonts.small) ImGui::PopFont();
    ImGui::SetCursorScreenPos(ImVec2(start.x + 20, start.y + 30));
    if (fonts.heading) ImGui::PushFont(fonts.heading);
    ImGui::TextUnformatted(tieuDeManHinh ? tieuDeManHinh : "");
    if (fonts.heading) ImGui::PopFont();
    const float userX = start.x + width - 204;
    const ImVec2 center(userX + 16, start.y + chieuCao * .5f);
    dl->AddCircleFilled(center, 16, C::toU32(C::PrimaryLight));
    dl->AddCircleFilled(ImVec2(center.x, center.y - 4), 4, C::toU32(C::Primary));
    dl->AddRectFilled(ImVec2(center.x - 7, center.y + 2), ImVec2(center.x + 7, center.y + 9), C::toU32(C::Primary), 5);
    ImGui::SetCursorScreenPos(ImVec2(userX + 42, start.y + 16));
    const char* name = tenNguoiDung ? tenNguoiDung : u8"Người dùng";
    const ImVec2 namePos = ImGui::GetCursorScreenPos();
    dl->PushClipRect(namePos, ImVec2(start.x + width - 12, namePos.y + ImGui::GetTextLineHeight()), true);
    dl->AddText(namePos, C::toU32(C::TextPrimary), name);
    dl->PopClipRect();
    ImGui::Dummy(ImVec2(145, ImGui::GetTextLineHeight()));
    if (ImGui::IsItemHovered() && ImGui::CalcTextSize(name).x > 145) ImGui::SetTooltip("%s",name);
    if (fonts.small) ImGui::PushFont(fonts.small);
    ImGui::SetCursorScreenPos(ImVec2(userX + 42, start.y + 48));
    ImGui::TextColored(C::TextSecondary,"%s", vaiTro ? vaiTro : "");
    if (fonts.small) ImGui::PopFont();
    if (onDangXuat) {
        ImGui::SetCursorScreenPos(ImVec2(userX, start.y + chieuCao));
        if (ImGui::SmallButton(u8"Đăng xuất##topbar_logout")) onDangXuat();
    }
    ImGui::SetCursorScreenPos(ImVec2(start.x, start.y + chieuCao));
    ImGui::Dummy(ImVec2(width, 1));
}

} // namespace Widgets
