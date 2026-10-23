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

// Cấu trúc dữ liệu Khách hàng
struct KhachHangInfo {
    std::string id;
    std::string hoTen;
    std::string sdt;
    int soLanDat = 0;
    int soLanHuy = 0;
    bool biChan = false; // Thuộc danh sách đen (Blacklist)
    std::string lyDoChan;
    std::string ngayChan;
    int mucUuDaiPhanTram = 0; // % giảm giá được hưởng
};

// Cấu trúc Ngưỡng ưu đãi tích lũy
struct NguongUuDai {
    int id = 0;
    int soLanToiThieu = 5;
    int phanTramGiam = 5;
    std::string tenHang = u8"Thân thiết";
};

// Cấu trúc Đánh giá nhận xét
struct DanhGiaInfo {
    std::string id;
    std::string tenKhach;
    std::string sdt;
    std::string maSan;
    int soSao = 5;
    std::string ngayGui;
    std::string noiDung;
};

// Trạng thái của giải đấu / sự kiện bóng đá
enum class TrangThaiGiaiDau {
    ChoDuyet,      // Đang chờ chủ sân phê duyệt yêu cầu
    DaDuyet,       // Đã phê duyệt, sẵn sàng tổ chức
    DangDienRa,    // Đang trong thời gian thi đấu
    DaKetThuc,     // Đã bế mạc kết thúc
    TuChoi         // Bị từ chối (trùng lịch sân hoặc quy định)
};

// Cấu trúc dữ liệu Giải đấu / Sự kiện sân bóng
struct GiaiDauInfo {
    std::string maGiai;          // Ví dụ: "GD-01", "GD-02"
    std::string tenGiai;         // "Cúp Tứ Hùng Doanh Nghiệp IT"
    std::string tenKhachDat;     // "Trần Văn Thuê"
    std::string sdtKhach;        // "0909876543"
    std::string ngayBatDau;      // "20/10/2026" hoặc "2026-10-20"
    std::string ngayKetThuc;     // "25/10/2026" hoặc "2026-10-25"
    std::string maSan;           // "S7-01"
    std::string khungGio;        // "17:30 - 21:00"
    std::string moTa;            // Điều lệ và quy chế thi đấu
    std::string ghiChuTuChoi;    // Lý do từ chối nếu có
    TrangThaiGiaiDau trangThai = TrangThaiGiaiDau::ChoDuyet;
};

