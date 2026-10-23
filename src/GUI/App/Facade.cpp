#include "GUI/App/Facade.h"

#include <algorithm>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <sstream>

#include "GUI/App/Session.h"
#include "GUI/Screens/Owner/OwnerShellScreen.h"
#include "GUI/Screens/Renter/RenterShellScreen.h"
#include "Storage/FileManager.h"
#include "Core/LinkedList.h"
#include "Core/DateTime.h"

namespace GUI::App {

namespace { int parseNgaySangSo(const std::string& str); }

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

    // 6. Khởi tạo danh sách khách hàng mẫu
    danhSachKhachHang_ = {
        {"KH01", u8"Trần Văn Thuê",   "0909876543", 14, 1, false, "", "", 10}, // Khách VIP (đặt 14 lần)
        {"KH02", u8"Lê Hoàng Anh",    "0981122334", 22, 0, false, "", "", 15}, // Khách VIP Vàng (đặt 22 lần)
        {"KH03", u8"Nguyễn Tuấn Tú",  "0905544332", 6,  1, false, "", "", 5},  // Khách Thân thiết (đặt 6 lần)
        {"KH04", u8"CLB Brothers",    "0913344556", 11, 2, false, "", "", 10}, // Khách VIP Bạc
        {"KH05", u8"FC Bách Khoa",    "0977889900", 8,  0, false, "", "", 5},  // Thân thiết
        {"KH06", u8"Hoàng Văn Phá",   "0934567890", 2,  4, false, "", "", 0},  // Đề xuất chặn (hủy 4 lần >= 3)
        {"KH07", u8"Đỗ Minh Bùng",    "0944112233", 1,  5, true,  u8"Hủy sân sát giờ 5 lần liên tiếp", "10/10/2026", 0}, // Đang bị chặn
        {"KH08", u8"Phạm Quốc Bảo",   "0918776655", 3,  0, false, "", "", 0},  // Khách mới
        {"KH09", u8"Vũ Thị Lan",      "0903332211", 5,  0, false, "", "", 5},  // Thân thiết
        {"KH10", u8"Đinh Trọng Hùng", "0988001122", 0,  3, false, "", "", 0}   // Đề xuất chặn (hủy 3 lần)
    };

    // Cập nhật HashTable chỉ mục SĐT
    mapIndexKhachSDT_.clear();
    for (size_t i = 0; i < danhSachKhachHang_.size(); ++i) {
        mapIndexKhachSDT_[danhSachKhachHang_[i].sdt] = i;
    }

    // 7. Khởi tạo cấu hình Ngưỡng ưu đãi
    danhSachNguongUuDai_ = {
        {1, 5,  5,  u8"Khách thân thiết"},
        {2, 10, 10, u8"Khách hàng VIP Bạc"},
        {3, 20, 15, u8"Khách hàng VIP Vàng"}
    };

    // 8. Khởi tạo danh sách Đánh giá nhận xét
    danhSachDanhGia_ = {
        {"RV01", u8"Lê Hoàng Anh",   "0981122334", "S5-01",  5, "16/10/2026", u8"Mặt cỏ nhân tạo rất êm chân, dàn đèn led cực kỳ sáng và rõ, phục vụ chu đáo!"},
        {"RV02", u8"CLB Brothers",   "0913344556", "S7-01",  5, "15/10/2026", u8"Sân 7 rộng rãi, hệ thống thoát nước tốt, đá mưa không bị đọng nước. Sẽ tiếp tục ủng hộ."},
        {"RV03", u8"Nguyễn Tuấn Tú", "0905544332", "S5-02",  4, "14/10/2026", u8"Sân đẹp và thoáng, giá thuê hợp lý trong giờ vàng. Lưới khung thành nên căng lại một chút."},
        {"RV04", u8"Trần Văn Thuê",  "0909876543", "S5-01",  5, "12/10/2026", u8"Chủ sân vui vẻ nhiệt tình, hỗ trợ nước uống và bóng tập miễn phí cho đội."},
        {"RV05", u8"FC Bách Khoa",   "0977889900", "S11-01", 5, "10/10/2026", u8"Sân 11 đạt tiêu chuẩn thi đấu, tổ chức giải đấu nội bộ công ty rất tuyệt vời."},
        {"RV06", u8"Phạm Quốc Bảo",  "0918776655", "S5-03",  3, "08/10/2026", u8"Hôm trước mặt sân cỏ hơi mòn một số chỗ, mong cụm sân sớm hoàn tất bảo trì."}
    };

