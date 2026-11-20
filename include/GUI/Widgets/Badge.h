#pragma once

#include <imgui.h>

namespace Widgets {

// LÝ DO CHỌN: Badge nhãn trạng thái bo tròn hình viên thuốc (pill) trực quan,
// giúp nhận diện tức thì tình trạng sân bóng, nhãn nổi bật và trạng thái cần chú ý.

// Vẽ badge tùy biến text và màu nền, màu chữ
void Badge(const char* text, const ImVec4& bgMau, const ImVec4& textMau = ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

// Các preset định nghĩa sẵn theo nghiệp vụ quản lý sân bóng mini
void BadgeTrong(const char* text = u8"Trống");
void BadgeDaDat(const char* text = u8"Đã đặt");
void BadgeBaoTri(const char* text = u8"Bảo trì");
void BadgeNoiBat(const char* text);
void BadgeCanhBao(const char* text);
// Stable display mapping for both legacy labels and persisted status codes.
void BookingStatusBadge(const char* status);

}  // namespace Widgets
