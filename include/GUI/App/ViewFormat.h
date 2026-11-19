#pragma once
// Hàm định dạng văn bản hiển thị dùng chung cho các màn hình.
// LÝ DO CHỌN: trước đây dinhDangTien / ngayHienThi bị sao chép ở 4 màn hình, mỗi bản
// một kiểu; gom về đây để sửa định dạng một lần là đúng ở mọi nơi.
#include <string>

#include "Core/DateTime.h"

namespace GUI::App::ViewFormat {

// 1234567 -> "1.234.567" (không kèm đơn vị; xử lý đúng cả số âm).
inline std::string nhomNghin(long long value) {
    const bool am = value < 0;
    std::string digits = std::to_string(am ? -value : value);
    for (int index = static_cast<int>(digits.size()) - 3; index > 0; index -= 3) {
        digits.insert(static_cast<std::size_t>(index), ".");
    }
    return am ? "-" + digits : digits;
}

// 1234567 -> "1.234.567 VNĐ"
inline std::string dinhDangTien(long long value) {
    return nhomNghin(value) + u8" VNĐ";
}

// "2026-10-20" -> "20/10/2026"; chuỗi không phải ngày ISO được giữ nguyên.
inline std::string ngayHienThi(const std::string& isoDate) {
    if (!DateTime::isValidDate(isoDate)) return isoDate;
    return isoDate.substr(8, 2) + "/" + isoDate.substr(5, 2) + "/" + isoDate.substr(0, 4);
}

}  // namespace GUI::App::ViewFormat
