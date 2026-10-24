#pragma once

#include <string>

namespace Widgets {

// LÝ DO CHỌN: FormValidator gom toàn bộ quy tắc xác thực dữ liệu giao diện tập trung,
// tái sử dụng trên mọi biểu mẫu (form), trả về thông điệp lỗi tiếng Việt dễ hiểu cho người dùng.
class FormValidator {
public:
    // Kiểm tra chuỗi không được rỗng hoặc toàn khoảng trắng
    static std::string kiemTraKhongDeTrong(const std::string& giaTri, const std::string& tenTruong = "Trường này");

    // LÝ DO CHỌN: Tên giải có quy tắc riêng nên dùng chung một validator và bộ chuẩn hóa tại tầng widget.
    static std::string kiemTraTenGiaiDau(const std::string& tenGiai);
    static void chuanHoaTenGiaiDau(std::string& tenGiai);

    // Kiểm tra số điện thoại: đúng 10 chữ số và bắt đầu bằng số 0
    static std::string kiemTraSDT(const std::string& sdt);

    // Kiểm tra độ dài mật khẩu tối thiểu 6 ký tự
    static std::string kiemTraMatKhau(const std::string& matKhau, size_t doDaiToiThieu = 6);

    // Kiểm tra ngày hợp lệ theo định dạng dd/MM/yyyy (xử lý chính xác ngày tháng và năm nhuận)
    static std::string kiemTraNgayHopLe(const std::string& ngayStr);
};

}  // namespace Widgets
