#include "GUI/Widgets/Sidebar.h"

#include <imgui.h>
#include <cmath>
#include "GUI/Theme/Fonts.h"
#include "GUI/Theme/Theme.h"
#include "GUI/Widgets/Button.h"

namespace Widgets {

namespace {
void navIcon(ImDrawList* dl, ImVec2 p, const std::string& type, ImU32 c) {
    auto at = [p](float x,float y){return ImVec2(p.x+x,p.y+y);};
    auto line = [&](float x,float y,float xx,float yy){dl->AddLine(at(x,y),at(xx,yy),c,1.5f);};
    auto rect = [&](float x,float y,float xx,float yy){dl->AddRect(at(x,y),at(xx,yy),c,2,0,1.4f);};
    if(type=="dashboard") {rect(0,0,6,6);rect(9,0,15,6);rect(0,9,6,15);rect(9,9,15,15);}
    else if(type=="pitch") {rect(0,2,16,14);line(8,2,8,14);dl->AddCircle(at(8,8),3,c,12,1.3f);}
    else if(type=="plus") {rect(0,0,16,16);line(4,8,12,8);line(8,4,8,12);}
    else if(type=="calendar") {rect(0,2,16,16);line(0,6,16,6);line(4,0,4,4);line(12,0,12,4);}
    else if(type=="users") {dl->AddCircle(at(8,4),3,c,16,1.4f);dl->AddRect(at(2,9),at(14,15),c,4,0,1.4f);}
    else if(type=="trophy") {rect(4,0,12,9);line(4,2,0,2);line(0,2,0,6);line(0,6,4,6);line(12,2,16,2);line(16,2,16,6);line(16,6,12,6);line(8,9,8,14);line(3,15,13,15);}
    else if(type=="logout") {rect(0,1,9,15);line(6,8,16,8);line(12,4,16,8);line(12,12,16,8);}
    else if(type=="star") {ImVec2 points[10];for(int j=0;j<10;++j){float a=-1.5708f+j*.628319f;float rad=j%2?3.5f:8.f;points[j]=at(8+std::cos(a)*rad,8+std::sin(a)*rad);}dl->AddPolyline(points,10,c,ImDrawFlags_Closed,1.4f);}
}
}


bool Sidebar(
    const std::vector<SidebarItem>& danhSachMuc,
    int& mucDangChon,
    float chieuRong,
    const std::function<void()>& onDangXuat
) {
    bool changed = false;

    const ImVec4 inkLight(0.94f,0.98f,0.96f,1), inkMuted(0.76f,0.86f,0.82f,1);
    const ImVec4 bar(0.071f,0.224f,0.173f,1), hover(0.15f,0.32f,0.25f,1), active(0.12f,0.43f,0.30f,1);
    ImGui::PushStyleColor(ImGuiCol_ChildBg, bar);
    ImGui::PushStyleColor(ImGuiCol_Border, bar);
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

        ImFont* titleFont = GUI::Theme::getFonts().regular;
        if (titleFont != nullptr) ImGui::PushFont(titleFont);
        ImGui::PushStyleColor(ImGuiCol_Text, inkLight);
        ImGui::TextUnformatted(u8"Sân bóng mini");
        ImGui::PopStyleColor();
        if (titleFont != nullptr) ImGui::PopFont();

        ImFont* smallFont = GUI::Theme::getFonts().small;
        if (smallFont != nullptr) ImGui::PushFont(smallFont);
        ImGui::PushStyleColor(ImGuiCol_Text, inkMuted);
        ImGui::TextUnformatted(u8"Quản lý sân bóng");
        ImGui::PopStyleColor();
        if (smallFont != nullptr) ImGui::PopFont();

        ImGui::EndGroup();

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8.0f);
        ImGui::PushStyleColor(ImGuiCol_Separator, hover);
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

            std::string itemLabel = item.tieuDe;

