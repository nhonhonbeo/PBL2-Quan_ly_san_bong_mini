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
    ImVec2 startPos = ImGui::GetCursorScreenPos();
    float totalWidth = ImGui::GetContentRegionAvail().x;
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    // Nền trắng và đường viền đáy tinh tế
    ImVec2 barMin = startPos;
    ImVec2 barMax = ImVec2(startPos.x + totalWidth, startPos.y + chieuCao);

    drawList->AddRectFilled(barMin, barMax, GUI::Theme::Colors::toU32(GUI::Theme::Colors::Surface));
    drawList->AddLine(
        ImVec2(barMin.x, barMax.y),
        ImVec2(barMax.x, barMax.y),
        GUI::Theme::Colors::toU32(GUI::Theme::Colors::Border),
        1.0f
    );

    ImGui::SetCursorScreenPos(ImVec2(startPos.x + 20.0f, startPos.y + 10.0f));
    ImGui::BeginGroup();

    // ==========================================
    // 1. BREADCRUMB & TIÊU ĐỀ BÊN TRÁI
    // ==========================================
    if (!breadcrumbs.empty()) {
        ImFont* smallFont = GUI::Theme::getFonts().small;
        if (smallFont != nullptr) ImGui::PushFont(smallFont);

        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::TextSecondary);
        for (size_t i = 0; i < breadcrumbs.size(); ++i) {
            if (i > 0) {
                ImGui::SameLine();
                ImGui::TextUnformatted(" › ");
                ImGui::SameLine();
            }
            ImGui::TextUnformatted(breadcrumbs[i].c_str());
        }
        ImGui::PopStyleColor();

        if (smallFont != nullptr) ImGui::PopFont();
    }

    if (tieuDeManHinh != nullptr && tieuDeManHinh[0] != '\0') {
        ImFont* titleFont = GUI::Theme::getFonts().title;
        if (titleFont != nullptr) ImGui::PushFont(titleFont);

        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::TextPrimary);
        ImGui::TextUnformatted(tieuDeManHinh);
        ImGui::PopStyleColor();

        if (titleFont != nullptr) ImGui::PopFont();
    }

    ImGui::EndGroup();

    // ==========================================
    // 2. USER CHIP & ĐĂNG XUẤT BÊN PHẢI (LOẠI BỎ ĐỒNG HỒ GIÂY GÂY XAO NHÃNG)
    // ==========================================
    std::string userStr = tenNguoiDung ? tenNguoiDung : u8"Người dùng";
    std::string roleStr = vaiTro ? vaiTro : "";

    ImVec2 userSize = ImGui::CalcTextSize(userStr.c_str());
    ImVec2 roleSize = ImGui::CalcTextSize(roleStr.c_str());
    float logoutTextWidth = onDangXuat ? 90.0f : 0.0f;
    float rightBlockWidth = userSize.x + roleSize.x + logoutTextWidth + 90.0f;

    float rightPosX = barMax.x - rightBlockWidth - 20.0f;
    if (rightPosX < startPos.x + 280.0f) {
        rightPosX = startPos.x + 280.0f;
    }
    ImGui::SetCursorScreenPos(ImVec2(rightPosX, startPos.y + 14.0f));
    ImGui::BeginGroup();

    // Chấm xanh trạng thái trực tuyến
    ImVec2 curPos = ImGui::GetCursorScreenPos();
    drawList->AddCircleFilled(ImVec2(curPos.x + 3.0f, curPos.y + 10.0f), 4.0f, GUI::Theme::Colors::toU32(GUI::Theme::Colors::Success));
    ImGui::Dummy(ImVec2(10.0f, 20.0f));
    ImGui::SameLine(0.0f, 6.0f);

    // Avatar tròn
    ImVec2 avatarPos = ImGui::GetCursorScreenPos();
    float avatarR = 14.0f;
    ImVec2 avatarCenter(avatarPos.x + avatarR, avatarPos.y + avatarR - 3.0f);
    drawList->AddCircleFilled(avatarCenter, avatarR, GUI::Theme::Colors::toU32(GUI::Theme::Colors::Primary));

    char initial[2] = {tenNguoiDung && tenNguoiDung[0] ? tenNguoiDung[0] : 'U', '\0'};
    ImVec2 initSize = ImGui::CalcTextSize(initial);
    drawList->AddText(
        ImVec2(avatarCenter.x - initSize.x * 0.5f, avatarCenter.y - initSize.y * 0.5f),
        IM_COL32_WHITE,
        initial
    );

    ImGui::Dummy(ImVec2(avatarR * 2.0f + 6.0f, avatarR * 2.0f));
    ImGui::SameLine();

    // Tên người dùng
    ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::TextPrimary);
    ImGui::TextUnformatted(userStr.c_str());
    ImGui::PopStyleColor();

    // Pill badge vai trò
    if (!roleStr.empty()) {
        ImGui::SameLine(0.0f, 8.0f);
        if (roleStr == u8"Chủ sân") {
            Badge(roleStr.c_str(), GUI::Theme::Colors::PrimaryLight, GUI::Theme::Colors::Primary);
        } else {
            Badge(roleStr.c_str(), ImVec4(0.910f, 0.949f, 0.980f, 1.0f), ImVec4(0.145f, 0.388f, 0.922f, 1.0f));
        }
    }

    // LÝ DO CHỌN: Text link Đăng xuất mờ nhạt đặt tại góc trên cùng bên phải cạnh hồ sơ người dùng,
    // vừa đúng chuẩn UX hiện đại vừa không chiếm spotlight luồng thao tác chính.
    if (onDangXuat) {
        ImGui::SameLine(0.0f, 14.0f);
        ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, GUI::Theme::Colors::GrayHover);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.92f, 0.94f, 0.93f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Text,          GUI::Theme::Colors::TextSecondary);
        if (ImGui::SmallButton(u8"Đăng xuất##topbar_logout")) {
            onDangXuat();
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        }
        ImGui::PopStyleColor(4);
    }

    ImGui::EndGroup();

    // Đẩy con trỏ xuống dưới TopBar kèm khoảng cách 16px
    ImGui::SetCursorScreenPos(ImVec2(startPos.x, barMax.y + 16.0f));
}

}  // namespace Widgets
