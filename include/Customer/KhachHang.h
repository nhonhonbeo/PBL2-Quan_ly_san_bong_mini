#ifndef KHACHHANG_H
#define KHACHHANG_H
#include <string>

class KhachHang {
private:
    std::string sdt;
    std::string hoTen;
    int soLanDat;
    int soLanHuy;
    bool trongDanhSachDen;

public:
    KhachHang(const std::string& sdt, const std::string& hoTen);

    void ghiNhanDat();
    void ghiNhanHuy(); // neu huy nhieu qua -> tu dong dua vao danh sach den
    bool laKhachVIP() const; // vd: soLanDat >= nguong uu dai

    // TODO: em bo sung danh gia (review), gan vao HashTable<sdt, KhachHang>
};

#endif
