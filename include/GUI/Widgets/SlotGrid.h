#pragma once

#include <functional>
#include <map>
#include <string>
#include <vector>
#include <imgui.h>

namespace Widgets {

enum class SlotStatus {
    Trong,     // Xanh cỏ (#1B8A5A)
    DaDat,     // Đỏ (#DC3545)
    BaoTri,    // Xám (#9CA3AF)
    DangChon   // Vàng nhấn (#F5B301)
};

struct SlotInfo {
    SlotStatus status = SlotStatus::Trong;
    std::string tenKhachHang;
    std::string sdt;
    int giaTien = 0;
    std::string ghiChu;
};

// LÝ DO CHỌN: SlotGrid là widget lưới thời gian ma trận dùng chung giữa Chủ sân và Người thuê,
// trực quan hóa tình trạng còn chỗ/đã đặt của mọi sân bóng theo từng khung giờ trong ngày.
class SlotGrid {
public:
    // Render lưới tình trạng sân bóng
    // maTranSlot: map key là pair(maSan, khungGio) -> SlotInfo
    static bool Render(
        const char* id,
        std::string& ngayHienTai, // dd/MM/yyyy (có thể thay đổi qua nút < > hoặc DatePicker)
        const std::vector<std::string>& danhSachMaSan,
        const std::vector<std::string>& danhSachKhungGio,
        const std::map<std::pair<std::string, std::string>, SlotInfo>& maTranSlot,
        bool choPhepChon = false,
        const std::string& maSanDangChon = "",
        const std::string& khungGioDangChon = "",
        const std::function<void(const std::string& maSan, const std::string& khungGio)>& onSlotClicked = nullptr
    );
};

}  // namespace Widgets
