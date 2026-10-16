#pragma once

#include <string>
#include <vector>

namespace Widgets {

// LÝ DO CHỌN: TopBar thanh tiêu đề trên đỉnh hiển thị định hướng người dùng (breadcrumb),
// tiêu đề màn hình hiện tại, thông tin tài khoản đang đăng nhập và đồng hồ hệ thống thời gian thực.
void TopBar(
    const char* tieuDeManHinh,
    const std::vector<std::string>& breadcrumbs = {},
    const char* tenNguoiDung = u8"Nguyễn Văn A",
    const char* vaiTro = u8"Chủ sân",
    float chieuCao = 60.0f
);

}  // namespace Widgets
