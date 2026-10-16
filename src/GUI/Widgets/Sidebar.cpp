#include "GUI/Widgets/Sidebar.h"

#include <imgui.h>
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

    ImGui::PushStyleColor(ImGuiCol_ChildBg, GUI::Theme::Colors::WhiteCard);
    ImGui::PushStyleColor(ImGuiCol_Border, GUI::Theme::Colors::BorderLight);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(14.0f, 18.0f));

    if (ImGui::BeginChild("##SidebarChild", ImVec2(chieuRong, 0.0f), true, ImGuiWindowFlags_NoScrollbar)) {
        // ==========================================
        // 1. LOGO & TÊN THƯƠNG HIỆU
        // ==========================================
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::GrassGreen);
        ImGui::TextUnformatted(u8"⚽ SÂN BÓNG MINI");
        ImGui::PopStyleColor();

        ImGui::PushStyleColor(ImGuiCol_Text, GUI::Theme::Colors::MutedText);
        ImGui::TextUnformatted(u8"Hệ thống quản lý sân");
        ImGui::PopStyleColor();

        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Separator, GUI::Theme::Colors::BorderLight);
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

            ImVec2 itemSize(ImGui::GetContentRegionAvail().x, 40.0f);

            if (isSelected) {
                // Mục đang chọn: nền xanh cỏ chính (#1B8A5A), chữ trắng
                ImGui::PushStyleColor(ImGuiCol_Button,        GUI::Theme::Colors::GrassGreen);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, GUI::Theme::Colors::GrassGreen);
                ImGui::PushStyleColor(ImGuiCol_ButtonActive,  GUI::Theme::Colors::DarkGreen);
                ImGui::PushStyleColor(ImGuiCol_Text,          GUI::Theme::Colors::WhiteCard);
            } else {
                // Mục bình thường: nền trắng trong suốt, hover xám nhạt, chữ đậm
                ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0, 0, 0, 0));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, GUI::Theme::Colors::GrayHover);
                ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.88f, 0.90f, 0.92f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_Text,          GUI::Theme::Colors::DarkText);
            }

            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
            std::string btnId = itemLabel + "##side_" + item.id;
            if (ImGui::Button(btnId.c_str(), itemSize)) {
                mucDangChon = i;
                changed = true;
            }
            ImGui::PopStyleVar();
            ImGui::PopStyleColor(4);

            ImGui::Spacing();
        }

        // ==========================================
        // 3. NÚT ĐĂNG XUẤT ĐẶT DƯỚI ĐÁY
        // ==========================================
        float windowHeight = ImGui::GetWindowHeight();
        float logoutBtnHeight = 38.0f;
        float bottomPadding = 20.0f;
        float logoutPosY = windowHeight - logoutBtnHeight - bottomPadding;

        if (logoutPosY > ImGui::GetCursorPosY() + 20.0f) {
            ImGui::SetCursorPosY(logoutPosY);
        }

        ImGui::PushStyleColor(ImGuiCol_Separator, GUI::Theme::Colors::BorderLight);
        ImGui::Separator();
        ImGui::PopStyleColor();
        ImGui::Spacing();

        if (NutNguyHiem(u8"Đăng xuất", ImVec2(ImGui::GetContentRegionAvail().x, logoutBtnHeight))) {
            if (onDangXuat) {
                onDangXuat();
            }
        }
    }
    ImGui::EndChild();

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(2);

    return changed;
}

}  // namespace Widgets
