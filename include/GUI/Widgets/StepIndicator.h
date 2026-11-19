#pragma once

#include <string>
#include <vector>

namespace Widgets {

// Thanh các bước có nhãn riêng cho từng bước (thay cho ProgressBar chỉ có một nhãn chung).
// buocHienTai tính từ 1: bước nhỏ hơn đã xong (dấu tick), bằng là bước hiện tại, lớn hơn là chưa tới.
// LÝ DO CHỌN: vẽ bằng ImDrawList (số, dấu tick, vòng tròn) để không phụ thuộc glyph của font.
void ThanhCacBuoc(const std::vector<std::string>& nhanCacBuoc, int buocHienTai);

}  // namespace Widgets