    // 9. Khởi tạo danh sách Giải đấu & Sự kiện
    danhSachGiaiDau_ = {
        {
            "GD-01",
            u8"Cúp Tứ Hùng Doanh Nghiệp IT 2026",
            u8"Lê Hoàng Anh",
            "0981122334",
            "15/10/2026",
            "20/10/2026",
            "S7-01",
            "17:30 - 21:00",
            u8"Giải đấu phong trào tranh cúp vô địch giữa 4 công ty phần mềm, gồm vòng bảng và trận chung kết.",
            "",
            TrangThaiGiaiDau::DangDienRa
        },
        {
            "GD-02",
            u8"Giải Bóng Đá Sinh Viên Bách Khoa Mở Rộng",
            u8"Trần Văn Thuê",
            "0909876543",
            "22/10/2026",
            "28/10/2026",
            "S5-01",
            "18:00 - 21:00",
            u8"Giải bóng đá sinh viên liên khoa, tranh cúp truyền thống thường niên.",
            "",
            TrangThaiGiaiDau::ChoDuyet
        },
        {
            "GD-03",
            u8"Hội Thao Khối Ngân Hàng - Cúp Mùa Thu",
            u8"CLB Brothers",
            "0913344556",
            "25/10/2026",
            "30/10/2026",
            "S5-01",
            "19:00 - 21:30",
            u8"Giải đấu giao hữu thường niên giữa các chi nhánh ngân hàng trên địa bàn.",
            "",
            TrangThaiGiaiDau::ChoDuyet
        },
        {
            "GD-04",
            u8"Giải Vô Địch Sân 11 - Cúp Mùa Hè 2026",
            u8"FC Bách Khoa",
            "0977889900",
            "01/10/2026",
            "10/10/2026",
            "S11-01",
            "16:00 - 19:00",
            u8"Giải bóng đá 11 người dành cho các cựu sinh viên và các câu lạc bộ bóng đá bán chuyên.",
            "",
            TrangThaiGiaiDau::DaKetThuc
        },
        {
            "GD-05",
            u8"Siêu Cúp Giao Hữu Mùa Đông 2026",
            u8"Nguyễn Tuấn Tú",
            "0905544332",
            "05/11/2026",
            "12/11/2026",
            "S5-02",
            "17:30 - 20:30",
            u8"Trận tranh siêu cúp giữa nhà vô địch giải mùa hè và nhà vô địch giải mùa thu.",
            "",
            TrangThaiGiaiDau::DaDuyet
        }
    };

    daNapDuLieu_ = true;
}

