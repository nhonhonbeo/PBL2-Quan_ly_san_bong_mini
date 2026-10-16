#pragma once

#include <string>
#include "GUI/App/Screen.h"

namespace GUI::Screens::Auth {

// LÝ DO CHỌN: AuthScreen quản lý quy trình chào mừng và xác thực người dùng (Đăng nhập / Đăng ký),
// chia bố cục 2 cột chuyên nghiệp với banner sân bóng vẽ vector bằng ImDrawList và biểu mẫu trực quan.
class AuthScreen : public GUI::App::Screen {
public:
    AuthScreen() = default;
    ~AuthScreen() override = default;

    void onEnter() override;
    void render() override;
    void onExit() override;

private:
    void renderBanner(float width, float height);
    void renderFormCard(float width, float height);

    void renderTabDangNhap();
    void renderTabDangKy();

    void xuLyDangNhap();
    void xuLyDangKy();

    // ==========================================
    // DỮ LIỆU VIEWMODEL
    // ==========================================
    int activeTab_ = 0; // 0: Đăng nhập, 1: Đăng ký

    // Đăng nhập
    std::string loginSDT_ = "0901234567"; // SĐT mẫu có sẵn
    std::string loginMatKhau_ = "123456";
    bool loginHienMatKhau_ = false;
    std::string loginLoiSDT_;
    std::string loginLoiMatKhau_;
    std::string loginLoiChung_;

    // Khóa nút đăng nhập khi sai 3 lần
    int soLanDangNhapSai_ = 0;
    float thoiGianKhoaConLai_ = 0.0f; // 10 giây đếm ngược

    // Đăng ký
    int regVaiTro_ = 1; // 0: Chủ sân, 1: Người thuê (mặc định)
    std::string regHoTen_;
    std::string regSDT_;
    std::string regMatKhau_;
    std::string regNhapLaiMatKhau_;
    bool regHienMatKhau_ = false;

    // Dành riêng cho Chủ sân
    std::string regTenCoSo_;
    std::string regDiaChi_;

    // Thông báo lỗi từng trường đăng ký
    std::string regLoiHoTen_;
    std::string regLoiSDT_;
    std::string regLoiMatKhau_;
    std::string regLoiNhapLai_;
    std::string regLoiTenCoSo_;
    std::string regLoiDiaChi_;
};

}  // namespace GUI::Screens::Auth
