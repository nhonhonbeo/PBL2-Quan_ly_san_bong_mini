#pragma once

#include <functional>

namespace Widgets {

// LÝ DO CHỌN: Card là khối giao diện cơ bản nhất giúp nhóm các thông tin liên quan,
// có nền trắng bo góc và bóng đổ nhẹ tạo chiều sâu không gian (elevation) theo chuẩn UI hiện đại.
void Card(const char* id, const char* tieuDe, const std::function<void()>& noiDung);

// Card có thể tùy chỉnh thêm khoảng cách đệm (padding)
void Card(const char* id, const char* tieuDe, float paddingX, float paddingY, const std::function<void()>& noiDung);

}  // namespace Widgets
