#pragma once

#include <string>
#include <vector>

namespace Widgets {

// LÝ DO CHỌN: Hộp chọn Combo và Radio có nhãn rõ ràng, thiết kế nhất quán với hệ thống Theme
// hỗ trợ thông báo lỗi và căn chỉnh ngang/dọc linh hoạt.

// Hộp thả xuống chọn 1 mục (Combobox)
bool Combo(
    const char* label,
    const char* id,
    int& selectedIndex,
    const std::vector<std::string>& items,
    const char* loi = nullptr
);

// Nút chọn một Radio đơn lẻ
bool Radio(const char* label, int& selectedValue, int value);

// Nhóm các nút Radio (hỗ trợ hiển thị theo hàng ngang hoặc cột dọc)
bool RadioGroup(
    const char* label,
    const char* id,
    int& selectedValue,
    const std::vector<std::pair<int, std::string>>& options,
    bool ngang = true
);

}  // namespace Widgets
