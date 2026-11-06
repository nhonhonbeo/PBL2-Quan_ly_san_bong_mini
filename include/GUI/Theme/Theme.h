#pragma once

#include <imgui.h>

namespace GUI::Theme {

// LÝ DO CHỌN: Design System chuẩn hóa toàn bộ bảng màu theo nhận diện Modern Desktop SaaS sân bóng mini.
// Tương thích 100% với các tên màu cũ và cung cấp đầy đủ các biến màu theo tài liệu đặc tả thiết kế.
struct Colors {
    // 1. Màu nhận diện thương hiệu & Hành động chính (Primary)
    static constexpr ImVec4 Primary      = ImVec4(0.086f, 0.525f, 0.357f, 1.0f); // #16865B - Xanh lá thương hiệu
    static constexpr ImVec4 PrimaryHover = ImVec4(0.067f, 0.420f, 0.282f, 1.0f); // #116B48 - Xanh lá khi hover
    static constexpr ImVec4 PrimaryDark  = ImVec4(0.063f, 0.329f, 0.235f, 1.0f); // #10543C - Xanh lá đậm khi click
    static constexpr ImVec4 PrimaryLight = ImVec4(0.910f, 0.965f, 0.937f, 1.0f); // #E8F6EF - Xanh lá nhạt nền active/badge

    // Bí danh tương thích ngược
    static constexpr ImVec4 GrassGreen   = Primary;
    static constexpr ImVec4 DarkGreen    = PrimaryDark;

    // 2. Nền & Bề mặt (Background & Surface)
    static constexpr ImVec4 Background   = ImVec4(0.953f, 0.965f, 0.973f, 1.0f); // #F3F6F8 - Nền ứng dụng sáng dịu
    static constexpr ImVec4 LightBg      = Background;
    static constexpr ImVec4 Surface      = ImVec4(1.000f, 1.000f, 1.000f, 1.0f); // #FFFFFF - Bề mặt card/panel
    static constexpr ImVec4 WhiteCard    = Surface;

    // 3. Văn bản & Phân cấp thị giác (Typography Colors)
    static constexpr ImVec4 TextPrimary   = ImVec4(0.094f, 0.169f, 0.141f, 1.0f); // #182B24 - Chữ chính đậm nét
    static constexpr ImVec4 DarkText      = TextPrimary;
    static constexpr ImVec4 TextSecondary = ImVec4(0.455f, 0.502f, 0.471f, 1.0f); // #748078 - Chữ phụ/nhãn/breadcrumb
    static constexpr ImVec4 MutedText     = TextSecondary;
    static constexpr ImVec4 Gray          = ImVec4(0.612f, 0.639f, 0.686f, 1.0f); // #9CA3AF - Xám vô hiệu hóa

    // 4. Đường viền & Tương tác (Borders & Hover)
    static constexpr ImVec4 Border        = ImVec4(0.886f, 0.910f, 0.898f, 1.0f); // #E2E8E5 - Viền chuẩn
    static constexpr ImVec4 BorderLight   = Border;
    static constexpr ImVec4 InputBorder   = ImVec4(0.824f, 0.863f, 0.843f, 1.0f); // #D2DCD7 - Viền ô nhập tương phản tốt
    static constexpr ImVec4 GrayHover     = ImVec4(0.973f, 0.980f, 0.976f, 1.0f); // #F8FAF9 - Nền hover nhẹ

    // 5. Trạng thái ngữ nghĩa (Status / Semantic Colors)
    static constexpr ImVec4 Success       = ImVec4(0.086f, 0.639f, 0.416f, 1.0f); // #16A36A - Thành công / Trống
    static constexpr ImVec4 Warning       = ImVec4(0.914f, 0.635f, 0.231f, 1.0f); // #E9A23B - Cảnh báo / Chờ duyệt
    static constexpr ImVec4 GoldAccent    = Warning;
    static constexpr ImVec4 Danger        = ImVec4(0.863f, 0.322f, 0.322f, 1.0f); // #DC5252 - Lỗi / Nguy hiểm / Đã hủy
    static constexpr ImVec4 RedDanger     = Danger;
    static constexpr ImVec4 Info          = ImVec4(0.231f, 0.510f, 0.769f, 1.0f); // #3B82C4 - Thông tin / Đã duyệt
    static constexpr ImVec4 BlueInfo      = Info;

    // LÝ DO CHỌN: Hàm tiện ích chuyển đổi nhanh ImVec4 sang ImU32 cho ImDrawList
    static ImU32 toU32(const ImVec4& color, float alphaMultiplier = 1.0f) {
        ImVec4 c = color;
        c.w *= alphaMultiplier;
        return ImGui::ColorConvertFloat4ToU32(c);
    }
};

// LÝ DO CHỌN: Cấu hình ImGuiStyle với bo góc tinh tế 6-8px, padding rộng rãi chuẩn SaaS hiện đại.
void apply();

}  // namespace GUI::Theme
