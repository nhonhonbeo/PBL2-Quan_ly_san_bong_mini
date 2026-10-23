#pragma once

#include <string>
#include "GUI/App/Facade.h"
#include "GUI/App/Screen.h"

namespace GUI::Screens::Renter {

// LÝ DO CHỌN: ProfileScreen quản lý thông tin định danh và bảo mật tài khoản cho Người thuê sân,
// cho phép cập nhật họ tên, SĐT và đổi mật khẩu với quy trình kiểm tra dữ liệu qua FormValidator.
class ProfileScreen : public GUI::App::Screen {
public:
    ProfileScreen() = default;
    ~ProfileScreen() override = default;

    void onEnter() override;
    void render() override;
    void onExit() override;

    void reloadViewModel();

private:
    void renderFormThongTin();
    void renderFormDoiMatKhau();

    // Dữ liệu cá nhân
    std::string formHoTen_;
    std::string formSDT_;
    std::string sdtBanDau_;
    int soLanDat_ = 0;
    int soLanHuy_ = 0;
    std::string hangThanhVien_;

    // Dữ liệu đổi mật khẩu
    std::string formMatKhauCu_;
    std::string formMatKhauMoi_;
    std::string formNhapLaiMatKhau_;

    std::string loiHoTen_;
    std::string loiSDT_;
    std::string loiMatKhauCu_;
    std::string loiMatKhauMoi_;
    std::string loiNhapLai_;

    bool hienMatKhauCu_ = false;
    bool hienMatKhauMoi_ = false;
    bool hienNhapLai_ = false;
};

}  // namespace GUI::Screens::Renter
