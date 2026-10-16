#pragma once

#include <imgui.h>

namespace Widgets {

// LÝ DO CHỌN: Badge nhãn trạng thái bo tròn hình viên thuốc (pill) trực quan,
// giúp nhận diện tức thì tình trạng sân bóng (trống, đã đặt, bảo trì) hoặc nhóm khách hàng (VIP, đen).

// Vẽ badge tùy biến text và màu nền, màu chữ
void Badge(const char* text, const ImVec4& bgMau, const ImVec4& textMau = ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

// Các preset định nghĩa sẵn theo nghiệp vụ quản lý sân bóng mini
void BadgeTrong(const char* text = u8"Trống");
void BadgeDaDat(const char* text = u8"Đã đặt");
void BadgeBaoTri(const char* text = u8"Bảo trì");
void BadgeVIP(const char* text = u8"VIP");
void BadgeDanhSachDen(const char* text = u8"Danh sách đen");

}  // namespace Widgets
