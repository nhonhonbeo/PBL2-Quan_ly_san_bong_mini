#pragma once

#include <functional>

namespace Widgets {

// LÝ DO CHỌN: Hộp thoại Modal chuẩn giúp chặn tương tác ngoài ý muốn và yêu cầu người dùng
// xác nhận tường minh trước khi thực hiện các hành động quan trọng (như hủy đặt sân, xóa dữ liệu).

// Kích hoạt mở popup modal xác nhận theo tiêu đề
void MoModalXacNhan(const char* tieuDe);

// Render modal xác nhận (gọi mỗi frame tại nơi quản lý màn hình hoặc loop)
bool ModalXacNhan(
    const char* tieuDe,
    const char* noiDung,
    const std::function<void()>& onDongY,
    const char* labelDongY = u8"Đồng ý",
    const char* labelHuy = u8"Hủy bỏ",
    bool laNguyHiem = false
);

}  // namespace Widgets
