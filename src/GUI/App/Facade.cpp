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
#include "Repository/VenueRepository.h"
#include "Repository/BookingRepository.h"
#include "Repository/CustomerRepository.h"
#include "Repository/FieldRepository.h"
#include "Service/BookingService.h"
#include "Service/CustomerService.h"
#include "Service/FieldService.h"
#include "Field/Field.h"

namespace GUI::App {

namespace {
int parseNgaySangSo(const std::string& str);

bool tachKhungGio(const std::string& text, std::string& start,
                  std::string& end, int& duration) {
    const std::size_t separator = text.find(" - ");
    if (separator == std::string::npos) return false;
    start = text.substr(0, separator);
    end = text.substr(separator + 3);
    if (!DateTime::isValidTime(start) || !DateTime::isValidTime(end)) return false;
    duration = DateTime::compareTime(end, start);
    if (duration <= 0) return false;
    const int startMinutes = std::stoi(start.substr(0, 2)) * 60 +
                             std::stoi(start.substr(3, 2));
    const int endMinutes = std::stoi(end.substr(0, 2)) * 60 +
                           std::stoi(end.substr(3, 2));
    duration = endMinutes - startMinutes;
    return duration == 60 || duration == 90;
}

void tachTruong(const std::string& line, LinkedList<std::string>& fields) {
    fields.clear();
    std::istringstream input(line);
    std::string value;
    while (std::getline(input, value, '|')) fields.push_back(value);
}

const std::string* truongTai(const LinkedList<std::string>& fields, int index) {
    auto* node = fields.get_head();
    while (node != nullptr && index > 0) {
        node = node->next;
        --index;
    }
    return node == nullptr ? nullptr : &node->data;
}

DonDatSanInfo toBookingInfo(const ReportBookingRecord& record) {
    DonDatSanInfo booking;
    booking.maDon = record.bookingId;
    booking.maSan = record.fieldId;
    booking.ownerUserId = record.ownerUserId;
    booking.customerId = record.customerId;
    booking.tenKhachHang = record.customerName;
    booking.sdt = record.customerPhone;
    booking.ngay = record.date;
    booking.khungGio = record.timeSlot.empty()
        ? record.startTime + " - " + record.endTime
        : record.timeSlot;
    booking.thoiLuongPhut = record.durationMinutes;
    booking.giaTien = static_cast<int>(record.amountVnd);
    booking.trangThai = record.status;
    return booking;
}

bool ownerOwnsField(const std::string& ownerId, const std::string& fieldId,
                    std::string& error) {
    VenueRepository repository;
    VenueService service(repository);
    LinkedList<VenueFieldRecord> fields;
    if (!service.getFields(ownerId, 0, true, fields, error)) return false;
    for (auto* node = fields.get_head(); node != nullptr; node = node->next) {
        if (node->data.fieldId == fieldId) return true;
    }
    error = "Sân không thuộc chủ sân đang đăng nhập.";
    return false;
}
}

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

    VenueRepository venueRepository;
    std::string venueError;
    if (!venueRepository.migrateLegacyData(venueError) ||
        !venueRepository.seedIfNeeded(venueError)) {
        std::cerr << venueError << '\n';
    }

    if (!venueRepository.loadPrices(bangGiaCoSo_, venueError)) {
        std::cerr << venueError << '\n';
    }

    // 7. Khởi tạo cấu hình Ngưỡng ưu đãi
    danhSachNguongUuDai_ = {
        {1, 5,  5,  u8"Khách thân thiết"},
        {2, 10, 10, u8"Khách hàng VIP Bạc"},
        {3, 20, 15, u8"Khách hàng VIP Vàng"}
    };

    ReportRepository repository;
    LinkedList<ReportFieldRecord> fields;
    LinkedList<ReportBookingRecord> bookings;
    std::string storageError;
    if (repository.loadFields(fields, storageError)) {
        for (auto* node = fields.get_head(); node != nullptr; node = node->next) {
            SanBongInfo field;
            field.maSan = node->data.fieldId;
            field.ownerUserId = node->data.ownerUserId;
            field.tenSan = node->data.name;
            field.loaiSan = node->data.size;
            field.giaThueGio = static_cast<int>(node->data.hourlyRateVnd);
            field.dangBaoTri = node->data.underMaintenance;
            field.quyDinh = node->data.rules;
            danhSachSan_.push_back(field);
        }
    } else {
        std::cerr << storageError << '\n';
    }
    if (repository.loadBookings(bookings, storageError)) {
        for (auto* node = bookings.get_head(); node != nullptr; node = node->next) {
            DonDatSanInfo booking;
            booking.maDon = node->data.bookingId;
            booking.maSan = node->data.fieldId;
            booking.ownerUserId = node->data.ownerUserId;
            booking.customerId = node->data.customerId;
            booking.tenKhachHang = node->data.customerName;
            booking.sdt = node->data.customerPhone;
            booking.ngay = node->data.date;
            booking.khungGio = node->data.timeSlot;
            booking.thoiLuongPhut = node->data.durationMinutes;
            booking.giaTien = static_cast<int>(node->data.amountVnd);
            booking.trangThai = node->data.status;
            danhSachDonDat_.push_back(booking);
        }
    } else {
        std::cerr << storageError << '\n';
    }

