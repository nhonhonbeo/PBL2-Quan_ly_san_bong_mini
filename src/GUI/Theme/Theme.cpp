#include "GUI/Theme/Theme.h"

namespace GUI::Theme {

void apply() {
    ImGuiStyle& style = ImGui::GetStyle();

    // ==========================================
    // 1. BO GÓC (Rounding)
    // ==========================================
    // LÝ DO CHỌN: Bo góc 8px tạo cảm giác mềm mại, hiện đại theo chuẩn Fluent/Material UI
    style.WindowRounding    = 8.0f;
    style.ChildRounding     = 8.0f;
    style.FrameRounding     = 8.0f;
    style.PopupRounding     = 8.0f;
    style.ScrollbarRounding = 6.0f;
    style.GrabRounding      = 6.0f;
    style.TabRounding       = 8.0f;

    // ==========================================
    // 2. KHOẢNG CÁCH VÀ ĐỆM (Padding & Spacing)
    // ==========================================
    // LÝ DO CHỌN: Padding rộng giúp các phần tử dễ nhìn, không bị chật chội
    style.WindowPadding     = ImVec2(16.0f, 16.0f);
    style.FramePadding      = ImVec2(12.0f, 8.0f);
    style.CellPadding       = ImVec2(10.0f, 8.0f);
    style.ItemSpacing       = ImVec2(10.0f, 10.0f);
    style.ItemInnerSpacing  = ImVec2(8.0f, 6.0f);
    style.TouchExtraPadding = ImVec2(0.0f, 0.0f);
    style.IndentSpacing     = 20.0f;

    // LÝ DO CHỌN: Thanh cuộn mảnh 8px để giao diện thanh thoát, không chiếm diện tích nội dung
    style.ScrollbarSize     = 8.0f;
    style.GrabMinSize       = 10.0f;

    // Đường viền tinh tế
    style.WindowBorderSize  = 1.0f;
    style.ChildBorderSize   = 1.0f;
    style.PopupBorderSize   = 1.0f;
    style.FrameBorderSize   = 1.0f;
    style.TabBorderSize     = 0.0f;

    // ==========================================
    // 3. THIẾT LẬP MÀU SẮC (Color Palette)
    // ==========================================
    ImVec4* colors = style.Colors;

    // Màu nền và chữ
    colors[ImGuiCol_Text]                  = Colors::DarkText;
    colors[ImGuiCol_TextDisabled]          = Colors::Gray;
    colors[ImGuiCol_WindowBg]              = Colors::LightBg;
    colors[ImGuiCol_ChildBg]               = Colors::WhiteCard;
    colors[ImGuiCol_PopupBg]               = Colors::WhiteCard;
    colors[ImGuiCol_Border]                = Colors::BorderLight;
    colors[ImGuiCol_BorderShadow]          = ImVec4(0.0f, 0.0f, 0.0f, 0.04f);

    // Khung nhập liệu (Input, Combobox, Checkbox)
    colors[ImGuiCol_FrameBg]               = Colors::WhiteCard;
    colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.96f, 0.98f, 0.96f, 1.0f);
    colors[ImGuiCol_FrameBgActive]         = ImVec4(0.92f, 0.96f, 0.93f, 1.0f);

    // Tiêu đề cửa sổ
    colors[ImGuiCol_TitleBg]               = Colors::LightBg;
    colors[ImGuiCol_TitleBgActive]         = Colors::LightBg;
    colors[ImGuiCol_TitleBgCollapsed]      = Colors::LightBg;
    colors[ImGuiCol_MenuBarBg]             = Colors::WhiteCard;

    // Thanh cuộn mảnh
    colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.95f, 0.96f, 0.95f, 0.5f);
    colors[ImGuiCol_ScrollbarGrab]         = Colors::Gray;
    colors[ImGuiCol_ScrollbarGrabHovered]  = Colors::GrassGreen;
    colors[ImGuiCol_ScrollbarGrabActive]   = Colors::DarkGreen;

    // Nút bấm mặc định (Xanh cỏ #1B8A5A)
    colors[ImGuiCol_Button]                = Colors::GrassGreen;
    colors[ImGuiCol_ButtonHovered]         = ImVec4(0.13f, 0.62f, 0.40f, 1.0f);
    colors[ImGuiCol_ButtonActive]          = Colors::DarkGreen;

    // Checkbox & Radio & Slider
    colors[ImGuiCol_CheckMark]             = Colors::GrassGreen;
    colors[ImGuiCol_SliderGrab]            = Colors::GrassGreen;
    colors[ImGuiCol_SliderGrabActive]      = Colors::DarkGreen;

    // Header & Danh sách lựa chọn
    colors[ImGuiCol_Header]                = ImVec4(0.106f, 0.541f, 0.353f, 0.12f);
    colors[ImGuiCol_HeaderHovered]         = ImVec4(0.106f, 0.541f, 0.353f, 0.22f);
    colors[ImGuiCol_HeaderActive]          = ImVec4(0.106f, 0.541f, 0.353f, 0.32f);

    // Phân cách (Separator)
    colors[ImGuiCol_Separator]             = Colors::BorderLight;
    colors[ImGuiCol_SeparatorHovered]      = Colors::GrassGreen;
    colors[ImGuiCol_SeparatorActive]       = Colors::DarkGreen;

    // Resize Grip & Tabs
    colors[ImGuiCol_ResizeGrip]            = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    colors[ImGuiCol_ResizeGripHovered]     = Colors::GrassGreen;
    colors[ImGuiCol_ResizeGripActive]      = Colors::DarkGreen;
    colors[ImGuiCol_Tab]                   = ImVec4(0.92f, 0.94f, 0.93f, 1.0f);
    colors[ImGuiCol_TabHovered]            = ImVec4(0.106f, 0.541f, 0.353f, 0.25f);
    colors[ImGuiCol_TabActive]             = Colors::GrassGreen;
    colors[ImGuiCol_TabUnfocused]          = ImVec4(0.95f, 0.95f, 0.95f, 1.0f);
    colors[ImGuiCol_TabUnfocusedActive]    = ImVec4(0.88f, 0.90f, 0.89f, 1.0f);

    // Bảng dữ liệu (Table)
    // LÝ DO CHỌN: Tô màu nền hàng xen kẽ và viền bảng nhẹ giúp dễ đọc bảng dữ liệu lớn
    colors[ImGuiCol_TableHeaderBg]         = ImVec4(0.92f, 0.95f, 0.93f, 1.0f);
    colors[ImGuiCol_TableBorderStrong]     = Colors::BorderLight;
    colors[ImGuiCol_TableBorderLight]      = ImVec4(0.90f, 0.92f, 0.91f, 0.6f);
    colors[ImGuiCol_TableRowBg]            = Colors::WhiteCard;
    colors[ImGuiCol_TableRowBgAlt]         = ImVec4(0.97f, 0.98f, 0.975f, 1.0f);

    // Khác
    colors[ImGuiCol_TextSelectedBg]        = ImVec4(0.106f, 0.541f, 0.353f, 0.35f);
    colors[ImGuiCol_DragDropTarget]        = Colors::GoldAccent;
    colors[ImGuiCol_NavHighlight]          = Colors::GrassGreen;
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg]     = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg]      = ImVec4(0.12f, 0.16f, 0.21f, 0.45f);
}

}  // namespace GUI::Theme
