#pragma once

#include <imgui.h>

namespace GUI::Theme {

// LÝ DO CHỌN: Gom toàn bộ bảng màu chuẩn của dự án theo chủ đề sân bóng mini vào struct Colors.
// Giúp thống nhất màu sắc toàn bộ ứng dụng, dễ tái sử dụng và tránh mã màu ma thuật rải rác.
struct Colors {
    // Màu chủ đạo & thương hiệu sân bóng
    static constexpr ImVec4 GrassGreen = ImVec4(0.106f, 0.541f, 0.353f, 1.0f); // #1B8A5A - Xanh cỏ chính
    static constexpr ImVec4 DarkGreen  = ImVec4(0.059f, 0.361f, 0.235f, 1.0f); // #0F5C3C - Xanh đậm
    static constexpr ImVec4 LightBg    = ImVec4(0.886f, 0.910f, 0.941f, 1.0f); // #E2E8F0 - Nền sáng
    static constexpr ImVec4 WhiteCard  = ImVec4(1.000f, 1.000f, 1.000f, 1.0f); // #FFFFFF - Thẻ trắng
    static constexpr ImVec4 DarkText   = ImVec4(0.122f, 0.161f, 0.216f, 1.0f); // #1F2937 - Chữ đậm
    static constexpr ImVec4 GoldAccent = ImVec4(0.961f, 0.702f, 0.004f, 1.0f); // #F5B301 - Vàng nhấn
    static constexpr ImVec4 RedDanger  = ImVec4(0.863f, 0.208f, 0.271f, 1.0f); // #DC3545 - Đỏ cảnh báo/nguy hiểm
    static constexpr ImVec4 Gray       = ImVec4(0.612f, 0.639f, 0.686f, 1.0f); // #9CA3AF - Xám trung tính

    // Màu bổ trợ cho đường viền, hiệu ứng hover và thông báo
    static constexpr ImVec4 BorderLight = ImVec4(0.898f, 0.906f, 0.922f, 1.0f); // #E5E7EB - Viền nhẹ
    static constexpr ImVec4 InputBorder = ImVec4(0.580f, 0.627f, 0.694f, 1.0f); // LÝ DO CHỌN: Tăng tương phản viền ô nhập trên nền trắng.
    static constexpr ImVec4 GrayHover   = ImVec4(0.941f, 0.949f, 0.961f, 1.0f); // #F0F2F5 - Nền hover thẻ/dòng
    static constexpr ImVec4 MutedText   = ImVec4(0.420f, 0.447f, 0.502f, 1.0f); // #6B7280 - Chữ phụ/mờ
    static constexpr ImVec4 BlueInfo    = ImVec4(0.008f, 0.518f, 0.780f, 1.0f); // #0284C7 - Xanh dương thông tin

    // LÝ DO CHỌN: Hàm tiện ích chuyển đổi nhanh ImVec4 sang ImU32 cho ImDrawList
    static ImU32 toU32(const ImVec4& color, float alphaMultiplier = 1.0f) {
        ImVec4 c = color;
        c.w *= alphaMultiplier;
        return ImGui::ColorConvertFloat4ToU32(c);
    }
};

// LÝ DO CHỌN: Cấu hình phong cách ImGuiStyle toàn cục với bo góc 8px, padding rộng rãi,
// màu sắc dịu mắt hiện đại theo phong cách ứng dụng desktop chuyên nghiệp.
void apply();

}  // namespace GUI::Theme
