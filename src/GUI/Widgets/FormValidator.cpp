#include "GUI/Widgets/FormValidator.h"

#include <cctype>
#include <string>

namespace Widgets {

namespace {

// LÝ DO CHỌN: Hàm loại bỏ khoảng trắng ở 2 đầu chuỗi để kiểm tra tính rỗng thực tế của người dùng
std::string catKhoangTrang(const std::string& str) {
    size_t first = 0;
    while (first < str.size() && std::isspace(static_cast<unsigned char>(str[first]))) {
        ++first;
    }
    if (first == str.size()) {
        return "";
    }
    size_t last = str.size() - 1;
    while (last > first && std::isspace(static_cast<unsigned char>(str[last]))) {
        --last;
    }
    return str.substr(first, last - first + 1);
}

// LÝ DO CHỌN: Năm nhuận theo lịch Gregory chuẩn: chia hết cho 400 HOẶC (chia hết cho 4 và không chia hết cho 100)
bool laNamNhuan(int nam) {
    return (nam % 400 == 0) || (nam % 4 == 0 && nam % 100 != 0);
}

int soNgayTrongThang(int thang, int nam) {
    switch (thang) {
        case 1: case 3: case 5: case 7: case 8: case 10: case 12:
            return 31;
        case 4: case 6: case 9: case 11:
            return 30;
        case 2:
            return laNamNhuan(nam) ? 29 : 28;
        default:
            return 0;
    }
}

}  // namespace

std::string FormValidator::kiemTraKhongDeTrong(const std::string& giaTri, const std::string& tenTruong) {
    if (catKhoangTrang(giaTri).empty()) {
        return tenTruong + u8" không được để trống.";
    }
    return "";
}

std::string FormValidator::kiemTraSDT(const std::string& sdt) {
    std::string trimmed = catKhoangTrang(sdt);
    if (trimmed.empty()) {
        return u8"Số điện thoại không được để trống.";
    }
    if (trimmed.length() != 10) {
        return u8"Số điện thoại phải gồm đúng 10 chữ số.";
    }
    if (trimmed[0] != '0') {
        return u8"Số điện thoại phải bắt đầu bằng chữ số 0.";
    }
    for (char c : trimmed) {
        if (!std::isdigit(static_cast<unsigned char>(c))) {
            return u8"Số điện thoại chỉ được chứa ký tự số.";
        }
    }
    return "";
}

std::string FormValidator::kiemTraMatKhau(const std::string& matKhau, size_t doDaiToiThieu) {
    if (matKhau.empty()) {
        return u8"Mật khẩu không được để trống.";
    }
    if (matKhau.length() < doDaiToiThieu) {
        return u8"Mật khẩu phải có ít nhất " + std::to_string(doDaiToiThieu) + u8" ký tự.";
    }
    return "";
}

std::string FormValidator::kiemTraNgayHopLe(const std::string& ngayStr) {
    std::string s = catKhoangTrang(ngayStr);
    if (s.empty()) {
        return u8"Ngày không được để trống.";
    }

    // Định dạng mong đợi: dd/MM/yyyy (đúng 10 ký tự)
    if (s.length() != 10 || s[2] != '/' || s[5] != '/') {
        return u8"Định dạng ngày phải là dd/MM/yyyy (ví dụ: 25/12/2025).";
    }

    for (size_t i = 0; i < 10; ++i) {
        if (i == 2 || i == 5) continue;
        if (!std::isdigit(static_cast<unsigned char>(s[i]))) {
            return u8"Ngày tháng chỉ được chứa số và dấu gạch chéo.";
        }
    }

    int ngay  = std::stoi(s.substr(0, 2));
    int thang = std::stoi(s.substr(3, 2));
    int nam   = std::stoi(s.substr(6, 4));

    if (nam < 1900 || nam > 2100) {
        return u8"Năm phải nằm trong khoảng từ 1900 đến 2100.";
    }
    if (thang < 1 || thang > 12) {
        return u8"Tháng phải từ 01 đến 12.";
    }

    int maxNgay = soNgayTrongThang(thang, nam);
    if (ngay < 1 || ngay > maxNgay) {
        if (thang == 2 && !laNamNhuan(nam) && ngay == 29) {
            return u8"Năm " + std::to_string(nam) + u8" không phải năm nhuận, tháng 2 chỉ có 28 ngày.";
        }
        return u8"Tháng " + std::to_string(thang) + u8" năm " + std::to_string(nam) +
               u8" chỉ có tối đa " + std::to_string(maxNgay) + u8" ngày.";
    }

    return "";
}

}  // namespace Widgets