    FileManager manager;
    LinkedList<std::string> lines;
    if (manager.readAllLines("data/reviews.txt", lines, storageError)) {
        for (auto* node = lines.get_head(); node != nullptr; node = node->next) {
            LinkedList<std::string> fields;
            tachTruong(node->data, fields);
            if (fields.size() < 7) continue;
            DanhGiaInfo review;
            review.id = *truongTai(fields, 0);
            review.tenKhach = *truongTai(fields, 1);
            review.sdt = *truongTai(fields, 2);
            review.maSan = *truongTai(fields, 3);
            try {
                review.soSao = std::stoi(*truongTai(fields, 4));
            } catch (const std::exception&) {
                continue;
            }
            review.ngayGui = *truongTai(fields, 5);
            review.noiDung = *truongTai(fields, 6);
            danhSachDanhGia_.push_back(review);
        }
    } else {
        std::cerr << storageError << '\n';
    }
    if (manager.readAllLines("data/tournaments.txt", lines, storageError)) {
        for (auto* node = lines.get_head(); node != nullptr; node = node->next) {
            LinkedList<std::string> fields;
            tachTruong(node->data, fields);
            if (fields.size() < 11) continue;
            GiaiDauInfo tournament;
            tournament.maGiai = *truongTai(fields, 0);
            tournament.tenGiai = *truongTai(fields, 1);
            tournament.tenKhachDat = *truongTai(fields, 2);
            tournament.sdtKhach = *truongTai(fields, 3);
            tournament.ngayBatDau = *truongTai(fields, 4);
            tournament.ngayKetThuc = *truongTai(fields, 5);
            tournament.maSan = *truongTai(fields, 6);
            tournament.khungGio = *truongTai(fields, 7);
            tournament.moTa = *truongTai(fields, 8);
            tournament.ghiChuTuChoi = *truongTai(fields, 9);
            try {
                const int status = std::stoi(*truongTai(fields, 10));
                if (status >= 0 && status <= 4) {
                    tournament.trangThai =
                        static_cast<TrangThaiGiaiDau>(status);
                }
            } catch (const std::exception&) {
                continue;
            }
            danhSachGiaiDau_.push_back(tournament);
        }
    } else {
        std::cerr << storageError << '\n';
    }

    if (manager.readAllLines("data/blacklist.txt", lines, storageError)) {
        for (auto* node = lines.get_head(); node != nullptr; node = node->next) {
            LinkedList<std::string> fields;
            tachTruong(node->data, fields);
            if (fields.size() < 4 || *truongTai(fields, 3) != "1") continue;
            KhachHangInfo customer;
            customer.id = *truongTai(fields, 0);
            customer.sdt = customer.id;
            customer.biChan = true;
            customer.lyDoChan = *truongTai(fields, 1);
            customer.ngayChan = *truongTai(fields, 2);
            mapIndexKhachSDT_[customer.sdt] = danhSachKhachHang_.size();
            danhSachKhachHang_.push_back(customer);
        }
    } else {
        std::cerr << storageError << '\n';
    }

    daNapDuLieu_ = true;
}

