#pragma once

struct ImFont;

namespace GUI::Theme {

// LÝ DO CHỌN: Phân cấp Typography rõ ràng theo chuẩn Modern Desktop App:
// - regular (15px): nội dung thông thường, bảng, form nhập liệu, button chuẩn.
// - heading (24px): tiêu đề màn hình lớn, banner thương hiệu.
// - title (18px): tiêu đề khu vực, card header, tiêu đề dialog modal.
// - small (13px): nhãn phụ, trạng thái badge, thông tin ghi chú footer, timestamp.
struct Fonts {
    ImFont* regular = nullptr;
    ImFont* heading = nullptr;
    ImFont* title = nullptr;
    ImFont* small = nullptr;
};

// Nạp bộ font UTF-8 tiếng Việt (Segoe UI / Roboto / Arial).
Fonts loadFonts();

// Lấy tham chiếu đến bộ font đã nạp để sử dụng trong toàn bộ ứng dụng.
const Fonts& getFonts();

}  // namespace GUI::Theme
