#pragma once

#include <string>

namespace Widgets {

// LÝ DO CHỌN: DatePicker tự vẽ popup lịch theo chuẩn giao diện desktop, hỗ trợ người dùng
// vừa có thể gõ trực tiếp dd/MM/yyyy vừa có thể mở lịch trực quan để chọn ngày chính xác.
bool DatePicker(
    const char* label,
    const char* id,
    std::string& dateStr,
    const char* loi = nullptr,
    bool choPhepNgayQuaKhu = false
);

}  // namespace Widgets
