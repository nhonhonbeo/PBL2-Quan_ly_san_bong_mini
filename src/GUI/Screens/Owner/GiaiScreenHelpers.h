#pragma once
// Hàm tiện ích nội bộ của GiaiScreen*.cpp (không phải API công khai).
#include <string>

#include "GUI/App/Facade.h"

namespace GUI::Screens::Owner::detail {

// Chuyển chuỗi ngày "DD/MM/YYYY" hoặc "YYYY-MM-DD" thành số nguyên YYYYMMDD để so sánh khoảng thời gian
inline int parseNgaySangSo(const std::string& str) {
    if (str.length() == 10) {
        if (str[2] == '/' && str[5] == '/') {
            try {
                int d = std::stoi(str.substr(0, 2));
                int m = std::stoi(str.substr(3, 2));
                int y = std::stoi(str.substr(6, 4));
                return y * 10000 + m * 100 + d;
            } catch (...) {}
        } else if (str[4] == '-' && str[7] == '-') {
            try {
                int y = std::stoi(str.substr(0, 4));
                int m = std::stoi(str.substr(5, 2));
                int d = std::stoi(str.substr(8, 2));
                return y * 10000 + m * 100 + d;
            } catch (...) {}
        }
    }
    return 0;
}

// Kiểm tra một ngày cụ thể có nằm trong khoảng [ngayBatDau, ngayKetThuc] hay không
inline bool namTrongKhoangNgay(const std::string& targetNgay, const std::string& startNgay, const std::string& endNgay) {
    int t = parseNgaySangSo(targetNgay);
    int s = parseNgaySangSo(startNgay);
    int e = parseNgaySangSo(endNgay);
    if (t == 0 || s == 0 || e == 0) return false;
    return (t >= s && t <= e);
}

inline const char* tenTrangThai(GUI::App::TrangThaiGiaiDau tt) {
    switch (tt) {
        case GUI::App::TrangThaiGiaiDau::ChoDuyet:   return u8"Chờ duyệt";
        case GUI::App::TrangThaiGiaiDau::DaDuyet:    return u8"Đã duyệt";
        case GUI::App::TrangThaiGiaiDau::DangDienRa: return u8"Đang diễn ra";
        case GUI::App::TrangThaiGiaiDau::DaKetThuc:  return u8"Đã kết thúc";
        case GUI::App::TrangThaiGiaiDau::TuChoi:     return u8"Bị từ chối";
    }
    return u8"Không xác định";
}

}  // namespace GUI::Screens::Owner::detail