// LÝ DO CHỌN: Facade là lớp trung gian độc lập duy nhất giữa GUI và nghiệp vụ.
// Cung cấp toàn bộ dữ liệu mẫu và các thao tác quản lý sân bóng, khách hàng cho Chủ sân.
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

    std::vector<std::pair<std::string, long long>> getDoanhThu7NgayGanNhat() const;
    std::vector<DonDatSanInfo> getDanhSachLuotDatSapToi(int limit = 5) const;

    // ==========================================
    // 4. QUẢN LÝ SÂN BÓNG (CRUD & WIZARD)
    // ==========================================
    std::vector<SanBongInfo> getDanhSachSanBong() const;
    SanBongInfo* getSanBong(const std::string& maSan);

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

    int getGiaMaTran(const std::string& khungGioStr, int loaiSan) const;
    KetQua capNhatGiaMaTran(
        const std::string& khungGioStr,
        int loaiSan,
        int giaMoi,
        bool apDungTatCaCungLoai
    );

    std::map<std::pair<std::string, std::string>, Widgets::SlotInfo> getTinhTrangSlotTheoNgay(const std::string& ngay);

    // ==========================================
    // 6. QUẢN LÝ KHÁCH HÀNG, ƯU ĐÃI & DANH SÁCH ĐEN
    // ==========================================
    // LÝ DO CHỌN HashTable: Tra cứu khách hàng theo SĐT trong O(1) thời gian trung bình
    std::vector<KhachHangInfo> getDanhSachKhachHang() const;
    KhachHangInfo* timKhachHangTheoSDT(const std::string& sdt);

    // LÝ DO CHỌN LinkedList: Quản lý danh sách đen với thao tác chèn/xóa phần tử linh hoạt O(1)
    std::vector<KhachHangInfo> getDanhSachDen() const;
    std::vector<KhachHangInfo> getDeXuatChan(int soLanHuyToiThieu = 3) const;

    KetQua themVaoDanhSachDen(const std::string& sdt, const std::string& lyDo);
    KetQua goKhoiDanhSachDen(const std::string& sdt);

    // Ưu đãi
    std::vector<NguongUuDai> getNguongUuDai() const;
    KetQua capNhatNguongUuDai(int id, int soLanToiThieu, int phanTramGiam);
    KetQua apDungUuDaiChoKhach();

    // LÝ DO CHỌN LinkedList: Danh sách đánh giá mới nhất thêm vào đầu danh sách với O(1)
    std::vector<DanhGiaInfo> getDanhSachDanhGia() const;
    float getDiemTrungBinhToanCoSo() const;
    std::map<std::string, float> getDiemTrungBinhTungSan() const;

    // Lịch sử đặt/hủy và đánh giá của một khách hàng cụ thể
    std::vector<DonDatSanInfo> getLichSuDatCuaKhach(const std::string& sdt) const;
    std::vector<DanhGiaInfo> getDanhGiaCuaKhach(const std::string& sdt) const;

    // ==========================================
    // 7. QUẢN LÝ GIẢI ĐẤU & SỰ KIỆN SÂN BÓNG
    // ==========================================
    // LÝ DO CHỌN LinkedList: Danh sách giải đấu hỗ trợ duyệt tuần tự và thêm/xóa linh hoạt
    std::vector<GiaiDauInfo> getDanhSachGiaiDau() const;
    std::vector<GiaiDauInfo> getYeuCauGiaiDauChoDuyet() const;
    int getSoLuongYeuCauChoDuyet() const;
    GiaiDauInfo* getGiaiDau(const std::string& maGiai);

    // Duyệt giải đấu (Nghiệp vụ tự động kiểm tra trùng lịch sân bóng)
    KetQua duyetGiaiDau(const std::string& maGiai);

    // Từ chối yêu cầu tổ chức giải đấu
    KetQua tuChoiGiaiDau(const std::string& maGiai, const std::string& lyDo);

    // Tạo giải đấu mới thủ công
    KetQua taoGiaiDauThuCong(const GiaiDauInfo& giaiMoi);

    // Cập nhật / sửa thông tin giải đấu
    KetQua suaGiaiDau(const GiaiDauInfo& giaiCapNhat);

    // Hủy giải đấu
    KetQua huyGiaiDau(const std::string& maGiai);

    // ==========================================
    // 8. NGHIỆP VỤ DÀNH CHO NGƯỜI THUÊ SÂN
    // ==========================================
    bool laKhachHangBiChan(const std::string& sdt) const;
    int getSoLanHuyCuaKhach(const std::string& sdt) const;
    int getSoLanDatCuaKhach(const std::string& sdt) const;
    int getMucUuDaiCuaKhach(const std::string& sdt) const;

    // Đặt sân mới và sinh mã đơn
    KetQua datSanMoi(
        const std::string& maSan,
        const std::string& sdtKhach,
        const std::string& tenKhach,
        const std::string& ngay,
        const std::string& khungGio,
        int giaGoc,
        int giaGiam,
        std::string& outMaDon
    );
    KetQua datNhieuCaMoi(const std::string& maSan, const std::string& sdtKhach,
                         const std::string& tenKhach, const std::string& ngay,
                         const LinkedList<std::string>& khungGio, int loaiSan,
                         std::string& outMaDon);

    // Hủy đơn đặt sân của khách
    KetQua huyDonDat(const std::string& maDon, const std::string& sdtKhach);
    KetQua xacNhanDaHoanThanhDon(const std::string& maDon);

    // Giải đấu do người thuê gửi yêu cầu
    std::vector<GiaiDauInfo> getGiaiDauCuaUser(const std::string& sdt) const;
    KetQua guiYeuCauTaoGiai(const GiaiDauInfo& giaiMoi);

    // Đánh giá của người thuê
    std::vector<SanBongInfo> getDanhSachSanDaThueXong(const std::string& sdt) const;
    KetQua guiHoacSuaDanhGia(
        const std::string& sdt,
        const std::string& tenKhach,
        const std::string& maSan,
        int soSao,
        const std::string& noiDung,
        const std::string& danhGiaId = ""
    );

    // Cập nhật thông tin cá nhân & Đổi mật khẩu
    KetQua capNhatThongTinUser(const std::string& sdt, const std::string& hoTenMoi);
    KetQua capNhatThongTinUser(const std::string& sdtCu, const std::string& hoTenMoi,
                               const std::string& sdtMoi);
    KetQua doiMatKhauUser(const std::string& sdt, const std::string& matKhauCu, const std::string& matKhauMoi);

    // ==========================================
    // 9. CÁC HÀM NGHIỆP VỤ TODO
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

    // Bộ nhớ quản lý sân bóng, khung giờ, bảng giá
    std::vector<SanBongInfo> danhSachSan_;
    std::vector<KhungGioInfo> danhSachKhungGio_;
    std::map<std::pair<std::string, int>, int> maTranGia_;
    std::vector<DonDatSanInfo> danhSachDonDat_;

    // Bộ nhớ quản lý khách hàng, ưu đãi, danh sách đen và đánh giá
    std::vector<KhachHangInfo> danhSachKhachHang_;
    // LÝ DO CHỌN: HashTable lưu chỉ mục khách hàng theo SĐT để tra cứu siêu tốc O(1)
    std::unordered_map<std::string, size_t> mapIndexKhachSDT_;
    std::vector<NguongUuDai> danhSachNguongUuDai_;
    std::vector<DanhGiaInfo> danhSachDanhGia_;

    // Bộ nhớ quản lý giải đấu & sự kiện
    std::vector<GiaiDauInfo> danhSachGiaiDau_;
};

}  // namespace GUI::App
