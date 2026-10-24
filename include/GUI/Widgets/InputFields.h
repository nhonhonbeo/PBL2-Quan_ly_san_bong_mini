#pragma once

#include <string>

namespace Widgets {

using InputTextValidator = std::string (*)(const std::string&);
using InputTextSanitizer = void (*)(std::string&);

// LÝ DO CHỌN: Gom các ô nhập liệu thành bộ chuẩn thống nhất có nhãn trên đầu,
// viền đổi màu đỏ khi có lỗi và tự động hiển thị dòng thông báo lỗi đỏ bên dưới.

// Ô nhập văn bản thông thường
bool InputText(
    const char* label,
    const char* id,
    std::string& buffer,
    const char* loi = nullptr,
    const char* placeholder = nullptr,
    // LÝ DO CHỌN: Cho phép DatePicker và TimePicker đặt ô nhập vừa với nút bên cạnh.
    float width = -1.0f
);

// Ô nhập có xác thực sau tương tác, tính lại sau khi ImGui cập nhật buffer trong frame hiện tại.
bool InputTextValidated(
    const char* label,
    const char* id,
    std::string& buffer,
    const char* placeholder,
    bool& daCham,
    bool batBuocKiemTra,
    InputTextValidator validator,
    InputTextSanitizer sanitizer,
    float width = -1.0f
);

// Ô nhập mật khẩu với nút chuyển đổi hiện/ẩn mật khẩu
bool InputPassword(
    const char* label,
    const char* id,
    std::string& buffer,
    bool& hienMatKhau,
    const char* loi = nullptr,
    const char* placeholder = nullptr
);

// Ô nhập số nguyên (giá sân, số lượng người, v.v.)
bool InputInt(
    const char* label,
    const char* id,
    int& giaTri,
    const char* loi = nullptr,
    int step = 1,
    int stepFast = 10
);

// Ô nhập số điện thoại: chỉ cho phép nhập số và giới hạn tối đa 10 chữ số
bool InputPhone(
    const char* label,
    const char* id,
    std::string& buffer,
    const char* loi = nullptr,
    const char* placeholder = nullptr
);

}  // namespace Widgets
