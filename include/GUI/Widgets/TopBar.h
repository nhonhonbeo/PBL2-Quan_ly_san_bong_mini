#pragma once

#include <functional>
#include <string>
#include <vector>

namespace Widgets {

// LÝ DO CHỌN: TopBar thanh tiêu đề trên đỉnh hiển thị định hướng người dùng (breadcrumb),
// tiêu đề màn hình, thông tin tài khoản và nút đăng xuất tinh gọn ở góc phải.
void TopBar(
    const char* tieuDeManHinh,
    const std::vector<std::string>& breadcrumbs = {},
    const char* tenNguoiDung = u8"Nguyễn Văn A",
    const char* vaiTro = u8"Chủ sân",
    float chieuCao = 60.0f,
    const std::function<void()>& onDangXuat = nullptr
);

}  // namespace Widgets
