#pragma once

struct ImFont;

namespace GUI::Theme {

// LÝ DO CHỌN: Phân cấp font làm 3 kích thước:
// - regular (18px): dùng cho nội dung, văn bản đọc thông thường, form nhập liệu.
// - heading (28px): dùng cho tiêu đề lớn, banner phần mềm.
// - small (14px): dùng cho ghi chú phụ, trạng thái, footer.
struct Fonts {
    ImFont* regular = nullptr;
    ImFont* heading = nullptr;
    ImFont* small = nullptr;
};

// Nạp bộ font UTF-8 tiếng Việt; trả về font mặc định nếu không tìm thấy file.
Fonts loadFonts();

}  // namespace GUI::Theme
