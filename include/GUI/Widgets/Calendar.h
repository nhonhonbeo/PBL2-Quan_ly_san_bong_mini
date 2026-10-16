#pragma once

#include <functional>
#include <string>
#include <vector>
#include <imgui.h>

namespace Widgets {

// LÝ DO CHỌN: Cấu trúc sự kiện trên lịch cho phép biểu diễn các giải đấu, trận đấu hoặc lượt đặt sân
// với nhãn tóm tắt và màu sắc trực quan (chấm màu), dễ dàng tái sử dụng cho cả Chủ sân và Người thuê.
struct SuKienLich {
    std::string ngay;           // Định dạng "dd/MM/yyyy" hoặc "YYYY-MM-DD"
    std::string nhan;           // Nhãn / Tiêu đề sự kiện
    ImU32 mau = 0;              // Màu chấm sự kiện (nếu 0, mặc định màu xanh cỏ Theme::Colors::GrassGreen)
    std::string id;             // Mã định danh sự kiện hoặc giải đấu
};

// Trạng thái lưu trữ của widget Calendar
struct CalendarState {
    int thang = 10;             // 1..12
    int nam = 2026;             // 1900..2100
    std::string ngayDangChon;   // Ngày được chọn ("dd/MM/yyyy")
    bool daKhoiTao = false;
};

// LÝ DO CHỌN: Calendar là widget tự vẽ lịch tháng trực quan (7 cột từ Thứ 2 đến Chủ Nhật theo chuẩn Việt Nam),
// có nút < > đổi tháng, nút quay về Hôm nay, tự động đánh dấu viền xanh cho ngày hiện tại và chấm màu cho các
// ngày có sự kiện/giải đấu. Hỗ trợ callback onNgayClick để cập nhật danh sách sự kiện bên cạnh.
bool Calendar(
    const char* id,
    CalendarState& state,
    const std::vector<SuKienLich>& danhSachSuKien = {},
    std::function<void(const std::string& ngay)> onNgayClick = nullptr,
    ImVec2 kichThuoc = ImVec2(0.0f, 0.0f)
);

}  // namespace Widgets
