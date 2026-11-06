#include "GUI/Theme/Theme.h"

namespace GUI::Theme {

void apply() {
    ImGuiStyle& style = ImGui::GetStyle();

    // ==========================================
    // 1. BO GÓC (Rounding)
    // ==========================================
    // LÝ DO CHỌN: Bo góc tinh tế 6-8px theo chuẩn Modern SaaS Desktop,
    // tạo sự thanh lịch, chuyên nghiệp, không bo góc quá đà.
    style.WindowRounding    = 8.0f;
    style.ChildRounding     = 8.0f;
    style.FrameRounding     = 6.0f;
    style.PopupRounding     = 8.0f;
    style.ScrollbarRounding = 6.0f;
    style.GrabRounding      = 6.0f;
    style.TabRounding       = 6.0f;

    // ==========================================
    // 2. KHOẢNG CÁCH VÀ ĐỆM (Padding & Spacing)
    // ==========================================
    // LÝ DO CHỌN: Hệ thống khoảng cách chuẩn bội số 4-8px (8pt grid system),
    // đảm bảo phân cấp rõ ràng, dễ nhìn, không chen chúc.
    style.WindowPadding     = ImVec2(20.0f, 18.0f);
    style.FramePadding      = ImVec2(12.0f, 8.0f);
    style.ButtonTextAlign   = ImVec2(0.5f, 0.5f);
    style.CellPadding       = ImVec2(10.0f, 8.0f);
    style.ItemSpacing       = ImVec2(10.0f, 8.0f);
    style.ItemInnerSpacing  = ImVec2(8.0f, 6.0f);
    style.TouchExtraPadding = ImVec2(0.0f, 0.0f);
    style.IndentSpacing     = 18.0f;

    // Thanh cuộn thanh mảnh 8px
    style.ScrollbarSize     = 8.0f;
    style.GrabMinSize       = 10.0f;

    // Đường viền tinh tế 1.0px
    style.WindowBorderSize  = 1.0f;
    style.ChildBorderSize   = 1.0f;
    style.PopupBorderSize   = 1.0f;
    style.FrameBorderSize   = 1.0f;
    style.TabBorderSize     = 1.0f;

    // ==========================================
    // 3. THIẾT LẬP MÀU SẮC (Color Palette)
    // ==========================================
    ImVec4* colors = style.Colors;

    // Nền và chữ
    colors[ImGuiCol_Text]                  = Colors::TextPrimary;
    colors[ImGuiCol_TextDisabled]          = Colors::TextSecondary;
    colors[ImGuiCol_WindowBg]              = Colors::Background;
    colors[ImGuiCol_ChildBg]               = Colors::Surface;
    colors[ImGuiCol_PopupBg]               = Colors::Surface;
    colors[ImGuiCol_Border]                = Colors::Border;
    colors[ImGuiCol_BorderShadow]          = ImVec4(0.0f, 0.0f, 0.0f, 0.03f);

    // Tab bar hiện đại: Tab active nền trắng Surface (#FFFFFF) thanh lịch, viền chỉ báo xanh lá thương hiệu (#16865B) trên đỉnh; Tab inactive nền xám lục nhạt tương phản tốt
    colors[ImGuiCol_Tab]                       = ImVec4(0.900f, 0.930f, 0.915f, 0.75f);
    colors[ImGuiCol_TabHovered]                = ImVec4(0.820f, 0.900f, 0.860f, 0.90f);
    colors[ImGuiCol_TabActive]                 = Colors::Surface;
    colors[ImGuiCol_TabSelected]               = Colors::Surface;
    colors[ImGuiCol_TabSelectedOverline]       = Colors::Primary;
    colors[ImGuiCol_TabDimmed]                 = ImVec4(0.900f, 0.930f, 0.915f, 0.75f);
    colors[ImGuiCol_TabDimmedSelected]         = Colors::Surface;
    colors[ImGuiCol_TabDimmedSelectedOverline] = Colors::Primary;

    // Frame ô nhập liệu
    colors[ImGuiCol_FrameBg]               = Colors::Surface;
    colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.975f, 0.985f, 0.978f, 1.0f);
    colors[ImGuiCol_FrameBgActive]         = ImVec4(0.940f, 0.975f, 0.950f, 1.0f);

    // Tiêu đề cửa sổ
    colors[ImGuiCol_TitleBg]               = Colors::Background;
    colors[ImGuiCol_TitleBgActive]         = Colors::Background;
    colors[ImGuiCol_TitleBgCollapsed]      = Colors::Background;
    colors[ImGuiCol_MenuBarBg]             = Colors::Surface;

    // Thanh cuộn
    colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.95f, 0.96f, 0.96f, 0.5f);
    colors[ImGuiCol_ScrollbarGrab]         = Colors::InputBorder;
    colors[ImGuiCol_ScrollbarGrabHovered]  = Colors::Primary;
    colors[ImGuiCol_ScrollbarGrabActive]   = Colors::PrimaryDark;

    // Nút bấm mặc định (Xanh lá nhận diện #16865B)
    colors[ImGuiCol_Button]                = Colors::Primary;
    colors[ImGuiCol_ButtonHovered]         = Colors::PrimaryHover;
    colors[ImGuiCol_ButtonActive]          = Colors::PrimaryDark;

    // Checkbox & Radio & Slider
    colors[ImGuiCol_CheckMark]             = Colors::Primary;
    colors[ImGuiCol_SliderGrab]            = Colors::Primary;
    colors[ImGuiCol_SliderGrabActive]      = Colors::PrimaryDark;

    // Header & Danh sách lựa chọn
    colors[ImGuiCol_Header]                = ImVec4(0.086f, 0.525f, 0.357f, 0.10f);
    colors[ImGuiCol_HeaderHovered]         = ImVec4(0.086f, 0.525f, 0.357f, 0.18f);
    colors[ImGuiCol_HeaderActive]          = ImVec4(0.086f, 0.525f, 0.357f, 0.26f);

    // Phân cách (Separator)
    colors[ImGuiCol_Separator]             = Colors::Border;
    colors[ImGuiCol_SeparatorHovered]      = Colors::Primary;
    colors[ImGuiCol_SeparatorActive]       = Colors::PrimaryDark;

    // Resize Grip
    colors[ImGuiCol_ResizeGrip]            = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    colors[ImGuiCol_ResizeGripHovered]     = Colors::Primary;
    colors[ImGuiCol_ResizeGripActive]      = Colors::PrimaryDark;

    // Bảng dữ liệu (Table)
    colors[ImGuiCol_TableHeaderBg]         = ImVec4(0.933f, 0.953f, 0.941f, 1.0f); // #EEF3F0
    colors[ImGuiCol_TableBorderStrong]     = Colors::Border;
    colors[ImGuiCol_TableBorderLight]      = ImVec4(0.910f, 0.930f, 0.920f, 0.7f);
    colors[ImGuiCol_TableRowBg]            = Colors::Surface;
    colors[ImGuiCol_TableRowBgAlt]         = ImVec4(0.976f, 0.984f, 0.980f, 1.0f);

    // Khác
    colors[ImGuiCol_TextSelectedBg]        = ImVec4(0.086f, 0.525f, 0.357f, 0.25f);
    colors[ImGuiCol_DragDropTarget]        = Colors::Warning;
    colors[ImGuiCol_NavHighlight]          = Colors::Primary;
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg]     = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg]      = ImVec4(0.094f, 0.169f, 0.141f, 0.45f);
}

}  // namespace GUI::Theme
