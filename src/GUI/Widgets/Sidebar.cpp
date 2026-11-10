#include "GUI/Widgets/Sidebar.h"

#include <imgui.h>
#include "GUI/Theme/Fonts.h"
#include "GUI/Theme/Theme.h"
#include "GUI/Widgets/Button.h"

namespace Widgets {

bool Sidebar(
    const std::vector<SidebarItem>& danhSachMuc,
    int& mucDangChon,
    float chieuRong,
    const std::function<void()>& onDangXuat
) {
    bool changed = false;

    ImGui::PushStyleColor(ImGuiCol_ChildBg, GUI::Theme::Colors::Surface);
    ImGui::PushStyleColor(ImGuiCol_Border, GUI::Theme::Colors::Border);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(14.0f, 16.0f));

    ImGuiWindowFlags sidebarFlags = ImGuiWindowFlags_NoScrollbar |
                                    ImGuiWindowFlags_NoScrollWithMouse;
    if (ImGui::BeginChild("##SidebarChild", ImVec2(chieuRong, 0.0f), true, sidebarFlags)) {
        ImDrawList* drawList = ImGui::GetWindowDrawList();

        // ==========================================
        // 1. LOGO & TÊN THƯƠNG HIỆU (BRAND HEADER)
        // ==========================================
        ImVec2 headerStart = ImGui::GetCursorScreenPos();
        float logoRadius = 16.0f;
        ImVec2 logoCenter(headerStart.x + logoRadius + 2.0f, headerStart.y + logoRadius + 2.0f);

        // Huy hiệu logo dạng vector sân bóng mini
        drawList->AddCircleFilled(logoCenter, logoRadius, GUI::Theme::Colors::toU32(GUI::Theme::Colors::Primary));
        // Vẽ vạch bóng / khung sân nhỏ bên trong logo
        drawList->AddCircle(logoCenter, logoRadius * 0.58f, IM_COL32_WHITE, 16, 1.2f);
        drawList->AddLine(
            ImVec2(logoCenter.x - logoRadius * 0.58f, logoCenter.y),
            ImVec2(logoCenter.x + logoRadius * 0.58f, logoCenter.y),
            IM_COL32_WHITE, 1.2f
        );

        // Tên thương hiệu bên cạnh logo
        ImGui::SetCursorScreenPos(ImVec2(headerStart.x + logoRadius * 2.0f + 12.0f, headerStart.y));
        ImGui::BeginGroup();

        ImFont* titleFont = GUI::Theme::getFonts().title;
        if (titleFont != nullptr) ImGui::PushFont(titleFont);
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::Primary);
        ImGui::TextUnformatted(u8"SÂN BÓNG MINI");
        ImGui::PopStyleColor();
        if (titleFont != nullptr) ImGui::PopFont();

        ImFont* smallFont = GUI::Theme::getFonts().small;
        if (smallFont != nullptr) ImGui::PushFont(smallFont);
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::TextSecondary);
        ImGui::TextUnformatted(u8"Hệ thống quản lý cụm sân");
        ImGui::PopStyleColor();
        if (smallFont != nullptr) ImGui::PopFont();

        ImGui::EndGroup();

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8.0f);
        ImGui::PushStyleColor(ImGuiCol_Separator, GUI::Theme::Colors::Border);
        ImGui::Separator();
        ImGui::PopStyleColor();
        ImGui::Spacing();
        ImGui::Spacing();

        // ==========================================
        // 2. DANH SÁCH MỤC MENU
        // ==========================================
        for (int i = 0; i < static_cast<int>(danhSachMuc.size()); ++i) {
            const auto& item = danhSachMuc[i];
            bool isSelected = (mucDangChon == i);

            std::string itemLabel = item.bieuTuong.empty()
                ? item.tieuDe
                : (item.bieuTuong + "  " + item.tieuDe);

            ImVec2 itemPos = ImGui::GetCursorScreenPos();
            ImVec2 itemSize(ImGui::GetContentRegionAvail().x, 42.0f);

            if (isSelected) {
                // Mục đang chọn: nền xanh nhạt hiện đại (#E8F6EF), chữ xanh đậm (#10543C)
                ImGui::PushStyleColor(ImGuiCol_Button,        GUI::Theme::Colors::PrimaryLight);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, GUI::Theme::Colors::PrimaryLight);
                ImGui::PushStyleColor(ImGuiCol_ButtonActive,  GUI::Theme::Colors::PrimaryLight);
                ImGui::PushStyleColor(ImGuiCol_Text,          GUI::Theme::Colors::PrimaryDark);
            } else {
                // Mục bình thường: nền trong suốt, hover xám nhạt, chữ đậm
                ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0, 0, 0, 0));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, GUI::Theme::Colors::GrayHover);
                ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.92f, 0.94f, 0.93f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_Text,          GUI::Theme::Colors::TextPrimary);
            }

            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.08f, 0.5f)); // Căn chữ lệch trái tự nhiên

            std::string btnId = itemLabel + "##side_" + item.id;
            if (ImGui::Button(btnId.c_str(), itemSize)) {
                mucDangChon = i;
                changed = true;
            }

            // Vẽ vạch chỉ báo bên trái cho mục đang chọn
            if (isSelected) {
                drawList->AddRectFilled(
                    ImVec2(itemPos.x, itemPos.y + 6.0f),
                    ImVec2(itemPos.x + 3.5f, itemPos.y + itemSize.y - 6.0f),
                    GUI::Theme::Colors::toU32(GUI::Theme::Colors::Primary),
                    2.0f
                );
            }

            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor(4);

            ImGui::Spacing();
        }

        // ==========================================
        // 3. NÚT ĐĂNG XUẤT ĐẶT DƯỚI ĐÁY
        // ==========================================
        float windowHeight = ImGui::GetWindowHeight();
        float logoutBtnHeight = 38.0f;
        float bottomPadding = 16.0f;
        float logoutPosY = windowHeight - logoutBtnHeight - bottomPadding;

        if (logoutPosY > ImGui::GetCursorPosY() + 20.0f) {
            ImGui::SetCursorPosY(logoutPosY - 24.0f);
        }

        ImGui::PushStyleColor(ImGuiCol_Separator, GUI::Theme::Colors::Border);
        ImGui::Separator();
        ImGui::PopStyleColor();
        ImGui::Spacing();

        // LÝ DO CHỌN: Dùng Ghost Button chữ xám tinh tế (#748078) thay vì nút đỏ cảnh báo,
        // giúp người dùng tập trung vào các chức năng chính, tránh tâm lý mời người dùng rời app.
        ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, GUI::Theme::Colors::GrayHover);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.92f, 0.94f, 0.93f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Text,          GUI::Theme::Colors::TextSecondary);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.12f, 0.5f));
        if (ImGui::Button(u8"Đăng xuất##sidebar_logout_btn", ImVec2(ImGui::GetContentRegionAvail().x, logoutBtnHeight))) {
            if (onDangXuat) {
                onDangXuat();
            }
        }
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(4);
    }
    ImGui::EndChild();

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(2);

    return changed;
}

}  // namespace Widgets
