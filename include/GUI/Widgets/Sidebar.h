#pragma once

#include <functional>
#include <string>
#include <vector>

namespace Widgets {

struct SidebarItem {
    std::string id;
    std::string bieuTuong; // Biểu tượng unicode hoặc text ngắn (ví dụ "[⚽]", "[📅]")
    std::string tieuDe;
};

// LÝ DO CHỌN: Sidebar thanh menu bên trái cố định giúp người dùng định hướng nhanh,
// logo thương hiệu nổi bật trên đỉnh và nút đăng xuất luôn hiện diện ở đáy màn hình.
bool Sidebar(
    const std::vector<SidebarItem>& danhSachMuc,
    int& mucDangChon,
    float chieuRong = 240.0f,
    const std::function<void()>& onDangXuat = nullptr
);

}  // namespace Widgets
