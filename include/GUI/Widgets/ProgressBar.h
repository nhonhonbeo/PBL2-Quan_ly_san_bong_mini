#pragma once

#include <imgui.h>
#include "GUI/Theme/Theme.h"

namespace Widgets {

// LÝ DO CHỌN: ProgressBar tùy biến với nhãn phần trăm hiển thị trực quan ở giữa thanh,
// dùng cho tỉ lệ lấp đầy sân bóng trong ngày, tiến độ giải đấu hoặc tiến độ tích điểm ưu đãi.
void ProgressBar(
    float tyLe,
    const char* nhanTuyChon = nullptr,
    const ImVec2& kichThuoc = ImVec2(-1.0f, 30.0f),
    const ImVec4& mauThanh = GUI::Theme::Colors::GrassGreen
);

}  // namespace Widgets
