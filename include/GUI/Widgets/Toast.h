#pragma once

#include <string>

namespace Widgets {

enum class LoaiToast {
    ThanhCong,  // Màu xanh cỏ (#1B8A5A)
    Loi,        // Màu đỏ (#DC3545)
    CanhBao,    // Màu vàng (#F5B301)
    ThongTin    // Màu xanh dương (#0284C7)
};

// LÝ DO CHỌN: Toast notification hàng đợi góc phải trên giúp phản hồi tức thì
// kết quả thao tác của người dùng mà không làm ngắt quãng trải nghiệm tương tác.

// Thêm thông báo vào hàng đợi
void ThemToast(const std::string& noiDung, LoaiToast loai = LoaiToast::ThongTin, float thoiGian = 3.0f);

// Các hàm tiện ích phát nhanh theo từng loại thông báo
void ToastThanhCong(const std::string& noiDung);
void ToastLoi(const std::string& noiDung);
void ToastCanhBao(const std::string& noiDung);
void ToastThongTin(const std::string& noiDung);

// Render toàn bộ toast ở góc phải trên màn hình (gọi ở cuối vòng lặp render mỗi frame)
void RenderToasts();

}  // namespace Widgets