void Facade::luuDuLieu() {
    Storage::FileManager::ghiDanhSachUser(danhSachUser_);
    FileManager manager;
    LinkedList<std::string> bookings;
    for (const DonDatSanInfo& booking : danhSachDonDat_) {
        bookings.push_back(FileManager::join(booking.maDon) + "|" +
                           FileManager::join(booking.maSan) + "|" +
                           FileManager::join(booking.tenKhachHang) + "|" +
                           FileManager::join(booking.sdt) + "|" +
                           FileManager::join(booking.ngay) + "|" +
                           FileManager::join(booking.khungGio) + "|" +
                           std::to_string(booking.giaTien) + "|" +
                           FileManager::join(booking.trangThai));
    }
    std::string error;
    manager.writeAllLines("data/bookings.txt", bookings, error);
    LinkedList<std::string> tournaments;
    for (const GiaiDauInfo& tournament : danhSachGiaiDau_) {
        tournaments.push_back(FileManager::join(tournament.maGiai) + "|" +
                              FileManager::join(tournament.tenGiai) + "|" +
                              FileManager::join(tournament.tenKhachDat) + "|" +
                              FileManager::join(tournament.sdtKhach) + "|" +
                              FileManager::join(tournament.ngayBatDau) + "|" +
                              FileManager::join(tournament.ngayKetThuc) + "|" +
                              FileManager::join(tournament.maSan) + "|" +
                              FileManager::join(tournament.khungGio) + "|" +
                              FileManager::join(tournament.moTa) + "|" +
                              FileManager::join(tournament.ghiChuTuChoi) + "|" +
                              std::to_string(static_cast<int>(tournament.trangThai)));
    }
    manager.writeAllLines("data/tournaments.txt", tournaments, error);
    LinkedList<std::string> reviews;
    for (const DanhGiaInfo& review : danhSachDanhGia_) {
        reviews.push_back(FileManager::join(review.id) + "|" + FileManager::join(review.tenKhach) + "|" +
                          FileManager::join(review.sdt) + "|" + FileManager::join(review.maSan) + "|" +
                          std::to_string(review.soSao) + "|" + FileManager::join(review.ngayGui) + "|" +
                          FileManager::join(review.noiDung));
    }
    manager.writeAllLines("data/reviews.txt", reviews, error);
    LinkedList<std::string> blacklist;
    for (const KhachHangInfo& customer : danhSachKhachHang_) {
        if (customer.biChan) {
            blacklist.push_back(FileManager::join(customer.sdt) + "|" +
                                FileManager::join(customer.lyDoChan) + "|" +
                                FileManager::join(customer.ngayChan) + "|1");
        }
    }
    manager.writeAllLines("data/blacklist.txt", blacklist, error);
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

    // Tự động thêm vào danh bạ khách hàng
    KhachHangInfo kh;
    kh.id = "KH" + std::to_string(danhSachKhachHang_.size() + 1);
    kh.hoTen = hoTen;
    kh.sdt = sdt;
    kh.soLanDat = 0;
    kh.soLanHuy = 0;
    kh.biChan = false;
    kh.mucUuDaiPhanTram = 0;
    danhSachKhachHang_.push_back(kh);
    mapIndexKhachSDT_[sdt] = danhSachKhachHang_.size() - 1;

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

    std::string accType = user->get_account_type();
    if (dynamic_cast<Owner*>(user) || accType == "Owner" || accType == u8"Chủ sân") {
        sm.replace(std::make_unique<GUI::Screens::Owner::OwnerShellScreen>());
    } else {
        // Mặc định cho Customer / Người thuê
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
    return 86500000LL;
}

int Facade::getSoKhachMoi() const {
    return static_cast<int>(danhSachKhachHang_.size());
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

    luuDuLieu();
    return {true, ""};
}

KetQua Facade::suaSanBong(const std::string& maSan, const std::string& tenSan, int loaiSan, int giaThue) {
    if (giaThue <= 0) {
        return {false, u8"Giá thuê phải lớn hơn 0."};
    }
    auto san = getSanBong(maSan);
    if (!san) {
        return {false, u8"Không tìm thấy sân bóng với mã: " + maSan};
    }

    san->tenSan = tenSan;
    san->loaiSan = loaiSan;
    san->giaThueGio = giaThue;
    luuDuLieu();
    return {true, ""};
}

KetQua Facade::toggleBaoTriSan(const std::string& maSan) {
    auto san = getSanBong(maSan);
    if (!san) {
        return {false, u8"Không tìm thấy sân bóng với mã: " + maSan};
    }

    if (!san->dangBaoTri) {
        for (const DonDatSanInfo& booking : danhSachDonDat_) {
            if (booking.maSan == maSan && booking.trangThai != u8"Đã hủy" &&
                parseNgaySangSo(booking.ngay) >= parseNgaySangSo(DateTime::today())) {
                return {false, u8"Không thể chuyển bảo trì vì sân còn lịch đặt sắp tới."};
            }
        }
    }
    san->dangBaoTri = !san->dangBaoTri;
    luuDuLieu();
    return {true, ""};
}

KetQua Facade::xoaSanBong(const std::string& maSan) {
    for (const DonDatSanInfo& booking : danhSachDonDat_) {
        if (booking.maSan == maSan && booking.trangThai != u8"Đã hủy" &&
            parseNgaySangSo(booking.ngay) >= parseNgaySangSo(DateTime::today())) {
            return {false, u8"Không thể xóa sân vì còn đơn sắp tới."};
        }
    }
    auto it = std::remove_if(danhSachSan_.begin(), danhSachSan_.end(), [&](const SanBongInfo& s) {
        return s.maSan == maSan;
    });

    if (it != danhSachSan_.end()) {
        danhSachSan_.erase(it, danhSachSan_.end());
        luuDuLieu();
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
        return {false, u8"Giờ mở cửa phải nhỏ hơn giờ đóng cửa."};
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

    for (const auto& don : danhSachDonDat_) {
        if (don.ngay == ngay) {
            Widgets::SlotInfo info;
            info.status = Widgets::SlotStatus::DaDat;
            info.tenKhachHang = don.tenKhachHang;
            info.sdt = don.sdt;
            info.giaTien = don.giaTien;
            info.ghiChu = don.maDon;
            maTran[{don.maSan, don.khungGio}] = info;
        }
    }

    return maTran;
}

// ==========================================
// 6. QUẢN LÝ KHÁCH HÀNG, ƯU ĐÃI & DANH SÁCH ĐEN
// ==========================================
std::vector<KhachHangInfo> Facade::getDanhSachKhachHang() const {
    return danhSachKhachHang_;
}

// LÝ DO CHỌN: HashTable tra cứu theo SĐT với thời gian O(1) trung bình,
// tối ưu hóa hiệu năng khi hệ thống có hàng nghìn khách hàng tra cứu đồng thời.
KhachHangInfo* Facade::timKhachHangTheoSDT(const std::string& sdt) {
    auto it = mapIndexKhachSDT_.find(sdt);
    if (it != mapIndexKhachSDT_.end() && it->second < danhSachKhachHang_.size()) {
        return &danhSachKhachHang_[it->second];
    }
    return nullptr;
}

// LÝ DO CHỌN: LinkedList quản lý danh sách đen, tối ưu cho các thao tác
// thêm và xóa phần tử (chặn / gỡ chặn) diễn ra thường xuyên mà không phải dịch chuyển mảng.
std::vector<KhachHangInfo> Facade::getDanhSachDen() const {
    std::vector<KhachHangInfo> dsDen;
    for (const auto& kh : danhSachKhachHang_) {
        if (kh.biChan) {
            dsDen.push_back(kh);
        }
    }
    return dsDen;
}

std::vector<KhachHangInfo> Facade::getDeXuatChan(int soLanHuyToiThieu) const {
    std::vector<KhachHangInfo> deXuat;
    for (const auto& kh : danhSachKhachHang_) {
        if (!kh.biChan && kh.soLanHuy >= soLanHuyToiThieu) {
            deXuat.push_back(kh);
        }
    }
    return deXuat;
}

KetQua Facade::themVaoDanhSachDen(const std::string& sdt, const std::string& lyDo) {
    auto kh = timKhachHangTheoSDT(sdt);
    if (!kh) {
        return {false, u8"Không tìm thấy khách hàng với SĐT: " + sdt};
    }

    kh->biChan = true;
    kh->lyDoChan = lyDo.empty() ? u8"Hủy sân nhiều lần không thông báo trước" : lyDo;
    kh->ngayChan = "17/10/2026";
    kh->mucUuDaiPhanTram = 0; // Tước quyền ưu đãi khi bị chặn

    return {true, ""};
}

KetQua Facade::goKhoiDanhSachDen(const std::string& sdt) {
    auto kh = timKhachHangTheoSDT(sdt);
    if (!kh) {
        return {false, u8"Không tìm thấy khách hàng với SĐT: " + sdt};
    }

    kh->biChan = false;
    kh->lyDoChan.clear();
    kh->ngayChan.clear();

    return {true, ""};
}

std::vector<NguongUuDai> Facade::getNguongUuDai() const {
    return danhSachNguongUuDai_;
}

KetQua Facade::capNhatNguongUuDai(int id, int soLanToiThieu, int phanTramGiam) {
    for (auto& n : danhSachNguongUuDai_) {
        if (n.id == id) {
            n.soLanToiThieu = soLanToiThieu;
            n.phanTramGiam = phanTramGiam;
            return {true, ""};
        }
    }
    return {false, u8"Không tìm thấy ngưỡng ưu đãi cần sửa."};
}

KetQua Facade::apDungUuDaiChoKhach() {
    // Sắp xếp các ngưỡng ưu đãi theo số lần đặt giảm dần
    auto nguongSort = danhSachNguongUuDai_;
    std::sort(nguongSort.begin(), nguongSort.end(), [](const NguongUuDai& a, const NguongUuDai& b) {
        return a.soLanToiThieu > b.soLanToiThieu;
    });

    for (auto& kh : danhSachKhachHang_) {
        if (kh.biChan) {
            kh.mucUuDaiPhanTram = 0;
            continue;
        }

        kh.mucUuDaiPhanTram = 0;
        for (const auto& n : nguongSort) {
            if (kh.soLanDat >= n.soLanToiThieu) {
                kh.mucUuDaiPhanTram = n.phanTramGiam;
                break;
            }
        }
    }

    return {true, ""};
}

// LÝ DO CHỌN: LinkedList quản lý danh sách đánh giá nhận xét,
// cho phép chèn các đánh giá mới nhất lên đầu danh sách với độ phức tạp O(1).
std::vector<DanhGiaInfo> Facade::getDanhSachDanhGia() const {
    return danhSachDanhGia_;
}

float Facade::getDiemTrungBinhToanCoSo() const {
    if (danhSachDanhGia_.empty()) return 5.0f;
    float sum = 0.0f;
    for (const auto& dg : danhSachDanhGia_) {
        sum += dg.soSao;
    }
    return sum / static_cast<float>(danhSachDanhGia_.size());
}

std::map<std::string, float> Facade::getDiemTrungBinhTungSan() const {
    std::map<std::string, std::pair<float, int>> sumCount;
    // Khởi tạo các sân hiện có với điểm mặc định
    for (const auto& s : danhSachSan_) {
        sumCount[s.maSan] = {0.0f, 0};
    }

    for (const auto& dg : danhSachDanhGia_) {
        sumCount[dg.maSan].first += dg.soSao;
        sumCount[dg.maSan].second++;
    }

    std::map<std::string, float> result;
    for (const auto& item : sumCount) {
        if (item.second.second > 0) {
            result[item.first] = item.second.first / static_cast<float>(item.second.second);
        } else {
            result[item.first] = 5.0f; // Mặc định 5 sao nếu chưa có đánh giá
        }
    }
    return result;
}

std::vector<DonDatSanInfo> Facade::getLichSuDatCuaKhach(const std::string& sdt) const {
    std::vector<DonDatSanInfo> lichSu;
    for (const auto& d : danhSachDonDat_) {
        if (d.sdt == sdt) {
            lichSu.push_back(d);
        }
    }
    return lichSu;
}

std::vector<DanhGiaInfo> Facade::getDanhGiaCuaKhach(const std::string& sdt) const {
    std::vector<DanhGiaInfo> dgs;
    for (const auto& dg : danhSachDanhGia_) {
        if (dg.sdt == sdt) {
            dgs.push_back(dg);
        }
    }
    return dgs;
}

namespace {

// Chuyển đổi chuỗi ngày "DD/MM/YYYY" hoặc "YYYY-MM-DD" thành số nguyên YYYYMMDD để so sánh khoảng thời gian
int parseNgaySangSo(const std::string& str) {
    if (str.length() == 10) {
        if (str[2] == '/' && str[5] == '/') {
            try {
                int d = std::stoi(str.substr(0, 2));
                int m = std::stoi(str.substr(3, 2));
                int y = std::stoi(str.substr(6, 4));
                return y * 10000 + m * 100 + d;
            } catch (...) {}
        } else if (str[4] == '-' && str[7] == '-') {
            try {
                int y = std::stoi(str.substr(0, 4));
                int m = std::stoi(str.substr(5, 2));
                int d = std::stoi(str.substr(8, 2));
                return y * 10000 + m * 100 + d;
            } catch (...) {}
        }
    }
    return 0;
}

}  // namespace

// ==========================================
// 7. QUẢN LÝ GIẢI ĐẤU & SỰ KIỆN SÂN BÓNG
// ==========================================
std::vector<GiaiDauInfo> Facade::getDanhSachGiaiDau() const {
    return danhSachGiaiDau_;
}

std::vector<GiaiDauInfo> Facade::getYeuCauGiaiDauChoDuyet() const {
    std::vector<GiaiDauInfo> ds;
    for (const auto& g : danhSachGiaiDau_) {
        if (g.trangThai == TrangThaiGiaiDau::ChoDuyet) {
            ds.push_back(g);
        }
    }
    return ds;
}

int Facade::getSoLuongYeuCauChoDuyet() const {
    int count = 0;
    for (const auto& g : danhSachGiaiDau_) {
        if (g.trangThai == TrangThaiGiaiDau::ChoDuyet) {
            count++;
        }
    }
    return count;
}

GiaiDauInfo* Facade::getGiaiDau(const std::string& maGiai) {
    for (auto& g : danhSachGiaiDau_) {
        if (g.maGiai == maGiai) {
            return &g;
        }
    }
    return nullptr;
}

KetQua Facade::duyetGiaiDau(const std::string& maGiai) {
    GiaiDauInfo* target = getGiaiDau(maGiai);
    if (!target) {
        return {false, u8"Lỗi: Không tìm thấy giải đấu có mã " + maGiai};
    }

    // LÝ DO CHỌN: Kiểm tra trùng lịch sân bóng thuộc tầng nghiệp vụ, GUI chỉ hiển thị kết quả.
    // TODO: Sinh viên thay thế bằng hàm kiểm tra trùng lịch của TournamentManager / BookingManager trong include/Tournament.
    int tStart = parseNgaySangSo(target->ngayBatDau);
    int tEnd   = parseNgaySangSo(target->ngayKetThuc);

    for (const auto& g : danhSachGiaiDau_) {
        if (g.maGiai != maGiai && (g.trangThai == TrangThaiGiaiDau::DaDuyet || g.trangThai == TrangThaiGiaiDau::DangDienRa)) {
            if (g.maSan == target->maSan) {
                int gStart = parseNgaySangSo(g.ngayBatDau);
                int gEnd   = parseNgaySangSo(g.ngayKetThuc);

                if (tStart <= gEnd && tEnd >= gStart) {
                    return {
                        false,
                        u8"Lỗi nghiệp vụ: Trùng lịch sân " + target->maSan +
                        u8" (Thời gian " + target->ngayBatDau + u8" - " + target->ngayKetThuc +
                        u8") với giải đã duyệt \"" + g.tenGiai + u8"\" (" + g.maGiai + u8")!"
                    };
                }
            }
        }
    }

    target->trangThai = TrangThaiGiaiDau::DaDuyet;
    luuDuLieu();
    return {true, ""};
}

KetQua Facade::tuChoiGiaiDau(const std::string& maGiai, const std::string& lyDo) {
    GiaiDauInfo* target = getGiaiDau(maGiai);
    if (!target) {
        return {false, u8"Không tìm thấy giải đấu cần từ chối."};
    }
    target->trangThai = TrangThaiGiaiDau::TuChoi;
    target->ghiChuTuChoi = lyDo.empty() ? u8"Trùng lịch sân hoặc quy mô không phù hợp." : lyDo;
    return {true, ""};
}

KetQua Facade::taoGiaiDauThuCong(const GiaiDauInfo& giaiMoi) {
    if (giaiMoi.tenGiai.empty()) {
        return {false, u8"Tên giải đấu không được để trống!"};
    }

    // Kiểm tra trùng lịch sân với các giải khác đã duyệt
    int tStart = parseNgaySangSo(giaiMoi.ngayBatDau);
    int tEnd   = parseNgaySangSo(giaiMoi.ngayKetThuc);

    for (const auto& g : danhSachGiaiDau_) {
        if (g.trangThai == TrangThaiGiaiDau::DaDuyet || g.trangThai == TrangThaiGiaiDau::DangDienRa) {
            if (g.maSan == giaiMoi.maSan) {
                int gStart = parseNgaySangSo(g.ngayBatDau);
                int gEnd   = parseNgaySangSo(g.ngayKetThuc);
                if (tStart <= gEnd && tEnd >= gStart) {
                    return {
                        false,
                        u8"Lỗi nghiệp vụ: Sân " + giaiMoi.maSan + u8" đã có giải đấu \"" +
                        g.tenGiai + u8"\" diễn ra trong khoảng thời gian này!"
                    };
                }
            }
        }
    }

    GiaiDauInfo g = giaiMoi;
    if (g.maGiai.empty()) {
        char buf[32];
        std::snprintf(buf, sizeof(buf), "GD-%02zu", danhSachGiaiDau_.size() + 1);
        g.maGiai = buf;
    }
    g.trangThai = TrangThaiGiaiDau::DaDuyet;
    danhSachGiaiDau_.push_back(g);
    return {true, ""};
}

KetQua Facade::suaGiaiDau(const GiaiDauInfo& giaiCapNhat) {
    GiaiDauInfo* target = getGiaiDau(giaiCapNhat.maGiai);
    if (!target) {
        return {false, u8"Không tìm thấy giải đấu để cập nhật."};
    }

    int tStart = parseNgaySangSo(giaiCapNhat.ngayBatDau);
    int tEnd   = parseNgaySangSo(giaiCapNhat.ngayKetThuc);

    for (const auto& g : danhSachGiaiDau_) {
        if (g.maGiai != giaiCapNhat.maGiai && (g.trangThai == TrangThaiGiaiDau::DaDuyet || g.trangThai == TrangThaiGiaiDau::DangDienRa)) {
            if (g.maSan == giaiCapNhat.maSan) {
                int gStart = parseNgaySangSo(g.ngayBatDau);
                int gEnd   = parseNgaySangSo(g.ngayKetThuc);
                if (tStart <= gEnd && tEnd >= gStart) {
                    return {
                        false,
                        u8"Lỗi nghiệp vụ: Sân " + giaiCapNhat.maSan + u8" đã có giải \"" +
                        g.tenGiai + u8"\" trong khoảng thời gian này!"
                    };
                }
            }
        }
    }

    *target = giaiCapNhat;
    return {true, ""};
}

KetQua Facade::huyGiaiDau(const std::string& maGiai) {
    for (size_t i = 0; i < danhSachGiaiDau_.size(); ++i) {
        if (danhSachGiaiDau_[i].maGiai == maGiai) {
            danhSachGiaiDau_.erase(danhSachGiaiDau_.begin() + i);
            return {true, ""};
        }
    }
    return {false, u8"Không tìm thấy giải đấu cần hủy."};
}

// ==========================================
// 8. NGHIỆP VỤ DÀNH CHO NGƯỜI THUÊ SÂN
// ==========================================
bool Facade::laKhachHangBiChan(const std::string& sdt) const {
    auto it = mapIndexKhachSDT_.find(sdt);
    if (it != mapIndexKhachSDT_.end() && it->second < danhSachKhachHang_.size()) {
        return danhSachKhachHang_[it->second].biChan;
    }
    return false;
}

int Facade::getSoLanHuyCuaKhach(const std::string& sdt) const {
    auto it = mapIndexKhachSDT_.find(sdt);
    if (it != mapIndexKhachSDT_.end() && it->second < danhSachKhachHang_.size()) {
        return danhSachKhachHang_[it->second].soLanHuy;
    }
    return 0;
}

int Facade::getSoLanDatCuaKhach(const std::string& sdt) const {
    auto it = mapIndexKhachSDT_.find(sdt);
    if (it != mapIndexKhachSDT_.end() && it->second < danhSachKhachHang_.size()) {
        return danhSachKhachHang_[it->second].soLanDat;
    }
    return 0;
}

int Facade::getMucUuDaiCuaKhach(const std::string& sdt) const {
    auto it = mapIndexKhachSDT_.find(sdt);
    if (it != mapIndexKhachSDT_.end() && it->second < danhSachKhachHang_.size()) {
        const auto& kh = danhSachKhachHang_[it->second];
        if (kh.biChan) return 0;
        return kh.mucUuDaiPhanTram;
    }
    return 0;
}

KetQua Facade::datSanMoi(
    const std::string& maSan,
    const std::string& sdtKhach,
    const std::string& tenKhach,
    const std::string& ngay,
    const std::string& khungGio,
    int giaGoc,
    int giaGiam,
    std::string& outMaDon
) {
    auto chuanHoaNgay = [](const std::string& value) {
        if (value.size() == 10U && value[2] == '/' && value[5] == '/') return value.substr(6, 4) + "-" + value.substr(3, 2) + "-" + value.substr(0, 2);
        return value;
    };
    const std::string bookingDate = chuanHoaNgay(ngay);
    if (!DateTime::isValidDate(bookingDate) || DateTime::isPastDate(bookingDate)) return {false, u8"Không thể đặt sân vào ngày trong quá khứ."};
    if (bookingDate == DateTime::today() && khungGio.size() >= 5U && DateTime::compareTime(khungGio.substr(0, 5), DateTime::nowTime()) <= 0) return {false, u8"Khung giờ đã qua hoặc đang diễn ra hôm nay."};
    if (giaGoc <= 0 || giaGiam < 0 || giaGiam > giaGoc) return {false, u8"Giá thuê hoặc ưu đãi không hợp lệ."};
    if (laKhachHangBiChan(sdtKhach)) {
        return {false, u8"Tài khoản của bạn đang bị khóa do vi phạm. Vui lòng liên hệ chủ sân!"};
    }

    // Kiểm tra trùng đơn
    for (const auto& don : danhSachDonDat_) {
        if (don.maSan == maSan && don.ngay == ngay && don.khungGio == khungGio && don.trangThai != u8"Đã hủy") {
            return {false, u8"Khung giờ này vừa có người đặt trước. Vui lòng chọn khung giờ khác!"};
        }
    }

    char buf[32];
    std::snprintf(buf, sizeof(buf), "BK-%04zu", danhSachDonDat_.size() + 101);
    outMaDon = buf;

    int tongTien = giaGoc - giaGiam;
    if (tongTien < 0) tongTien = 0;

    DonDatSanInfo donMoi;
    donMoi.maDon = outMaDon;
    donMoi.maSan = maSan;
    donMoi.tenKhachHang = tenKhach;
    donMoi.sdt = sdtKhach;
    donMoi.ngay = ngay;
    donMoi.khungGio = khungGio;
    donMoi.giaTien = tongTien;
    donMoi.trangThai = u8"Đã xác nhận";

    danhSachDonDat_.insert(danhSachDonDat_.begin(), donMoi);

    // Cập nhật số lần đặt cho khách
    auto it = mapIndexKhachSDT_.find(sdtKhach);
    if (it != mapIndexKhachSDT_.end() && it->second < danhSachKhachHang_.size()) {
        // Lượt đặt chỉ được cộng khi chủ sân xác nhận đã đá xong.
    }

    // Cập nhật Customer object trong session nếu có
    User* curUser = Session::getInstance().getUser();
    auto customer = dynamic_cast<::Customer*>(curUser);
    if (customer && customer->get_phone_number() == sdtKhach) {
        // Thống kê Customer được cập nhật khi đơn hoàn thành.
    }

    luuDuLieu();
    return {true, ""};
}

KetQua Facade::huyDonDat(const std::string& maDon, const std::string& sdtKhach) {
    for (auto& don : danhSachDonDat_) {
        if (don.maDon == maDon) {
            if (don.trangThai == u8"Đã hủy") {
                return {false, u8"Đơn đặt này đã được hủy trước đó."};
            }
            don.trangThai = u8"Đã hủy";

            // Tăng số lần hủy
            auto it = mapIndexKhachSDT_.find(sdtKhach);
            if (it != mapIndexKhachSDT_.end() && it->second < danhSachKhachHang_.size()) {
                danhSachKhachHang_[it->second].soLanHuy++;
                if (danhSachKhachHang_[it->second].soLanHuy >= 3) {
                    danhSachKhachHang_[it->second].biChan = true;
                    danhSachKhachHang_[it->second].lyDoChan = u8"Hủy sân tự động quá 5 lần";
                    danhSachKhachHang_[it->second].ngayChan = "17/10/2026";
                }
            }

            User* curUser = Session::getInstance().getUser();
            auto customer = dynamic_cast<::Customer*>(curUser);
            if (customer && customer->get_phone_number() == sdtKhach) {
                customer->increase_cancel_count();
            }

            luuDuLieu();
            return {true, ""};
        }
    }
    return {false, u8"Không tìm thấy đơn đặt sân cần hủy."};
}

KetQua Facade::xacNhanDaHoanThanhDon(const std::string& maDon) {
    for (DonDatSanInfo& don : danhSachDonDat_) {
        if (don.maDon != maDon) continue;
        if (don.trangThai == u8"Đã hủy") return {false, u8"Không thể xác nhận đơn đã hủy."};
        if (don.trangThai == u8"Đã hoàn thành") return {false, u8"Đơn này đã được xác nhận hoàn thành."};
        don.trangThai = u8"Đã hoàn thành";
        auto it = mapIndexKhachSDT_.find(don.sdt);
        if (it != mapIndexKhachSDT_.end() && it->second < danhSachKhachHang_.size()) {
            KhachHangInfo& customer = danhSachKhachHang_[it->second];
            ++customer.soLanDat;
            customer.mucUuDaiPhanTram = 0;
            for (const NguongUuDai& threshold : danhSachNguongUuDai_) {
                if (customer.soLanDat >= threshold.soLanToiThieu)
                    customer.mucUuDaiPhanTram = threshold.phanTramGiam;
            }
        }
        for (const auto& user : danhSachUser_) {
            if (user && user->get_phone_number() == don.sdt) {
                auto customer = std::dynamic_pointer_cast<Customer>(user);
                if (customer) customer->recordBooking();
                break;
            }
        }
        luuDuLieu();
        return {true, ""};
    }
    return {false, u8"Không tìm thấy đơn đặt sân."};
}

std::vector<GiaiDauInfo> Facade::getGiaiDauCuaUser(const std::string& sdt) const {
    std::vector<GiaiDauInfo> ds;
    for (const auto& g : danhSachGiaiDau_) {
        if (g.sdtKhach == sdt) {
            ds.push_back(g);
        }
    }
    return ds;
}

KetQua Facade::guiYeuCauTaoGiai(const GiaiDauInfo& giaiMoi) {
    if (laKhachHangBiChan(giaiMoi.sdtKhach)) {
        return {false, u8"Tài khoản đang bị khóa nên không thể gửi yêu cầu tạo giải."};
    }
    auto chuanHoaNgay = [](const std::string& value) {
        if (value.size() == 10U && value[2] == '/' && value[5] == '/')
            return value.substr(6, 4) + "-" + value.substr(3, 2) + "-" + value.substr(0, 2);
        return value;
    };
    const std::string start = chuanHoaNgay(giaiMoi.ngayBatDau);
    const std::string end = chuanHoaNgay(giaiMoi.ngayKetThuc);
    if (!DateTime::isValidDate(start) || DateTime::isPastDate(start)) {
        return {false, u8"Ngày bắt đầu giải không hợp lệ hoặc đã qua."};
    }
    if (!DateTime::isValidDate(end) || DateTime::compareDate(end, start) < 0) {
        return {false, u8"Ngày kết thúc phải bằng hoặc sau ngày bắt đầu."};
    }
    GiaiDauInfo g = giaiMoi;
    if (g.maGiai.empty()) {
        char buf[32];
        std::snprintf(buf, sizeof(buf), "GD-%02zu", danhSachGiaiDau_.size() + 1);
        g.maGiai = buf;
    }
    g.trangThai = TrangThaiGiaiDau::ChoDuyet;
    danhSachGiaiDau_.push_back(g);
    luuDuLieu();
    return {true, ""};
}

std::vector<SanBongInfo> Facade::getDanhSachSanDaThueXong(const std::string& sdt) const {
    std::vector<SanBongInfo> ds;
    std::unordered_map<std::string, bool> daThem;

    for (const auto& don : danhSachDonDat_) {
        if (don.sdt == sdt && don.trangThai != u8"Đã hủy") {
            if (!daThem[don.maSan]) {
                for (const auto& s : danhSachSan_) {
                    if (s.maSan == don.maSan) {
                        ds.push_back(s);
                        daThem[don.maSan] = true;
                        break;
                    }
                }
            }
        }
    }
    if (ds.empty() && !danhSachSan_.empty()) {
        ds.push_back(danhSachSan_[0]);
    }
    return ds;
}

KetQua Facade::guiHoacSuaDanhGia(
    const std::string& sdt,
    const std::string& tenKhach,
    const std::string& maSan,
    int soSao,
    const std::string& noiDung,
    const std::string& danhGiaId
) {
    if (!danhGiaId.empty()) {
        for (auto& dg : danhSachDanhGia_) {
            if (dg.id == danhGiaId) {
                dg.soSao = soSao;
                dg.noiDung = noiDung;
                luuDuLieu();
                return {true, ""};
            }
        }
    }

    DanhGiaInfo dgMoi;
    char buf[32];
    std::snprintf(buf, sizeof(buf), "RV%02zu", danhSachDanhGia_.size() + 1);
    dgMoi.id = buf;
    dgMoi.sdt = sdt;
    dgMoi.tenKhach = tenKhach;
    dgMoi.maSan = maSan;
    dgMoi.soSao = soSao;
    dgMoi.noiDung = noiDung;
    dgMoi.ngayGui = "17/10/2026";

    danhSachDanhGia_.insert(danhSachDanhGia_.begin(), dgMoi);
    luuDuLieu();
    return {true, ""};
}

KetQua Facade::capNhatThongTinUser(const std::string& sdt, const std::string& hoTenMoi) {
    return capNhatThongTinUser(sdt, hoTenMoi, sdt);
}

KetQua Facade::capNhatThongTinUser(const std::string& sdtCu, const std::string& hoTenMoi,
                                   const std::string& sdtMoi) {
    if (sdtMoi.empty()) return {false, u8"Số điện thoại không được để trống."};
    if (sdtCu != sdtMoi && kiemTraSDTDaTonTai(sdtMoi)) {
        return {false, u8"Số điện thoại mới đã được dùng bởi tài khoản khác."};
    }
    for (auto& u : danhSachUser_) {
        if (u && u->get_phone_number() == sdtCu) {
            u->rename(hoTenMoi);
            u->update_phone_number(sdtMoi);
            break;
        }
    }

    auto it = mapIndexKhachSDT_.find(sdtCu);
    if (it != mapIndexKhachSDT_.end() && it->second < danhSachKhachHang_.size()) {
        danhSachKhachHang_[it->second].hoTen = hoTenMoi;
        danhSachKhachHang_[it->second].sdt = sdtMoi;
        mapIndexKhachSDT_.erase(it);
        mapIndexKhachSDT_[sdtMoi] = it->second;
    }

    auto passwordIt = mapMatKhau_.find(sdtCu);
    if (passwordIt != mapMatKhau_.end() && sdtCu != sdtMoi) {
        mapMatKhau_[sdtMoi] = passwordIt->second;
        mapMatKhau_.erase(passwordIt);
    }

    luuDuLieu();
    return {true, ""};
}

KetQua Facade::doiMatKhauUser(const std::string& sdt, const std::string& matKhauCu, const std::string& matKhauMoi) {
    auto it = mapMatKhau_.find(sdt);
    std::string curPw = (it != mapMatKhau_.end()) ? it->second : "123456";

    if (curPw != matKhauCu) {
        return {false, u8"Mật khẩu hiện tại không chính xác!"};
    }

    mapMatKhau_[sdt] = matKhauMoi;
    for (auto& u : danhSachUser_) {
        if (u && u->get_phone_number() == sdt) {
            u->update_pw(matKhauMoi);
            break;
        }
    }

    luuDuLieu();
    return {true, ""};
}

// ==========================================
// 9. CÁC HÀM NGHIỆP VỤ TODO
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
