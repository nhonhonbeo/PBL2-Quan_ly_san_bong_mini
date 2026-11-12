#pragma once

#include <imgui.h>
#include "GUI/Theme/Theme.h"

namespace Widgets {

// LÝ DO CHỌN: ProgressBar chia 4 thanh nhỏ đại diện cho 4 bước tiến độ,
// không hiển thị ký hiệu %, giúp người dùng dễ dàng theo dõi từng giai đoạn trực quan.
void ProgressBar(
    float tyLe,
    const char* nhanTuyChon = nullptr,
    const ImVec2& kichThuoc = ImVec2(-1.0f, 30.0f),
    const ImVec4& mauThanh = GUI::Theme::Colors::GrassGreen
);

}  // namespace Widgets