void Facade::luuDuLieu() {
    Storage::FileManager::ghiDanhSachUser(danhSachUser_);
    std::string error;
    FileManager manager;
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
bool Facade::getBaoCaoDashboard(
    ReportDashboardMetrics& metrics,
    LinkedList<ReportRevenueDay>& doanhThuTheoNgay,
    LinkedList<ReportBookingRecord>& luotDatGanDay,
    std::string& error
) const {
    User* user = Session::getInstance().getUser();
    if (user == nullptr || user->get_user_id().empty()) {
        error = u8"Không xác định được chủ sân đang đăng nhập.";
        return false;
    }

    ReportRepository repository;
    ReportService service;
    return service.buildOwnerDashboard(
        user->get_user_id(),
        repository,
        metrics,
        doanhThuTheoNgay,
        luotDatGanDay,
        error
    );
}

bool Facade::getDanhSachCoSo(
    LinkedList<VenueFacilityRecord>& facilities,
    std::string& error
) const {
    VenueRepository repository;
    VenueService service(repository);
    return service.getFacilities(facilities, error);
}

bool Facade::getGiaThapNhatCoSo(
    const std::string& ownerId,
    long long& giaVnd,
    std::string& error
) const {
    VenueRepository repository;
    VenueService service(repository);
    return service.getLowestPrice(ownerId, giaVnd, error);
}

bool Facade::getDanhSachSanTheoChuSan(
    const std::string& ownerId,
    int loaiSan,
    bool gomSanKhongNhanDat,
    LinkedList<VenueFieldRecord>& fields,
    std::string& error
) const {
    VenueRepository repository;
    VenueService service(repository);
    return service.getFields(
        ownerId, loaiSan, gomSanKhongNhanDat, fields, error
    );
}

bool Facade::getKhungGioSan(
    const std::string& fieldId,
    const std::string& ngay,
    int thoiLuongPhut,
    LinkedList<VenueSlotRecord>& slots,
    std::string& error,
    bool choPhepNgayQuaKhu
) const {
    VenueRepository repository;
    VenueService service(repository);
    return service.getSlots(
        fieldId, ngay, thoiLuongPhut, slots, error, choPhepNgayQuaKhu
    );
}

bool Facade::datSanTheoKhungGio(
    const VenueBookingRequest& request,
    std::string& bookingId,
    std::string& error
) const {
    FileManager fileManager;
    BookingRepository bookingRepository(fileManager);
    CustomerRepository customerRepository(fileManager);
    CustomerService customerService(customerRepository);
    BookingService service(bookingRepository, customerService);
    return service.createBooking(request, bookingId, error);
}

bool Facade::datNhieuKhungGio(
    const LinkedList<VenueBookingRequest>& requests,
    LinkedList<std::string>& bookingIds,
    std::string& error
) const {
    FileManager fileManager;
    BookingRepository bookingRepository(fileManager);
    CustomerRepository customerRepository(fileManager);
    CustomerService customerService(customerRepository);
    BookingService service(bookingRepository, customerService);
    return service.createBookings(requests, bookingIds, error);
}

bool Facade::capNhatGiaCoSo(
    int loaiSan,
    int thoiLuongPhut,
    const std::string& gioBatDau,
    long long giaVnd,
    std::string& error
) {
    User* user = Session::getInstance().getUser();
    if (user == nullptr || user->get_account_type() != "Owner") {
        error = "Cần đăng nhập bằng tài khoản chủ sân để sửa bảng giá.";
        return false;
    }
    VenueRepository repository;
    VenueService service(repository);
    if (!service.setFieldPrice(
            user->get_user_id(), loaiSan, thoiLuongPhut, gioBatDau, giaVnd,
            error)) {
        return false;
    }
    return repository.loadPrices(bangGiaCoSo_, error);
}

// ==========================================
// 4. QUẢN LÝ SÂN BÓNG (CRUD & WIZARD)
// ==========================================
std::vector<SanBongInfo> Facade::getDanhSachSanBong() const {
    return danhSachSan_;
}

std::vector<SanBongInfo> Facade::getDanhSachSanBongCuaChuSan() const {
    User* user = Session::getInstance().getUser();
    std::vector<SanBongInfo> fields;
    if (user == nullptr || user->get_account_type() != "Owner") {
        return fields;
    }
    LinkedList<VenueFacilityRecord> facilities;
    VenueRepository repository;
    std::string error;
    bool allows60Minutes = false;
    bool allows90Minutes = false;
    if (!repository.loadFacilities(facilities, error)) {
        std::cerr << error << '\n';
        return fields;
    }
    for (auto* node = facilities.get_head(); node != nullptr;
         node = node->next) {
        if (node->data.ownerId != user->get_user_id()) continue;
        allows60Minutes = node->data.allows60Minutes;
        allows90Minutes = node->data.allows90Minutes;
        break;
    }
    for (const SanBongInfo& field : danhSachSan_) {
        if (field.ownerUserId == user->get_user_id()) {
            SanBongInfo viewField = field;
            viewField.giaThueGio = 0;
            for (auto* node = bangGiaCoSo_.get_head();
                 node != nullptr; node = node->next) {
                const VenuePriceRecord& price = node->data;
                if (price.ownerId == user->get_user_id() &&
                    price.fieldSize == field.loaiSan &&
                    price.durationMinutes > 0 && price.priceVnd > 0 &&
                    ((price.durationMinutes == 60 && allows60Minutes) ||
                     (price.durationMinutes == 90 && allows90Minutes))) {
                    const long long hourlyPrice =
                        price.priceVnd * 60 / price.durationMinutes;
                    if (viewField.giaThueGio == 0 ||
                        hourlyPrice < viewField.giaThueGio) {
                        viewField.giaThueGio =
                            static_cast<int>(hourlyPrice);
                    }
                }
            }
            fields.push_back(viewField);
        }
    }
    return fields;
}

SanBongInfo* Facade::getSanBong(const std::string& maSan) {
    User* owner = Session::getInstance().getUser();
    if (owner == nullptr || owner->get_account_type() != "Owner") {
        return nullptr;
    }
    for (auto& s : danhSachSan_) {
        if (s.maSan == maSan && s.ownerUserId == owner->get_user_id()) {
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
    User* owner = Session::getInstance().getUser();
    if (owner == nullptr || owner->get_account_type() != "Owner") {
        return {false, u8"Vui lòng đăng nhập bằng tài khoản chủ sân trước khi tạo sân."};
    }
    if (loaiSan != 5 && loaiSan != 7 && loaiSan != 11) {
        return {false, u8"Loại sân không hợp lệ."};
    }
    if (soLuong <= 0) {
        return {false, u8"Số lượng sân cần tạo phải lớn hơn 0."};
    }
    VenueRepository repository;
    LinkedList<VenueFieldRecord> fields;
    std::string error;
    if (!repository.loadFields(fields, error)) return {false, error};
    int sequence = 1;
    for (auto* node = fields.get_head(); node != nullptr; node = node->next) {
        const std::string prefix = "S" + std::to_string(loaiSan) + "-";
        if (node->data.fieldId.rfind(prefix, 0) == 0) {
            try {
                const int existing = std::stoi(node->data.fieldId.substr(prefix.size()));
                if (existing >= sequence) sequence = existing + 1;
            } catch (const std::exception&) {
                // Mã sân không theo hậu tố số được bỏ qua khi tạo mã mới.
            }
        }
    }
    FileManager fileManager;
    FieldRepository fieldRepository(fileManager);
    FieldService service(fieldRepository);
    for (int i = 0; i < soLuong; ++i) {
        const int stt = sequence + i;
        char maBuf[16];
        std::snprintf(maBuf, sizeof(maBuf), "S%d-%02d", loaiSan, stt);
        VenueFieldRecord newField;
        newField.fieldId = maBuf;
        newField.ownerId = owner->get_user_id();
        newField.name = u8"Sân " + std::to_string(loaiSan) + u8" số " +
                        (stt < 10 ? "0" : "") + std::to_string(stt);
        newField.size = loaiSan;
        newField.rules = quyDinh;
        if (!service.createField(owner->get_user_id(), newField, error)) {
            return {false, error};
        }
        SanBongInfo displayField;
        displayField.maSan = newField.fieldId;
        displayField.ownerUserId = newField.ownerId;
        displayField.tenSan = newField.name;
        displayField.loaiSan = newField.size;
        displayField.quyDinh = newField.rules;
        danhSachSan_.push_back(displayField);
        outMaSan.push_back(maBuf);
    }
    if (!repository.loadPrices(bangGiaCoSo_, error)) {
        return {false, error};
    }
    return {true, ""};
}

KetQua Facade::suaSanBong(const std::string& maSan, const std::string& tenSan, int loaiSan, int giaThue) {
    (void)giaThue;
    auto san = getSanBong(maSan);
    if (!san) {
        return {false, u8"Không tìm thấy sân bóng với mã: " + maSan};
    }
    User* owner = Session::getInstance().getUser();
    if (owner == nullptr) return {false, u8"Vui lòng đăng nhập lại."};
    VenueRepository repository;
    LinkedList<VenueFieldRecord> fields;
    std::string error;
    if (!repository.loadFields(fields, error)) return {false, error};
    VenueFieldRecord* updatedField = nullptr;
    for (auto* node = fields.get_head(); node != nullptr; node = node->next) {
        VenueFieldRecord& field = node->data;
        if (field.fieldId != maSan) continue;
        if (field.ownerId != owner->get_user_id()) {
            return {false, u8"Không có quyền sửa sân của chủ sân khác."};
        }
        field.name = tenSan;
        field.size = loaiSan;
        updatedField = &field;
        break;
    }
    if (updatedField == nullptr) {
        return {false, u8"Không tìm thấy sân bóng cần cập nhật."};
    }
    VenueService venueService(repository);
    if (!venueService.updateField(owner->get_user_id(), *updatedField, error)) {
        return {false, error};
    }
    san->tenSan = tenSan;
    san->loaiSan = loaiSan;
    return {true, ""};
}

KetQua Facade::toggleBaoTriSan(const std::string& maSan) {
    auto san = getSanBong(maSan);
    if (!san) {
        return {false, u8"Không tìm thấy sân bóng với mã: " + maSan};
    }
    User* owner = Session::getInstance().getUser();
    if (owner == nullptr) return {false, u8"Vui lòng đăng nhập lại."};
    FileManager fileManager;
    FieldRepository fieldRepository(fileManager);
    FieldService service(fieldRepository);
    std::string error;
    const FieldStatus status = san->dangBaoTri
        ? FieldStatus::ACTIVE
        : FieldStatus::MAINTENANCE;
    if (!service.setFieldStatus(owner->get_user_id(), maSan, status, error)) {
        return {false, error};
    }
    san->dangBaoTri = !san->dangBaoTri;
    return {true, ""};
}

KetQua Facade::xoaSanBong(const std::string& maSan) {
    User* owner = Session::getInstance().getUser();
    if (owner == nullptr || owner->get_account_type() != "Owner") {
        return {false, u8"Vui lòng đăng nhập bằng tài khoản chủ sân."};
    }
    VenueRepository repository;
    VenueService service(repository);
    std::string error;
    if (!service.deleteField(owner->get_user_id(), maSan, error)) {
        return {false, error};
    }
    auto it = std::remove_if(danhSachSan_.begin(), danhSachSan_.end(), [&](const SanBongInfo& s) {
        return s.maSan == maSan && s.ownerUserId == owner->get_user_id();
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
    std::vector<KhungGioInfo> slots;
    User* owner = Session::getInstance().getUser();
    if (owner == nullptr || owner->get_account_type() != "Owner") return slots;
    LinkedList<VenueFacilityRecord> facilities;
    VenueRepository repository;
    std::string error;
    if (!repository.loadFacilities(facilities, error)) {
        std::cerr << error << '\n';
        return slots;
    }
    std::string openTime;
    std::string closeTime;
    int durationMinutes = 0;
    for (auto* node = facilities.get_head(); node != nullptr; node = node->next) {
        if (node->data.ownerId == owner->get_user_id()) {
            openTime = node->data.openTime;
            closeTime = node->data.closeTime;
            durationMinutes = node->data.allows90Minutes
                ? 90
                : (node->data.allows60Minutes ? 60 : 0);
            break;
        }
    }
    if (durationMinutes == 0) return slots;
    const auto minutes = [](const std::string& time) {
        if (!DateTime::isValidTime(time)) return -1;
        return std::stoi(time.substr(0, 2)) * 60 +
               std::stoi(time.substr(3, 2));
    };
    const int opening = minutes(openTime);
    const int closing = minutes(closeTime);
    std::map<std::string, bool> starts;
    for (auto* node = bangGiaCoSo_.get_head(); node != nullptr; node = node->next) {
        const VenuePriceRecord& price = node->data;
        if (price.ownerId != owner->get_user_id() ||
            price.durationMinutes != durationMinutes) {
            continue;
        }
        const int start = minutes(price.startTime);
        if (start < opening || start + durationMinutes > closing) continue;
        starts[price.startTime] = true;
    }
    int index = 1;
    for (const auto& item : starts) {
        const int start = minutes(item.first);
        const int end = start + durationMinutes;
        char endText[6];
        std::snprintf(
            endText, sizeof(endText), "%02d:%02d", end / 60, end % 60
        );
        char id[16];
        std::snprintf(id, sizeof(id), "KG%02d", index++);
        slots.push_back({
            id, item.first, endText, start >= 17 * 60 && start < 21 * 60
        });
    }
    return slots;
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
    User* owner = Session::getInstance().getUser();
    if (owner == nullptr || owner->get_account_type() != "Owner") {
        return {false, u8"Cần đăng nhập bằng tài khoản chủ sân để sửa giờ hoạt động."};
    }
    if (!DateTime::isValidTime(gioMo) || !DateTime::isValidTime(gioDong) ||
        endM <= startM || (buocPhut != 60 && buocPhut != 90) ||
        endM - startM < buocPhut) {
        return {false, u8"Giờ hoạt động hoặc thời lượng ca không hợp lệ."};
    }
    VenueRepository repository;
    VenueService service(repository);
    std::string error;
    if (!service.updateFacilityHours(
            owner->get_user_id(), gioMo, gioDong, buocPhut == 60,
            buocPhut == 90, error)) {
        return {false, error};
    }
    for (const auto& account : danhSachUser_) {
        if (!account || account->get_user_id() != owner->get_user_id()) continue;
        auto ownerAccount = std::dynamic_pointer_cast<Owner>(account);
        if (ownerAccount) {
            ownerAccount->set_open_time(gioMo);
            ownerAccount->set_close_time(gioDong);
            ownerAccount->setAllowedSlotDurations(
                buocPhut == 60, buocPhut == 90
            );
        }
        break;
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

        cur += buocPhut;
        stt++;
    }
    if (!repository.loadPrices(bangGiaCoSo_, error)) {
        return {false, error};
    }
    return {true, ""};
}

int Facade::getGiaMaTran(const std::string& khungGioStr, int loaiSan) const {
    User* owner = Session::getInstance().getUser();
    if (owner == nullptr || owner->get_account_type() != "Owner") return 0;
    std::string start;
    std::string end;
    int duration = 0;
    if (!tachKhungGio(khungGioStr, start, end, duration)) return 0;
    for (auto* node = bangGiaCoSo_.get_head(); node != nullptr; node = node->next) {
        const VenuePriceRecord& price = node->data;
        if (price.ownerId == owner->get_user_id() &&
            price.fieldSize == loaiSan && price.durationMinutes == duration &&
            price.startTime == start) {
            return static_cast<int>(price.priceVnd);
        }
    }
    return 0;
}

KetQua Facade::capNhatGiaMaTran(
    const std::string& khungGioStr,
    int loaiSan,
    int giaMoi,
    bool apDungTatCaCungLoai
) {
    std::string start;
    std::string end;
    int duration = 0;
    if (!tachKhungGio(khungGioStr, start, end, duration)) {
        return {false, u8"Khung giờ hoặc thời lượng không hợp lệ."};
    }
    User* owner = Session::getInstance().getUser();
    if (owner == nullptr || owner->get_account_type() != "Owner") {
        return {false, u8"Cần đăng nhập bằng tài khoản chủ sân để sửa giá."};
    }
    VenueRepository repository;
    VenueService service(repository);
    std::string error;
    if (!service.setFieldPrice(
            owner->get_user_id(), loaiSan, duration, start, giaMoi, error,
            apDungTatCaCungLoai)) {
        return {false, error};
    }
    if (!repository.loadPrices(bangGiaCoSo_, error)) {
        return {false, error};
    }
    return {true, ""};
}

std::map<std::pair<std::string, std::string>, Widgets::SlotInfo> Facade::getTinhTrangSlotTheoNgay(const std::string& ngay) {
    std::map<std::pair<std::string, std::string>, Widgets::SlotInfo> maTran;
    User* owner = Session::getInstance().getUser();
    if (owner == nullptr || owner->get_account_type() != "Owner") return maTran;
    LinkedList<VenueFieldRecord> fields;
    std::string error;
    if (!getDanhSachSanTheoChuSan(owner->get_user_id(), 0, true, fields, error)) {
        std::cerr << error << '\n';
        return maTran;
    }
    LinkedList<VenueFacilityRecord> facilities;
    if (!getDanhSachCoSo(facilities, error)) {
        std::cerr << error << '\n';
        return maTran;
    }
    int durationMinutes = 0;
    for (auto* node = facilities.get_head(); node != nullptr; node = node->next) {
        if (node->data.ownerId != owner->get_user_id()) continue;
        durationMinutes = node->data.allows90Minutes
            ? 90
            : (node->data.allows60Minutes ? 60 : 0);
        break;
    }
    if (durationMinutes == 0) return maTran;
    const std::vector<KhungGioInfo> ownerSlots = getDanhSachKhungGio();
    for (auto* fieldNode = fields.get_head(); fieldNode != nullptr;
         fieldNode = fieldNode->next) {
        const VenueFieldRecord& field = fieldNode->data;
        if (field.status != 0) {
            for (const KhungGioInfo& slot : ownerSlots) {
                Widgets::SlotInfo info;
                info.status = Widgets::SlotStatus::BaoTri;
                info.ghiChu = field.rules;
                maTran[{field.fieldId, slot.batDau + " - " + slot.ketThuc}] = info;
            }
            continue;
        }
        LinkedList<VenueSlotRecord> slots;
        if (!getKhungGioSan(
                field.fieldId, ngay, durationMinutes, slots, error, true)) {
            std::cerr << error << '\n';
            continue;
        }
        for (auto* slotNode = slots.get_head(); slotNode != nullptr;
             slotNode = slotNode->next) {
            const VenueSlotRecord& slot = slotNode->data;
            Widgets::SlotInfo info;
            info.status = !slot.bookingId.empty()
                ? Widgets::SlotStatus::DaDat
                : (slot.past
                    ? Widgets::SlotStatus::DaQua
                    : Widgets::SlotStatus::Trong);
            info.giaTien = static_cast<int>(slot.priceVnd);
            info.ghiChu = slot.bookingId;
            maTran[{field.fieldId, slot.startTime + " - " + slot.endTime}] = info;
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

std::vector<KhachHangInfo> Facade::getDanhSachKhachHangCuaChuSan() const {
    User* owner = Session::getInstance().getUser();
    std::vector<KhachHangInfo> customers;
    if (owner == nullptr || owner->get_account_type() != "Owner") {
        return customers;
    }
    const std::string ownerId = owner->get_user_id();
    ReportRepository repository;
    LinkedList<ReportBookingRecord> bookings;
    std::string error;
    if (!repository.loadBookings(bookings, error)) {
        std::cerr << error << '\n';
        return customers;
    }
    for (auto* node = bookings.get_head(); node != nullptr; node = node->next) {
        const ReportBookingRecord& booking = node->data;
        if (booking.ownerUserId != ownerId) {
            continue;
        }
        auto found = std::find_if(
            customers.begin(),
            customers.end(),
            [&booking](const KhachHangInfo& customer) {
                return customer.sdt == booking.customerPhone;
            }
        );
        if (found == customers.end()) {
            KhachHangInfo customer;
            customer.id = booking.customerId;
            customer.hoTen = booking.customerName;
            customer.sdt = booking.customerPhone;
            auto existing = std::find_if(
                danhSachKhachHang_.begin(),
                danhSachKhachHang_.end(),
                [&booking](const KhachHangInfo& item) {
                    return item.sdt == booking.customerPhone;
                }
            );
            if (existing != danhSachKhachHang_.end()) {
                customer.biChan = existing->biChan;
                customer.lyDoChan = existing->lyDoChan;
                customer.ngayChan = existing->ngayChan;
            }
            customers.push_back(customer);
            found = customers.end() - 1;
        }
        if (booking.status == "COMPLETED" ||
            booking.status == u8"Đã hoàn thành" ||
            booking.status == u8"Đã thanh toán") {
            ++found->soLanDat;
        } else if (booking.status == "CANCELLED" ||
                   booking.status == u8"Đã hủy") {
            ++found->soLanHuy;
        }
    }

    for (KhachHangInfo& customer : customers) {
        if (customer.biChan) {
            continue;
        }
        for (const NguongUuDai& threshold : danhSachNguongUuDai_) {
            if (customer.soLanDat >= threshold.soLanToiThieu) {
                customer.mucUuDaiPhanTram = threshold.phanTramGiam;
            }
        }
    }
    return customers;
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
    for (const auto& kh : getDanhSachKhachHangCuaChuSan()) {
        if (kh.biChan) {
            dsDen.push_back(kh);
        }
    }
    return dsDen;
}

std::vector<KhachHangInfo> Facade::getDeXuatChan(int soLanHuyToiThieu) const {
    std::vector<KhachHangInfo> deXuat;
    for (const auto& kh : getDanhSachKhachHangCuaChuSan()) {
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
    User* owner = Session::getInstance().getUser();
    std::vector<DanhGiaInfo> reviews;
    if (owner == nullptr) {
        return reviews;
    }
    for (const DanhGiaInfo& review : danhSachDanhGia_) {
        const SanBongInfo* field = nullptr;
        for (const SanBongInfo& candidate : danhSachSan_) {
            if (candidate.maSan == review.maSan) {
                field = &candidate;
                break;
            }
        }
        if (field != nullptr && field->ownerUserId == owner->get_user_id()) {
            reviews.push_back(review);
        }
    }
    return reviews;
}

float Facade::getDiemTrungBinhToanCoSo() const {
    const std::vector<DanhGiaInfo> reviews = getDanhSachDanhGia();
    if (reviews.empty()) return 0.0f;
    float sum = 0.0f;
    for (const auto& dg : reviews) {
        sum += dg.soSao;
    }
    return sum / static_cast<float>(reviews.size());
}

std::map<std::string, float> Facade::getDiemTrungBinhTungSan() const {
    std::map<std::string, std::pair<float, int>> sumCount;
    // Khởi tạo các sân hiện có với điểm mặc định
    for (const auto& s : getDanhSachSanBongCuaChuSan()) {
        sumCount[s.maSan] = {0.0f, 0};
    }

    for (const auto& dg : getDanhSachDanhGia()) {
        sumCount[dg.maSan].first += dg.soSao;
        sumCount[dg.maSan].second++;
    }

    std::map<std::string, float> result;
    for (const auto& item : sumCount) {
        if (item.second.second > 0) {
            result[item.first] = item.second.first / static_cast<float>(item.second.second);
        } else {
            result[item.first] = 0.0f;
        }
    }
    return result;
}

std::vector<DonDatSanInfo> Facade::getLichSuDatCuaKhach(const std::string& sdt) const {
    std::vector<DonDatSanInfo> lichSu;
    ReportRepository repository;
    LinkedList<ReportBookingRecord> bookings;
    std::string error;
    if (!repository.loadBookings(bookings, error)) {
        std::cerr << error << '\n';
        return lichSu;
    }
    for (auto* node = bookings.get_head(); node != nullptr; node = node->next) {
        if (node->data.customerPhone == sdt) {
            lichSu.push_back(toBookingInfo(node->data));
        }
    }
    return lichSu;
}

std::vector<DonDatSanInfo> Facade::getLichSuDatCuaKhachCuaChuSan(
    const std::string& sdt
) const {
    User* owner = Session::getInstance().getUser();
    std::vector<DonDatSanInfo> history;
    if (owner == nullptr) {
        return history;
    }
    ReportRepository repository;
    LinkedList<ReportBookingRecord> bookings;
    std::string error;
    if (!repository.loadBookings(bookings, error)) {
        std::cerr << error << '\n';
        return history;
    }
    for (auto* node = bookings.get_head(); node != nullptr; node = node->next) {
        if (node->data.customerPhone == sdt &&
            node->data.ownerUserId == owner->get_user_id()) {
            history.push_back(toBookingInfo(node->data));
        }
    }
    return history;
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

std::vector<DanhGiaInfo> Facade::getDanhGiaCuaKhachCuaChuSan(
    const std::string& sdt
) const {
    std::vector<DanhGiaInfo> reviews;
    for (const DanhGiaInfo& review : getDanhSachDanhGia()) {
        if (review.sdt == sdt) {
            reviews.push_back(review);
        }
    }
    return reviews;
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
    User* owner = Session::getInstance().getUser();
    std::vector<GiaiDauInfo> tournaments;
    if (owner == nullptr || owner->get_account_type() != "Owner") {
        return tournaments;
    }
    for (const GiaiDauInfo& tournament : danhSachGiaiDau_) {
        for (const SanBongInfo& field : danhSachSan_) {
            if (field.maSan == tournament.maSan &&
                field.ownerUserId == owner->get_user_id()) {
                tournaments.push_back(tournament);
                break;
            }
        }
    }
    return tournaments;
}

std::vector<GiaiDauInfo> Facade::getYeuCauGiaiDauChoDuyet() const {
    std::vector<GiaiDauInfo> ds;
    for (const auto& g : getDanhSachGiaiDau()) {
        if (g.trangThai == TrangThaiGiaiDau::ChoDuyet) {
            ds.push_back(g);
        }
    }
    return ds;
}

int Facade::getSoLuongYeuCauChoDuyet() const {
    return static_cast<int>(getYeuCauGiaiDauChoDuyet().size());
}

GiaiDauInfo* Facade::getGiaiDau(const std::string& maGiai) {
    User* owner = Session::getInstance().getUser();
    if (owner == nullptr || owner->get_account_type() != "Owner") return nullptr;
    for (auto& g : danhSachGiaiDau_) {
        if (g.maGiai != maGiai) continue;
        for (const SanBongInfo& field : danhSachSan_) {
            if (field.maSan == g.maSan &&
                field.ownerUserId == owner->get_user_id()) {
                return &g;
            }
        }
    }
    return nullptr;
}

KetQua Facade::duyetGiaiDau(const std::string& maGiai) {
    User* owner = Session::getInstance().getUser();
    GiaiDauInfo* target = getGiaiDau(maGiai);
    if (owner == nullptr || target == nullptr) {
        return {false, u8"Lỗi: Không tìm thấy giải đấu có mã " + maGiai};
    }
    std::string ownerError;
    if (!ownerOwnsField(owner->get_user_id(), target->maSan, ownerError)) {
        return {false, ownerError};
    }
    const std::string startDate = DateTime::toIsoDate(target->ngayBatDau);
    const std::string endDate = DateTime::toIsoDate(target->ngayKetThuc);
    if (startDate.empty() || DateTime::isPastDate(startDate)) {
        return {false, u8"Không thể duyệt giải có ngày bắt đầu đã qua hoặc không hợp lệ."};
    }
    if (endDate.empty() || DateTime::compareDate(endDate, startDate) < 0) {
        return {false, u8"Ngày kết thúc phải bằng hoặc sau ngày bắt đầu."};
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
    User* owner = Session::getInstance().getUser();
    GiaiDauInfo* target = getGiaiDau(maGiai);
    if (owner == nullptr || target == nullptr) {
        return {false, u8"Không tìm thấy giải đấu cần từ chối."};
    }
    std::string ownerError;
    if (!ownerOwnsField(owner->get_user_id(), target->maSan, ownerError)) {
        return {false, ownerError};
    }
    target->trangThai = TrangThaiGiaiDau::TuChoi;
    target->ghiChuTuChoi = lyDo.empty() ? u8"Trùng lịch sân hoặc quy mô không phù hợp." : lyDo;
    luuDuLieu();
    return {true, ""};
}

KetQua Facade::taoGiaiDauThuCong(const GiaiDauInfo& giaiMoi) {
    User* owner = Session::getInstance().getUser();
    if (owner == nullptr || owner->get_account_type() != "Owner") {
        return {false, u8"Cần đăng nhập bằng tài khoản chủ sân."};
    }
    if (giaiMoi.tenGiai.empty()) {
        return {false, u8"Tên giải đấu không được để trống!"};
    }
    const std::string startDate = DateTime::toIsoDate(giaiMoi.ngayBatDau);
    const std::string endDate = DateTime::toIsoDate(giaiMoi.ngayKetThuc);
    if (startDate.empty() || DateTime::isPastDate(startDate)) {
        return {false, u8"Ngày bắt đầu giải không hợp lệ hoặc đã qua."};
    }
    if (endDate.empty() || DateTime::compareDate(endDate, startDate) < 0) {
        return {false, u8"Ngày kết thúc phải bằng hoặc sau ngày bắt đầu."};
    }
    std::string ownerError;
    if (!ownerOwnsField(owner->get_user_id(), giaiMoi.maSan, ownerError)) {
        return {false, ownerError};
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
    luuDuLieu();
    return {true, ""};
}

KetQua Facade::suaGiaiDau(const GiaiDauInfo& giaiCapNhat) {
    User* owner = Session::getInstance().getUser();
    GiaiDauInfo* target = getGiaiDau(giaiCapNhat.maGiai);
    if (owner == nullptr || target == nullptr) {
        return {false, u8"Không tìm thấy giải đấu để cập nhật."};
    }
    std::string ownerError;
    if (!ownerOwnsField(owner->get_user_id(), giaiCapNhat.maSan, ownerError)) {
        return {false, ownerError};
    }
    const std::string startDate = DateTime::toIsoDate(giaiCapNhat.ngayBatDau);
    const std::string endDate = DateTime::toIsoDate(giaiCapNhat.ngayKetThuc);
    if (startDate.empty() || endDate.empty() ||
        DateTime::compareDate(endDate, startDate) < 0) {
        return {false, u8"Ngày kết thúc phải bằng hoặc sau ngày bắt đầu; ngày phải hợp lệ."};
    }
    const std::string previousStart = DateTime::toIsoDate(target->ngayBatDau);
    const bool startDateChanged = startDate != previousStart;
    if (startDateChanged && DateTime::isPastDate(startDate)) {
        return {false, u8"Ngày bắt đầu giải không được ở trong quá khứ."};
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
    luuDuLieu();
    return {true, ""};
}

KetQua Facade::huyGiaiDau(const std::string& maGiai) {
    GiaiDauInfo* target = getGiaiDau(maGiai);
    if (target == nullptr) {
        return {false, u8"Không tìm thấy giải đấu cần hủy."};
    }
    for (size_t i = 0; i < danhSachGiaiDau_.size(); ++i) {
        if (danhSachGiaiDau_[i].maGiai == target->maGiai) {
            danhSachGiaiDau_.erase(danhSachGiaiDau_.begin() + i);
            luuDuLieu();
            return {true, ""};
        }
    }
    return {false, u8"Không tìm thấy giải đấu cần hủy."};
}

// ==========================================
// 8. NGHIỆP VỤ DÀNH CHO NGƯỜI THUÊ SÂN
// ==========================================
bool Facade::laKhachHangBiChan(const std::string& sdt) const {
    std::string customerId = sdt;
    for (const auto& user : danhSachUser_) {
        if (user && user->get_phone_number() == sdt) {
            customerId = user->get_user_id();
            break;
        }
    }
    VenueRepository repository;
    std::string reason;
    std::string error;
    const bool blocked =
        repository.isCustomerBlacklisted(customerId, sdt, reason, error);
    if (!error.empty()) std::cerr << error << '\n';
    return blocked;
}

int Facade::getSoLanHuyCuaKhach(const std::string& sdt) const {
    int count = 0;
    for (const DonDatSanInfo& booking : getLichSuDatCuaKhach(sdt)) {
        if (booking.trangThai == "CANCELLED" ||
            booking.trangThai == u8"Đã hủy") {
            ++count;
        }
    }
    return count;
}

int Facade::getSoLanDatCuaKhach(const std::string& sdt) const {
    int count = 0;
    for (const DonDatSanInfo& booking : getLichSuDatCuaKhach(sdt)) {
        if (booking.sdt == sdt &&
            (booking.trangThai == u8"Đã hoàn thành" ||
             booking.trangThai == u8"Đã thanh toán")) {
            ++count;
        }
    }
    return count;
}

int Facade::getMucUuDaiCuaKhach(const std::string& sdt) const {
    if (laKhachHangBiChan(sdt)) {
        return 0;
    }
    const int bookings = getSoLanDatCuaKhach(sdt);
    int discount = 0;
    for (const NguongUuDai& threshold : danhSachNguongUuDai_) {
        if (bookings >= threshold.soLanToiThieu) {
            discount = threshold.phanTramGiam;
        }
    }
    return discount;
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

    SanBongInfo* field = getSanBong(maSan);
    if (field == nullptr || field->ownerUserId.empty()) {
        return {false, u8"Không tìm thấy sân thuộc chủ sân nào để nhận đặt chỗ."};
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
    donMoi.ownerUserId = field->ownerUserId;
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
    User* user = Session::getInstance().getUser();
    if (user == nullptr || user->get_account_type() != "Customer" ||
        user->get_phone_number() != sdtKhach) {
        return {false, u8"Chỉ người thuê sở hữu đơn mới được hủy."};
    }
    VenueRepository venueRepository;
    LinkedList<VenueBookingRecord> bookings;
    std::string error;
    if (!venueRepository.loadBookings(bookings, error)) return {false, error};
    std::string customerId;
    bool found = false;
    for (auto* node = bookings.get_head(); node != nullptr; node = node->next) {
        const VenueBookingRecord& booking = node->data;
        if (booking.bookingId != maDon) continue;
        if (booking.customerPhone != sdtKhach) {
            return {false, u8"Không có quyền hủy đơn đặt sân của người khác."};
        }
        customerId = booking.customerId;
        found = true;
        break;
    }
    if (!found) return {false, u8"Không tìm thấy đơn đặt sân cần hủy."};
    FileManager fileManager;
    BookingRepository bookingRepository(fileManager);
    CustomerRepository customerRepository(fileManager);
    CustomerService customerService(customerRepository);
    BookingService service(bookingRepository, customerService);
    if (!service.cancelBooking(maDon, customerId, error)) return {false, error};
    for (DonDatSanInfo& booking : danhSachDonDat_) {
        if (booking.maDon == maDon) booking.trangThai = "CANCELLED";
    }
    return {true, ""};
}

KetQua Facade::xacNhanDaHoanThanhDon(const std::string& maDon) {
    User* owner = Session::getInstance().getUser();
    if (owner == nullptr || owner->get_account_type() != "Owner") {
        return {false, u8"Chỉ chủ sân mới được xác nhận hoàn tất đơn."};
    }
    FileManager fileManager;
    BookingRepository bookingRepository(fileManager);
    CustomerRepository customerRepository(fileManager);
    CustomerService customerService(customerRepository);
    BookingService service(bookingRepository, customerService);
    std::string error;
    if (!service.completeBooking(owner->get_user_id(), maDon, error)) {
        return {false, error};
    }
    for (DonDatSanInfo& booking : danhSachDonDat_) {
        if (booking.maDon == maDon &&
            booking.ownerUserId == owner->get_user_id()) {
            booking.trangThai = "COMPLETED";
        }
    }
    return {true, ""};
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

    for (const DonDatSanInfo& don : getLichSuDatCuaKhach(sdt)) {
        if (don.trangThai == "COMPLETED" ||
            don.trangThai == u8"Đã hoàn thành" ||
            don.trangThai == u8"Đã thanh toán") {
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
