#pragma once

#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "Account/Customer.h"
#include "Account/Owner.h"
#include "Account/User.h"
#include "GUI/App/ScreenManager.h"
#include "GUI/Widgets/SlotGrid.h"

namespace GUI::App {

// LÝ DO CHỌN: Cấu trúc kết quả trả về chuẩn hóa cho mọi tác vụ nghiệp vụ
struct KetQua {
    bool ok = false;
    std::string loi;
};

template <typename T>
struct KetQuaDuLieu {
    bool ok = false;
    std::string loi;
    T duLieu{};
};

// Cấu trúc dữ liệu Sân bóng
struct SanBongInfo {
    std::string maSan;       // ví dụ: "S5-01", "S5-02", "S7-01", "S11-01"
    std::string tenSan;      // ví dụ: "Sân 5 số 01"
    int loaiSan = 5;         // 5, 7, 11 người
    int giaThueGio = 250000; // VNĐ / giờ
    bool dangBaoTri = false; // true: bảo trì, false: sẵn sàng
    std::string quyDinh;     // Quy định sử dụng sân
};

// Cấu trúc dữ liệu Khung giờ
struct KhungGioInfo {
    std::string id;
    std::string batDau;      // "06:00"
    std::string ketThuc;     // "07:30"
    bool laGioVang = false;  // 17:30 - 20:30
};

// Cấu trúc dữ liệu Đơn đặt sân
struct DonDatSanInfo {
    std::string maDon;
    std::string maSan;
    std::string tenKhachHang;
    std::string sdt;
    std::string ngay;        // "dd/MM/yyyy"
    std::string khungGio;    // "17:30 - 19:00"
    int giaTien = 0;
    std::string trangThai;   // "Đã xác nhận", "Đã thanh toán", "Đã hủy"
};

// LÝ DO CHỌN: Facade là lớp trung gian độc lập kết nối GUI với tầng nghiệp vụ.
// Cung cấp toàn bộ dữ liệu mẫu và các thao tác quản lý sân bóng cho Chủ sân.
class Facade {
public:
    static Facade& getInstance();

    // ==========================================
    // 1. KHỞI TẠO VÀ LƯU TRỮ DỮ LIỆU
    // ==========================================
    void napDuLieu();
    void luuDuLieu();

    // ==========================================
    // 2. XÁC THỰC & ĐĂNG KÝ
    // ==========================================
    KetQuaDuLieu<std::shared_ptr<User>> dangNhap(const std::string& sdt, const std::string& matKhau);

    KetQua dangKyNguoiThue(
        const std::string& hoTen,
        const std::string& sdt,
        const std::string& matKhau
    );

    KetQua dangKyChuSan(
        const std::string& hoTen,
        const std::string& sdt,
        const std::string& matKhau,
        const std::string& tenCoSo,
        const std::string& diaChi
    );

    bool kiemTraSDTDaTonTai(const std::string& sdt) const;

    void moManHinhChinhChoUser(User* user, ScreenManager& sm);

    // ==========================================
    // 3. THỐNG KÊ DASHBOARD CHỦ SÂN
    // ==========================================
    int getTongSoSan() const;
    int getLuotDatHomNay() const;
    long long getDoanhThuThang() const;
    int getSoKhachMoi() const;

    // Doanh thu 7 ngày gần nhất (nhãn thứ/ngày, số tiền)
    std::vector<std::pair<std::string, long long>> getDoanhThu7NgayGanNhat() const;

    // Danh sách các lượt đặt sắp tới trong ngày
    std::vector<DonDatSanInfo> getDanhSachLuotDatSapToi(int limit = 5) const;

    // ==========================================
    // 4. QUẢN LÝ SÂN BÓNG (CRUD & WIZARD)
    // ==========================================
    std::vector<SanBongInfo> getDanhSachSanBong() const;
    SanBongInfo* getSanBong(const std::string& maSan);

    // Tự sinh mã sân theo loại (ví dụ: S5-01, S5-02, S7-01...)
    KetQua taoNhieuSan(
        int loaiSan,
        int soLuong,
        const std::string& quyDinh,
        std::vector<std::string>& outMaSan
    );

    KetQua suaSanBong(const std::string& maSan, const std::string& tenSan, int loaiSan, int giaThue);
    KetQua toggleBaoTriSan(const std::string& maSan);
    KetQua xoaSanBong(const std::string& maSan);

    // ==========================================
    // 5. KHUNG GIỜ VÀ GIÁ THUÊ MA TRẬN
    // ==========================================
    std::vector<KhungGioInfo> getDanhSachKhungGio() const;

    KetQua sinhKhungGioTuDong(
        const std::string& gioMo,
        const std::string& gioDong,
        int buocPhut
    );

    KetQua themKhungGio(const std::string& batDau, const std::string& ketThuc, bool laGioVang);
    KetQua suaKhungGio(const std::string& id, const std::string& batDau, const std::string& ketThuc, bool laGioVang);
    KetQua xoaKhungGio(const std::string& id);

    // Ma trận giá thuê (Khung giờ x Loại sân)
    int getGiaMaTran(const std::string& khungGioStr, int loaiSan) const;
    KetQua capNhatGiaMaTran(
        const std::string& khungGioStr,
        int loaiSan,
        int giaMoi,
        bool apDungTatCaCungLoai
    );

    // Lưới ma trận tình trạng Slot theo ngày (SlotGrid)
    std::map<std::pair<std::string, std::string>, Widgets::SlotInfo> getTinhTrangSlotTheoNgay(const std::string& ngay);

    // ==========================================
    // 6. CÁC HÀM NGHIỆP VỤ TODO
    // ==========================================
    KetQua datSan(const std::string& maSan, const std::string& ngay, const std::string& gio);
    KetQua huyDatSan(const std::string& maDonDat);
    KetQua themSanBong(const std::string& tenSan, int loaiSan, int giaThue);
    KetQua capNhatKhungGioGia(const std::string& maSan, const std::string& gioMo, const std::string& gioDong, int giaMoi);
    KetQua themGiaiDau(const std::string& tenGiai, const std::string& ngayBatDau, const std::string& ngayKetThuc);
    KetQua guiDanhGiaSan(const std::string& maSan, int soSao, const std::string& noiDung);

private:
    Facade();
    ~Facade() = default;

    Facade(const Facade&) = delete;
    Facade& operator=(const Facade&) = delete;

    std::vector<std::shared_ptr<User>> danhSachUser_;
    std::unordered_map<std::string, std::string> mapMatKhau_;
    bool daNapDuLieu_ = false;

    // Bộ nhớ quản lý sân bóng, khung giờ, bảng giá và đơn đặt sân
    std::vector<SanBongInfo> danhSachSan_;
    std::vector<KhungGioInfo> danhSachKhungGio_;
    // map key: pair(khungGioStr, loaiSan) -> giaTien
    std::map<std::pair<std::string, int>, int> maTranGia_;
    std::vector<DonDatSanInfo> danhSachDonDat_;
};

}  // namespace GUI::App
