#pragma once

namespace Widgets {

// LÝ DO CHỌN: StarRating tự vẽ hình ngôi sao 5 cánh sắc nét bằng ImDrawList,
// hỗ trợ cả 2 chế độ: người dùng tương tác chấm điểm hoặc chế độ chỉ xem đánh giá sân.
bool StarRating(
    const char* label,
    const char* id,
    int& giaTri,
    bool chiXem = false,
    float kichThuocSao = 22.0f
);

}  // namespace Widgets
