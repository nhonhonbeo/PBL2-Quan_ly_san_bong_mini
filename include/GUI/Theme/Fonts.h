#pragma once

struct ImFont;

namespace GUI::Theme {

struct Fonts {
    ImFont* regular = nullptr;
    ImFont* heading = nullptr;
    ImFont* small = nullptr;
};

// Nạp bộ font UTF-8 tiếng Việt; trả về font mặc định nếu không tìm thấy file.
Fonts loadFonts();

}  // namespace GUI::Theme
