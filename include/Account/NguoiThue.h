#ifndef NGUOITHUE_H
#define NGUOITHUE_H
#include "NguoiDung.h"

// Ke thua tu NguoiDung - dai dien cho tai khoan nguoi thue san
class NguoiThue : public NguoiDung {
private:
    int soLanDatSan;

public:
    NguoiThue(const std::string& id, const std::string& tenDangNhap,
              const std::string& matKhau, const std::string& sdt,
              const std::string& hoTen);

    void hienThiMenu() const override;
    std::string layVaiTro() const override { return "NguoiThue"; }

    int laySoLanDatSan() const { return soLanDatSan; }
    void tangSoLanDatSan() { soLanDatSan++; }

    // TODO: em bo sung ham dat san, huy san, tao giai, danh gia
};

#endif
