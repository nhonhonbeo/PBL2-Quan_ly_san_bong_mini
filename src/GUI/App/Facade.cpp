#include "GUI/App/Facade.h"

#include <algorithm>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "GUI/App/Session.h"
#include "GUI/Screens/Owner/OwnerShellScreen.h"
#include "GUI/Screens/Renter/RenterShellScreen.h"
#include "Storage/FileManager.h"

namespace GUI::App {

Facade::Facade() {
    napDuLieu();
}

Facade& Facade::getInstance() {
    static Facade instance;
    return instance;
}

void Facade::napDuLieu() {
    if (daNapDuLieu_) {
        return;
    }

    // 1. Nạp danh sách tài khoản người dùng từ FileManager
    Storage::FileManager::docDanhSachUser(danhSachUser_);
    for (const auto& user : danhSachUser_) {
        if (user) {
            mapMatKhau_[user->get_phone_number()] = "123456";
        }
    }

    // 2. Khởi tạo danh sách sân bóng ban đầu cho cơ sở
    danhSachSan_ = {
        {"S5-01",  u8"Sân 5 số 01 (Cỏ nhân tạo)", 5,  250000, false, u8"Đi giày đế TF/IC, không hút thuốc"},
        {"S5-02",  u8"Sân 5 số 02 (Cỏ nhân tạo)", 5,  250000, false, u8"Đi giày đế TF/IC, không hút thuốc"},
        {"S5-03",  u8"Sân 5 số 03 (Cỏ nhân tạo)", 5,  250000, true,  u8"Bảo dưỡng thay mặt cỏ định kỳ"},
        {"S7-01",  u8"Sân 7 số 01 (Khuôn viên ngoài)", 7, 400000, false, u8"Đi giày TF/FG mini, trả bóng đúng giờ"},
        {"S7-02",  u8"Sân 7 số 02 (Khuôn viên ngoài)", 7, 400000, false, u8"Đi giày TF/FG mini, trả bóng đúng giờ"},
        {"S11-01", u8"Sân 11 số 01 (Tiêu chuẩn thi đấu)", 11, 750000, false, u8"Phục vụ giải đấu phong trào và giao hữu"}
    };

    // 3. Khởi tạo danh sách khung giờ ban đầu
    danhSachKhungGio_ = {
        {"KG01", "06:00", "07:30", false},
        {"KG02", "07:30", "09:00", false},
        {"KG03", "16:00", "17:30", false},
        {"KG04", "17:30", "19:00", true},  // Giờ vàng
        {"KG05", "19:00", "20:30", true},  // Giờ vàng
        {"KG06", "20:30", "22:00", false}
    };

    // 4. Khởi tạo ma trận giá thuê (Khung giờ x Loại sân)
    for (const auto& kg : danhSachKhungGio_) {
        std::string kgStr = kg.batDau + " - " + kg.ketThuc;
        if (kg.laGioVang) {
            maTranGia_[{kgStr, 5}]  = 320000;
            maTranGia_[{kgStr, 7}]  = 480000;
            maTranGia_[{kgStr, 11}] = 850000;
        } else {
            maTranGia_[{kgStr, 5}]  = 220000;
            maTranGia_[{kgStr, 7}]  = 380000;
            maTranGia_[{kgStr, 11}] = 700000;
        }
    }

    // 5. Khởi tạo danh sách lượt đặt sắp tới trong ngày
    danhSachDonDat_ = {
        {"BK-1001", "S5-01",  u8"Lê Hoàng Anh",   "0981122334", "17/10/2026", "17:30 - 19:00", 320000, u8"Đã xác nhận"},
        {"BK-1002", "S5-02",  u8"Nguyễn Tuấn Tú", "0905544332", "17/10/2026", "17:30 - 19:00", 320000, u8"Đã xác nhận"},
        {"BK-1003", "S7-01",  u8"CLB Brothers",   "0913344556", "17/10/2026", "19:00 - 20:30", 480000, u8"Đã thanh toán"},
        {"BK-1004", "S11-01", u8"FC Bách Khoa",   "0977889900", "17/10/2026", "19:00 - 20:30", 850000, u8"Đã xác nhận"},
        {"BK-1005", "S5-01",  u8"Trần Văn Thuê",  "0909876543", "17/10/2026", "20:30 - 22:00", 220000, u8"Đã xác nhận"}
    };

    daNapDuLieu_ = true;
}

void Facade::luuDuLieu() {
    Storage::FileManager::ghiDanhSachUser(danhSachUser_);
}

bool Facade::kiemTraSDTDaTonTai(const std::string& sdt) const {
    for (const auto& user : danhSachUser_) {
        if (user && user->get_phone_number() == sdt) {
            return true;
        }
    }
    return false;
}

KetQuaDuLieu<std::shared_ptr<User>> Facade::dangNhap(const std::string& sdt, const std::string& matKhau) {
    napDuLieu();

    std::shared_ptr<User> timThay = nullptr;
    for (const auto& user : danhSachUser_) {
        if (user && user->get_phone_number() == sdt) {
            timThay = user;
            break;
        }
    }

    if (!timThay) {
        return {false, u8"Số điện thoại này chưa được đăng ký trong hệ thống.", nullptr};
    }

    auto itPw = mapMatKhau_.find(sdt);
    std::string matKhauDung = (itPw != mapMatKhau_.end()) ? itPw->second : "123456";

    if (matKhau != matKhauDung) {
        return {false, u8"Mật khẩu đăng nhập không chính xác.", nullptr};
    }

    Session::getInstance().login(timThay, timThay->get_account_type());
    return {true, "", timThay};
}

KetQua Facade::dangKyNguoiThue(
    const std::string& hoTen,
    const std::string& sdt,
    const std::string& matKhau
) {
    napDuLieu();

    if (kiemTraSDTDaTonTai(sdt)) {
        return {false, u8"Số điện thoại này đã được đăng ký tài khoản khác."};
    }

    char idBuf[16];
    std::snprintf(idBuf, sizeof(idBuf), "US%02d", static_cast<int>(danhSachUser_.size() + 1));

    auto cust = std::make_shared<Customer>(
        idBuf,
        sdt,
        matKhau,
        hoTen,
        sdt
    );

    danhSachUser_.push_back(cust);
    mapMatKhau_[sdt] = matKhau;

    luuDuLieu();
    return {true, ""};
}

KetQua Facade::dangKyChuSan(
    const std::string& hoTen,
    const std::string& sdt,
    const std::string& matKhau,
    const std::string& tenCoSo,
    const std::string& diaChi
) {
    napDuLieu();

    if (kiemTraSDTDaTonTai(sdt)) {
        return {false, u8"Số điện thoại này đã được đăng ký tài khoản khác."};
    }

    char idBuf[16];
    std::snprintf(idBuf, sizeof(idBuf), "US%02d", static_cast<int>(danhSachUser_.size() + 1));

    auto owner = std::make_shared<Owner>(
        idBuf,
        sdt,
        matKhau,
        hoTen,
        sdt,
        tenCoSo,
        diaChi,
        u8"Quy định tiêu chuẩn cụm sân",
        "06:00",
        "23:00",
        hoTen
    );

    danhSachUser_.push_back(owner);
    mapMatKhau_[sdt] = matKhau;

    luuDuLieu();
    return {true, ""};
}

void Facade::moManHinhChinhChoUser(User* user, ScreenManager& sm) {
    if (!user) {
        return;
    }

    // LÝ DO CHỌN: Đa hình thay cho if/else theo vai trò để mở Shell tương ứng.
    if (auto owner = dynamic_cast<Owner*>(user)) {
        (void)owner;
        sm.replace(std::make_unique<GUI::Screens::Owner::OwnerShellScreen>());
    } else if (auto customer = dynamic_cast<Customer*>(user)) {
        (void)customer;
        sm.replace(std::make_unique<GUI::Screens::Renter::RenterShellScreen>());
    }
}

// ==========================================
// 3. THỐNG KÊ DASHBOARD CHỦ SÂN
// ==========================================
int Facade::getTongSoSan() const {
    return static_cast<int>(danhSachSan_.size());
}

int Facade::getLuotDatHomNay() const {
    return static_cast<int>(danhSachDonDat_.size());
}

long long Facade::getDoanhThuThang() const {
    return 86500000LL; // 86.5 triệu VNĐ
}

int Facade::getSoKhachMoi() const {
    return 14;
}

std::vector<std::pair<std::string, long long>> Facade::getDoanhThu7NgayGanNhat() const {
    return {
        {u8"T2", 5200000LL},
        {u8"T3", 6400000LL},
        {u8"T4", 4800000LL},
        {u8"T5", 7200000LL},
        {u8"T6", 8900000LL},
        {u8"T7", 13500000LL},
        {u8"CN", 15200000LL}
    };
}

std::vector<DonDatSanInfo> Facade::getDanhSachLuotDatSapToi(int limit) const {
    std::vector<DonDatSanInfo> res;
    int count = 0;
    for (const auto& d : danhSachDonDat_) {
        res.push_back(d);
        count++;
        if (count >= limit) break;
    }
    return res;
}

// ==========================================
// 4. QUẢN LÝ SÂN BÓNG (CRUD & WIZARD)
// ==========================================
std::vector<SanBongInfo> Facade::getDanhSachSanBong() const {
    return danhSachSan_;
}

SanBongInfo* Facade::getSanBong(const std::string& maSan) {
    for (auto& s : danhSachSan_) {
        if (s.maSan == maSan) {
            return &s;
        }
    }
    return nullptr;
}

KetQua Facade::taoNhieuSan(
    int loaiSan,
    int soLuong,
    const std::string& quyDinh,
    std::vector<std::string>& outMaSan
) {
    outMaSan.clear();
    if (soLuong <= 0) {
        return {false, u8"Số lượng sân cần tạo phải lớn hơn 0."};
    }

    // Đếm số lượng sân cùng loại hiện có để sinh số thứ tự tiếp theo
    int currentCount = 0;
    for (const auto& s : danhSachSan_) {
        if (s.loaiSan == loaiSan) {
            currentCount++;
        }
    }

    int giaMacDinh = (loaiSan == 5) ? 250000 : ((loaiSan == 7) ? 400000 : 750000);

    for (int i = 1; i <= soLuong; ++i) {
        int stt = currentCount + i;
        char maBuf[16];
        std::snprintf(maBuf, sizeof(maBuf), "S%d-%02d", loaiSan, stt);

        std::string tenSan = u8"Sân " + std::to_string(loaiSan) + u8" số " + (stt < 10 ? "0" : "") + std::to_string(stt);

        SanBongInfo newField;
        newField.maSan = maBuf;
        newField.tenSan = tenSan;
        newField.loaiSan = loaiSan;
        newField.giaThueGio = giaMacDinh;
        newField.dangBaoTri = false;
        newField.quyDinh = quyDinh;

        danhSachSan_.push_back(newField);
        outMaSan.push_back(maBuf);
    }

    return {true, ""};
}

KetQua Facade::suaSanBong(const std::string& maSan, const std::string& tenSan, int loaiSan, int giaThue) {
    auto san = getSanBong(maSan);
    if (!san) {
        return {false, u8"Không tìm thấy sân bóng với mã: " + maSan};
    }

    san->tenSan = tenSan;
    san->loaiSan = loaiSan;
    san->giaThueGio = giaThue;
    return {true, ""};
}

KetQua Facade::toggleBaoTriSan(const std::string& maSan) {
    auto san = getSanBong(maSan);
    if (!san) {
        return {false, u8"Không tìm thấy sân bóng với mã: " + maSan};
    }

    san->dangBaoTri = !san->dangBaoTri;
    return {true, ""};
}

KetQua Facade::xoaSanBong(const std::string& maSan) {
    auto it = std::remove_if(danhSachSan_.begin(), danhSachSan_.end(), [&](const SanBongInfo& s) {
        return s.maSan == maSan;
    });

    if (it != danhSachSan_.end()) {
        danhSachSan_.erase(it, danhSachSan_.end());
        return {true, ""};
    }
    return {false, u8"Không tìm thấy sân bóng cần xóa."};
}

// ==========================================
// 5. KHUNG GIỜ VÀ GIÁ THUÊ MA TRẬN
// ==========================================
std::vector<KhungGioInfo> Facade::getDanhSachKhungGio() const {
    return danhSachKhungGio_;
}

namespace {
int gioSangPhut(const std::string& timeStr) {
    if (timeStr.length() == 5 && timeStr[2] == ':') {
        int h = std::stoi(timeStr.substr(0, 2));
        int m = std::stoi(timeStr.substr(3, 2));
        return h * 60 + m;
    }
    return 0;
}

std::string phutSangGio(int totalMinutes) {
    int h = (totalMinutes / 60) % 24;
    int m = totalMinutes % 60;
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << h << ":"
        << std::setfill('0') << std::setw(2) << m;
    return oss.str();
}
}  // namespace

KetQua Facade::sinhKhungGioTuDong(const std::string& gioMo, const std::string& gioDong, int buocPhut) {
    int startM = gioSangPhut(gioMo);
    int endM = gioSangPhut(gioDong);

    if (endM <= startM) {
        endM += 24 * 60; // Qua đêm
    }

    if (buocPhut <= 0 || (endM - startM) < buocPhut) {
        return {false, u8"Khoảng thời gian mở cửa không đủ cho bước thời lượng đã chọn."};
    }

    danhSachKhungGio_.clear();
    int cur = startM;
    int stt = 1;

    while (cur + buocPhut <= endM) {
        char idBuf[16];
        std::snprintf(idBuf, sizeof(idBuf), "KG%02d", stt);

        std::string startStr = phutSangGio(cur);
        std::string endStr = phutSangGio(cur + buocPhut);

        // Khung giờ vàng từ 17:30 đến 20:30
        bool laVang = (cur >= 17 * 60 + 30 && cur < 20 * 60 + 30);

        danhSachKhungGio_.push_back({idBuf, startStr, endStr, laVang});

        std::string kgStr = startStr + " - " + endStr;
        if (maTranGia_.find({kgStr, 5}) == maTranGia_.end()) {
            maTranGia_[{kgStr, 5}]  = laVang ? 320000 : 220000;
            maTranGia_[{kgStr, 7}]  = laVang ? 480000 : 380000;
            maTranGia_[{kgStr, 11}] = laVang ? 850000 : 700000;
        }

        cur += buocPhut;
        stt++;
    }

    return {true, ""};
}

KetQua Facade::themKhungGio(const std::string& batDau, const std::string& ketThuc, bool laGioVang) {
    char idBuf[16];
    std::snprintf(idBuf, sizeof(idBuf), "KG%02d", static_cast<int>(danhSachKhungGio_.size() + 1));
    danhSachKhungGio_.push_back({idBuf, batDau, ketThuc, laGioVang});

    std::string kgStr = batDau + " - " + ketThuc;
    maTranGia_[{kgStr, 5}]  = laGioVang ? 320000 : 220000;
    maTranGia_[{kgStr, 7}]  = laGioVang ? 480000 : 380000;
    maTranGia_[{kgStr, 11}] = laGioVang ? 850000 : 700000;

    return {true, ""};
}

KetQua Facade::suaKhungGio(const std::string& id, const std::string& batDau, const std::string& ketThuc, bool laGioVang) {
    for (auto& kg : danhSachKhungGio_) {
        if (kg.id == id) {
            kg.batDau = batDau;
            kg.ketThuc = ketThuc;
            kg.laGioVang = laGioVang;
            return {true, ""};
        }
    }
    return {false, u8"Không tìm thấy khung giờ cần sửa."};
}

KetQua Facade::xoaKhungGio(const std::string& id) {
    auto it = std::remove_if(danhSachKhungGio_.begin(), danhSachKhungGio_.end(), [&](const KhungGioInfo& kg) {
        return kg.id == id;
    });

    if (it != danhSachKhungGio_.end()) {
        danhSachKhungGio_.erase(it, danhSachKhungGio_.end());
        return {true, ""};
    }
    return {false, u8"Không tìm thấy khung giờ cần xóa."};
}

int Facade::getGiaMaTran(const std::string& khungGioStr, int loaiSan) const {
    auto it = maTranGia_.find({khungGioStr, loaiSan});
    if (it != maTranGia_.end()) {
        return it->second;
    }
    return (loaiSan == 5) ? 250000 : ((loaiSan == 7) ? 400000 : 750000);
}

KetQua Facade::capNhatGiaMaTran(
    const std::string& khungGioStr,
    int loaiSan,
    int giaMoi,
    bool apDungTatCaCungLoai
) {
    if (apDungTatCaCungLoai) {
        for (const auto& kg : danhSachKhungGio_) {
            std::string k = kg.batDau + " - " + kg.ketThuc;
            maTranGia_[{k, loaiSan}] = giaMoi;
        }
    } else {
        maTranGia_[{khungGioStr, loaiSan}] = giaMoi;
    }
    return {true, ""};
}

std::map<std::pair<std::string, std::string>, Widgets::SlotInfo> Facade::getTinhTrangSlotTheoNgay(const std::string& ngay) {
    std::map<std::pair<std::string, std::string>, Widgets::SlotInfo> maTran;

    // 1. Khởi tạo tất cả sân bóng theo trạng thái mặc định
    for (const auto& san : danhSachSan_) {
        for (const auto& kg : danhSachKhungGio_) {
            std::string kgStr = kg.batDau + " - " + kg.ketThuc;
            Widgets::SlotInfo info;
            if (san.dangBaoTri) {
                info.status = Widgets::SlotStatus::BaoTri;
                info.ghiChu = san.quyDinh;
            } else {
                info.status = Widgets::SlotStatus::Trong;
                info.giaTien = getGiaMaTran(kgStr, san.loaiSan);
            }
            maTran[{san.maSan, kgStr}] = info;
        }
    }

    // 2. Nạp các đơn đặt sân trong ngày đó (nếu có)
    for (const auto& don : danhSachDonDat_) {
        if (don.ngay == ngay) {
            Widgets::SlotInfo info;
            info.status = Widgets::SlotStatus::DaDat;
            info.tenKhachHang = don.tenKhachHang;
            info.sdt = don.sdt;
            info.giaTien = don.giaTien;
            info.ghiChu = don.trangThai;
            maTran[{don.maSan, don.khungGio}] = info;
        }
    }

    return maTran;
}

// ==========================================
// 6. CÁC HÀM NGHIỆP VỤ TODO
// ==========================================
KetQua Facade::datSan(const std::string& maSan, const std::string& ngay, const std::string& gio) {
    (void)maSan; (void)ngay; (void)gio;
    // TODO: Sinh viên triển khai kiểm tra khung giờ trùng trong BookingManager và lưu đơn đặt sân
    return {true, ""};
}

KetQua Facade::huyDatSan(const std::string& maDonDat) {
    (void)maDonDat;
    // TODO: Sinh viên triển khai nghiệp vụ hoàn cọc và tăng biến cancel_count của Customer
    return {true, ""};
}

KetQua Facade::themSanBong(const std::string& tenSan, int loaiSan, int giaThue) {
    (void)tenSan; (void)loaiSan; (void)giaThue;
    // TODO: Sinh viên triển khai lưu danh sách sân mới vào FieldManager
    return {true, ""};
}

KetQua Facade::capNhatKhungGioGia(const std::string& maSan, const std::string& gioMo, const std::string& gioDong, int giaMoi) {
    (void)maSan; (void)gioMo; (void)gioDong; (void)giaMoi;
    // TODO: Sinh viên triển khai cập nhật thời gian mở đóng cửa và biểu giá theo khung giờ
    return {true, ""};
}

KetQua Facade::themGiaiDau(const std::string& tenGiai, const std::string& ngayBatDau, const std::string& ngayKetThuc) {
    (void)tenGiai; (void)ngayBatDau; (void)ngayKetThuc;
    // TODO: Sinh viên triển khai lưu thông tin sự kiện giải đấu và danh sách bảng đấu
    return {true, ""};
}

KetQua Facade::guiDanhGiaSan(const std::string& maSan, int soSao, const std::string& noiDung) {
    (void)maSan; (void)soSao; (void)noiDung;
    // TODO: Sinh viên triển khai lưu đánh giá và tính lại số sao trung bình của sân bóng
    return {true, ""};
}

}  // namespace GUI::App
