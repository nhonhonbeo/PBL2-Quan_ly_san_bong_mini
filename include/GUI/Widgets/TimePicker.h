#pragma once

#include <string>

namespace Widgets {

// LÝ DO CHỌN: TimePicker chuyên dụng cho việc chọn khung giờ đặt sân (giờ:phút),
// vừa cho phép gõ trực tiếp vừa có popup chọn nhanh giờ và phút trực quan.
bool TimePicker(
    const char* label,
    const char* id,
    std::string& timeStr,
    const char* loi = nullptr
);

}  // namespace Widgets
