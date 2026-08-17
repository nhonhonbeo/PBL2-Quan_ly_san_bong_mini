#ifndef DATSAN_H
#define DATSAN_H
#include <string>

enum class TrangThaiDat { CHO_XAC_NHAN, DA_XAC_NHAN, DA_HUY };

// Lop the hien 1 lan dat san - lien ket giua NguoiThue va San
class DatSan {
private:
    std::string maDon;
    std::string maSan;
    std::string maKhachHang;
    std::string ngay;
    std::string khungGio; // vd: "18:00-19:00"
    TrangThaiDat trangThai;

public:
    DatSan(const std::string& maDon, const std::string& maSan,
           const std::string& maKhachHang, const std::string& ngay,
           const std::string& khungGio);

    void huy() { trangThai = TrangThaiDat::DA_HUY; }
    void xacNhan() { trangThai = TrangThaiDat::DA_XAC_NHAN; }
    TrangThaiDat layTrangThai() const { return trangThai; }

    // TODO: em dung HashTable<(maSan+ngay), LinkedList<DatSan>> de kiem tra
    // nhanh khung gio con trong hay khong
};

#endif
