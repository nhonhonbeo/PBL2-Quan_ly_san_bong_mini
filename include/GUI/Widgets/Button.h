#pragma once

#include <imgui.h>

namespace Widgets {

// LÝ DO CHỌN: Phân cấp 3 kiểu nút rõ rệt (Chính, Phụ, Nguy hiểm) giúp người dùng nhận diện nhanh
// hành động ưu tiên và giảm thiểu nguy cơ bấm nhầm các thao tác phá hủy dữ liệu (xóa/hủy).

// Nút chính: màu xanh cỏ chính (#1B8A5A), chữ trắng nổi bật
bool NutChinh(const char* label, const ImVec2& kichThuoc = ImVec2(0.0f, 0.0f));

// Nút phụ: nền sáng, viền xám, chữ đậm
bool NutPhu(const char* label, const ImVec2& kichThuoc = ImVec2(0.0f, 0.0f));

// Nút nguy hiểm: nền đỏ (#DC3545), dùng cho hủy đặt sân, xóa, v.v.
bool NutNguyHiem(const char* label, const ImVec2& kichThuoc = ImVec2(0.0f, 0.0f));

}  // namespace Widgets
