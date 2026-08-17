#ifndef CHUSAN_H
#define CHUSAN_H
#include "NguoiDung.h"
#include <vector>
#include <string>

// Ke thua tu NguoiDung - dai dien cho tai khoan chu san
class ChuSan : public NguoiDung {
private:
    std::vector<std::string> danhSachMaSanSoHuu;
    std::string quyDinhSan;

public:
    ChuSan(const std::string& id, const std::string& tenDangNhap,
           const std::string& matKhau, const std::string& sdt,
           const std::string& hoTen);

    void hienThiMenu() const override;
    std::string layVaiTro() const override { return "ChuSan"; }

    // TODO: em bo sung cac ham quan ly san, quan ly khach hang, quan ly giai
};

#endif
