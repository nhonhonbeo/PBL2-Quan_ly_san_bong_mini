#ifndef SAN_H
#define SAN_H
#include <string>

enum class LoaiSan { SAN_5, SAN_7, SAN_11 };

class San {
private:
    std::string maSan;
    LoaiSan loai;
    double giaThueMoiGio;
    bool dangHoatDong;

public:
    San(const std::string& maSan, LoaiSan loai, double gia);

    std::string layMaSan() const { return maSan; }
    double layGia() const { return giaThueMoiGio; }
    void datGia(double gia) { giaThueMoiGio = gia; }

    // TODO: em bo sung quy dinh san, gio dong/mo cua
};

#endif