            ImVec2 itemPos = ImGui::GetCursorScreenPos();
            ImVec2 itemSize(ImGui::GetContentRegionAvail().x, 46.0f);

            if (isSelected) {
                // Mục đang chọn: nền xanh nhạt hiện đại (#E8F6EF), chữ xanh đậm (#10543C)
                ImGui::PushStyleColor(ImGuiCol_Button,        active);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, active);
                ImGui::PushStyleColor(ImGuiCol_ButtonActive,  active);
                ImGui::PushStyleColor(ImGuiCol_Text,          inkLight);
            } else {
                // Mục bình thường: nền trong suốt, hover xám nhạt, chữ đậm
                ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0, 0, 0, 0));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hover);
                ImGui::PushStyleColor(ImGuiCol_ButtonActive,  active);
                ImGui::PushStyleColor(ImGuiCol_Text,          inkLight);
            }

            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.0f, 0.5f)); // Căn chữ lệch trái tự nhiên

            std::string btnId = "##side_" + item.id;
            if (ImGui::Button(btnId.c_str(), itemSize)) {
                mucDangChon = i;
                changed = true;
            }

            const ImU32 ink = GUI::Theme::Colors::toU32(isSelected ? inkLight : inkMuted);
            navIcon(drawList, ImVec2(itemPos.x+12,itemPos.y+(itemSize.y-16)*.5f), item.bieuTuong, ink);
            drawList->AddText(ImVec2(itemPos.x+36,itemPos.y+(itemSize.y-ImGui::GetTextLineHeight())*.5f), ink, itemLabel.c_str());
            // Vẽ vạch chỉ báo bên trái cho mục đang chọn
            if (isSelected) {
                drawList->AddRectFilled(
                    ImVec2(itemPos.x, itemPos.y + 6.0f),
                    ImVec2(itemPos.x + 3.5f, itemPos.y + itemSize.y - 6.0f),
                    GUI::Theme::Colors::toU32(inkLight),
                    2.0f
                );
            }

            ImGui::PopStyleVar(3);
            ImGui::PopStyleColor(4);

            if (ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        }

        // ==========================================
        // 3. NÚT ĐĂNG XUẤT ĐẶT DƯỚI ĐÁY
        // ==========================================
        float windowHeight = ImGui::GetWindowHeight();
        float logoutBtnHeight = 46.0f;
        float bottomPadding = 16.0f;
        float logoutPosY = windowHeight - logoutBtnHeight - bottomPadding;

        if (logoutPosY > ImGui::GetCursorPosY() + 20.0f) {
            ImGui::SetCursorPosY(logoutPosY - 24.0f);
        }

        ImGui::PushStyleColor(ImGuiCol_Separator, hover);
        ImGui::Separator();
        ImGui::PopStyleColor();
        ImGui::Spacing();

        // LÝ DO CHỌN: Dùng Ghost Button chữ xám tinh tế (#748078) thay vì nút đỏ cảnh báo,
        // giúp người dùng tập trung vào các chức năng chính, tránh tâm lý mời người dùng rời app.
        ImGui::PushStyleColor(ImGuiCol_Button,        hover);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hover);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  active);
        ImGui::PushStyleColor(ImGuiCol_Text,          inkLight);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.12f, 0.5f));
        const ImVec2 logoutStart = ImGui::GetCursorScreenPos();
        if (ImGui::Button(u8"    Đăng xuất##sidebar_logout_btn", ImVec2(ImGui::GetContentRegionAvail().x, logoutBtnHeight))) {
            if (onDangXuat) {
                onDangXuat();
            }
        }
        navIcon(drawList, ImVec2(logoutStart.x+12, logoutStart.y+15), "logout", IM_COL32(255,183,183,255));
        if (ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        ImGui::PopStyleVar(3);
        ImGui::PopStyleColor(4);
    }
    ImGui::EndChild();

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(2);

    return changed;
}

}  // namespace Widgets
