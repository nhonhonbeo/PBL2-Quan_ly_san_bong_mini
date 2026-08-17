#ifndef NGUOIDUNG_H
#define NGUOIDUNG_H
#include <string>

// Lop co so cho tai khoan - the hien tinh DONG GOI (encapsulation)
class NguoiDung {
protected:
    std::string id;
    std::string tenDangNhap;
    std::string matKhau;
    std::string soDienThoai;
    std::string hoTen;

public:
    NguoiDung(const std::string& id, const std::string& tenDangNhap,
              const std::string& matKhau, const std::string& sdt,
              const std::string& hoTen);
    virtual ~NguoiDung() {}

    bool xacThuc(const std::string& matKhauNhap) const;
    std::string layTenDangNhap() const { return tenDangNhap; }

    // Ham ao - moi lop con (ChuSan / NguoiThue) se override khac nhau
    // => the hien tinh DA HINH (polymorphism)
    virtual void hienThiMenu() const = 0;
    virtual std::string layVaiTro() const = 0;
};

#endif
