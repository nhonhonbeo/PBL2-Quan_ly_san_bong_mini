#include "GUI/App/Facade.h"

#include <algorithm>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "Core/DateTime.h"
#include "Field/Field.h"
#include "GUI/App/Session.h"
#include "GUI/Screens/Owner/OwnerShellScreen.h"
#include "GUI/Screens/Renter/RenterShellScreen.h"

namespace GUI::App {

namespace {

bool tachKhungGio(const std::string& text, std::string& start, std::string& end, int& duration) {
    const std::size_t sep = text.find(" - ");
    if (sep == std::string::npos) return false;
    start = text.substr(0, sep); end = text.substr(sep + 3);
    if (!DateTime::isValidTime(start) || !DateTime::isValidTime(end)) return false;
    duration = (std::stoi(end.substr(0, 2)) * 60 + std::stoi(end.substr(3, 2))) -
               (std::stoi(start.substr(0, 2)) * 60 + std::stoi(start.substr(3, 2)));
    return duration == 60 || duration == 90;
}

int gioSangPhut(const std::string& timeStr) {
    return (timeStr.length() == 5 && timeStr[2] == ':') ?
        std::stoi(timeStr.substr(0, 2)) * 60 + std::stoi(timeStr.substr(3, 2)) : 0;
}

std::string phutSangGio(int totalMinutes) {
    char buf[8];
    std::snprintf(buf, sizeof(buf), "%02d:%02d", (totalMinutes / 60) % 24, totalMinutes % 60);
    return std::string(buf);
}

DonDatSanInfo toBookingInfo(const ReportBookingRecord& record) {
    DonDatSanInfo b;
    b.maDon = record.bookingId; b.maSan = record.fieldId; b.ownerUserId = record.ownerUserId;
    b.customerId = record.customerId; b.tenKhachHang = record.customerName; b.sdt = record.customerPhone;
    b.ngay = record.date; b.khungGio = record.timeSlot.empty() ? (record.startTime + " - " + record.endTime) : record.timeSlot;
    b.thoiLuongPhut = record.durationMinutes; b.giaTien = static_cast<int>(record.amountVnd); b.trangThai = record.status;
    return b;
}

}  // namespace

Facade::Facade()
    : file_manager_(), account_repository_(file_manager_), venue_repository_(),
      customer_repository_(file_manager_), booking_repository_(file_manager_),
      report_repository_(), tournament_repository_(file_manager_), field_repository_(file_manager_),
      account_service_(account_repository_), venue_service_(venue_repository_),
      customer_service_(customer_repository_), booking_service_(booking_repository_, customer_service_),
      report_service_(), tournament_service_(tournament_repository_, booking_repository_),
      field_service_(field_repository_) {
    napDuLieu();
}

Facade& Facade::getInstance() {
    static Facade instance;
    return instance;
}

void Facade::dongBoTuCacService() {
    std::string err;
    account_service_.loadAccounts(err);
    danhSachUser_ = account_service_.getAccounts();
    for (const auto& u : danhSachUser_) if (u) mapMatKhau_[u->get_phone_number()] = u->get_password();

    venue_repository_.migrateLegacyData(err);
    venue_repository_.seedIfNeeded(err);
    venue_repository_.loadPrices(bangGiaCoSo_, err);

    danhSachSan_.clear();
    LinkedList<ReportFieldRecord> repFields;
    if (report_repository_.loadFields(repFields, err)) {
        for (auto* n = repFields.get_head(); n; n = n->next) {
            danhSachSan_.push_back({n->data.fieldId, n->data.ownerUserId, n->data.name,
                                    n->data.size, static_cast<int>(n->data.hourlyRateVnd),
                                    n->data.underMaintenance, n->data.rules});
        }
    }

    danhSachDonDat_.clear();
    LinkedList<ReportBookingRecord> repBookings;
    if (report_repository_.loadBookings(repBookings, err)) {
        for (auto* n = repBookings.get_head(); n; n = n->next) {
            danhSachDonDat_.push_back(toBookingInfo(n->data));
        }
    }

    danhSachNguongUuDai_.clear();
    for (const auto& t : customer_service_.getDiscountThresholds()) {
        danhSachNguongUuDai_.push_back({t.id, t.minBookings, t.percentage, t.tierName});
    }

    danhSachKhachHang_.clear();
    LinkedList<Customer> customers;
    customer_service_.getAllCustomers(customers, err);
    for (auto* n = customers.get_head(); n; n = n->next) {
        KhachHangInfo kh{n->data.getUserId(), n->data.get_name(), n->data.get_phone_number(),
                         n->data.getBookingCount(), n->data.getCancelCount(), false, "", "", 0};
        customer_service_.isBlacklisted(kh.sdt, kh.biChan, kh.lyDoChan);
        kh.mucUuDaiPhanTram = customer_service_.getCustomerDiscountPercentage(kh.sdt, err);
        mapIndexKhachSDT_.insert(kh.sdt, danhSachKhachHang_.size());
        danhSachKhachHang_.push_back(kh);
    }

    LinkedList<Blacklist> blacklists;
    customer_service_.getAllBlacklists(blacklists, err);
    for (auto* n = blacklists.get_head(); n; n = n->next) {
        size_t idx = 0;
        if (!mapIndexKhachSDT_.search(n->data.getCustomerId(), idx)) {
            KhachHangInfo bkh{n->data.getCustomerId(), n->data.getCustomerId(), n->data.getCustomerId(),
                              0, 0, n->data.isActive(), n->data.getReason(), n->data.getCreatedAt(), 0};
            mapIndexKhachSDT_.insert(bkh.sdt, danhSachKhachHang_.size());
            danhSachKhachHang_.push_back(bkh);
        }
    }

    danhSachDanhGia_.clear();
    LinkedList<Review> reviews;
    customer_service_.getAllReviews(reviews, err);
    for (auto* n = reviews.get_head(); n; n = n->next) {
        danhSachDanhGia_.push_back({n->data.getReviewId(), n->data.getCustomerName(),
                                   n->data.getCustomerId(), n->data.getFieldId(),
                                   n->data.getRating(), n->data.getCreatedAt(), n->data.getComment()});
    }

    danhSachGiaiDau_.clear();
    LinkedList<Tournament> tourList;
    tournament_service_.loadAllTournaments(tourList, err);
    for (auto* n = tourList.get_head(); n; n = n->next) {
        danhSachGiaiDau_.push_back({n->data.getTournamentId(), n->data.getName(),
                                   n->data.getRequesterName(), n->data.getRequesterUserId(),
                                   n->data.getStartDate(), n->data.getEndDate(),
                                   n->data.getRequestedFieldId(), n->data.getTimeSlot(),
                                   n->data.getNote(), n->data.getRejectReason(),
                                   static_cast<TrangThaiGiaiDau>(static_cast<int>(n->data.getStatus()))});
    }
}
void Facade::napDuLieu() { if (!daNapDuLieu_) { dongBoTuCacService(); daNapDuLieu_ = true; } }
void Facade::luuDuLieu() { std::string err; account_repository_.save(danhSachUser_, err); }

bool Facade::kiemTraSDTDaTonTai(const std::string& sdt) const {
    for (const auto& u : danhSachUser_) if (u && u->get_phone_number() == sdt) return true;
    return false;
}

KetQuaDuLieu<std::shared_ptr<User>> Facade::dangNhap(const std::string& sdt, const std::string& matKhau) {
    napDuLieu();
    std::string err;
    std::shared_ptr<User> user = account_service_.authenticate(sdt, matKhau, err);
    if (!user) {
        auto it = mapMatKhau_.find(sdt);
        if (it != mapMatKhau_.end() && it->second == matKhau) {
            for (const auto& u : danhSachUser_) if (u && u->get_phone_number() == sdt) { user = u; break; }
        }
    }
    if (!user) {
        if (!kiemTraSDTDaTonTai(sdt)) return {false, u8"Số điện thoại này chưa được đăng ký trong hệ thống.", nullptr};
        return {false, u8"Mật khẩu đăng nhập không chính xác.", nullptr};
    }
    Session::getInstance().login(user, user->get_account_type());
    return {true, "", user};
}

KetQua Facade::dangKyNguoiThue(const std::string& hoTen, const std::string& sdt, const std::string& matKhau) {
    napDuLieu();
    if (kiemTraSDTDaTonTai(sdt)) return {false, u8"Số điện thoại này đã được đăng ký tài khoản khác."};
    char idBuf[16];
    std::snprintf(idBuf, sizeof(idBuf), "US%02d", static_cast<int>(danhSachUser_.size() + 1));
    auto cust = std::make_shared<Customer>(idBuf, sdt, matKhau, hoTen, sdt);
    std::string err;
    if (!account_service_.registerUser(cust, err)) return {false, err};
    customer_repository_.saveCustomer(*cust, err);

    danhSachUser_.push_back(cust);
    mapMatKhau_[sdt] = matKhau;
    KhachHangInfo kh{idBuf, hoTen, sdt, 0, 0, false, "", "", 0};
    mapIndexKhachSDT_.insert(sdt, danhSachKhachHang_.size());
    danhSachKhachHang_.push_back(kh);
    return {true, ""};
}

KetQua Facade::dangKyChuSan(const std::string& hoTen, const std::string& sdt, const std::string& matKhau,
                            const std::string& tenCoSo, const std::string& diaChi) {
    napDuLieu();
    if (kiemTraSDTDaTonTai(sdt)) return {false, u8"Số điện thoại này đã được đăng ký tài khoản khác."};
    char idBuf[16];
    std::snprintf(idBuf, sizeof(idBuf), "US%02d", static_cast<int>(danhSachUser_.size() + 1));
    auto owner = std::make_shared<Owner>(idBuf, sdt, matKhau, hoTen, sdt, tenCoSo, diaChi,
                                         u8"Quy định tiêu chuẩn cụm sân", "06:00", "23:00", hoTen);
    std::string err;
    if (!account_service_.registerUser(owner, err)) return {false, err};
    danhSachUser_.push_back(owner);
    mapMatKhau_[sdt] = matKhau;
    return {true, ""};
}

void Facade::moManHinhChinhChoUser(User* user, ScreenManager& sm) {
    if (!user) return;
    std::string acc = user->get_account_type();
    if (dynamic_cast<Owner*>(user) || acc == "Owner" || acc == u8"Chủ sân") {
        sm.replace(std::make_unique<GUI::Screens::Owner::OwnerShellScreen>());
    } else {
        sm.replace(std::make_unique<GUI::Screens::Renter::RenterShellScreen>());
    }
}

bool Facade::getBaoCaoDashboard(ReportDashboardMetrics& metrics, LinkedList<ReportRevenueDay>& doanhThuTheoNgay,
                                LinkedList<ReportBookingRecord>& luotDatGanDay, std::string& error) const {
    User* user = Session::getInstance().getUser();
    if (!user || user->get_user_id().empty()) {
        error = u8"Không xác định được chủ sân đang đăng nhập.";
        return false;
    }
    return report_service_.buildOwnerDashboard(user->get_user_id(), report_repository_, metrics,
                                               doanhThuTheoNgay, luotDatGanDay, error);
}

bool Facade::getDanhSachCoSo(LinkedList<VenueFacilityRecord>& f, std::string& err) const { return venue_service_.getFacilities(f, err); }

bool Facade::getGiaThapNhatCoSo(const std::string& ownerId, long long& giaVnd, std::string& err) const { return venue_service_.getLowestPrice(ownerId, giaVnd, err); }

bool Facade::getDanhSachSanTheoChuSan(const std::string& ownerId, int loaiSan, bool gomSanKhongNhanDat,
                                     LinkedList<VenueFieldRecord>& fields, std::string& err) const { return venue_service_.getFields(ownerId, loaiSan, gomSanKhongNhanDat, fields, err); }

bool Facade::getKhungGioSan(const std::string& fieldId, const std::string& ngay, int thoiLuongPhut,
                           LinkedList<VenueSlotRecord>& slots, std::string& err, bool choPhepNgayQuaKhu) const { return venue_service_.getSlots(fieldId, ngay, thoiLuongPhut, slots, err, choPhepNgayQuaKhu); }

bool Facade::datSanTheoKhungGio(const VenueBookingRequest& req, std::string& bId, std::string& err) const { return booking_service_.createBooking(req, bId, err); }

bool Facade::datNhieuKhungGio(const LinkedList<VenueBookingRequest>& reqs, LinkedList<std::string>& bIds, std::string& err) const { return booking_service_.createBookings(reqs, bIds, err); }

bool Facade::capNhatGiaCoSo(int loaiSan, int thoiLuongPhut, const std::string& gioBatDau,
                           long long giaVnd, std::string& error) {
    User* user = Session::getInstance().getUser();
    if (!user || user->get_account_type() != "Owner") {
        error = "Cần đăng nhập bằng tài khoản chủ sân để sửa bảng giá.";
        return false;
    }
    if (!venue_service_.setFieldPrice(user->get_user_id(), loaiSan, thoiLuongPhut, gioBatDau, giaVnd, error)) return false;
    return venue_repository_.loadPrices(bangGiaCoSo_, error);
}
std::vector<SanBongInfo> Facade::getDanhSachSanBong() const { return danhSachSan_; }

std::vector<SanBongInfo> Facade::getDanhSachSanBongCuaChuSan() const {
    User* user = Session::getInstance().getUser();
    std::vector<SanBongInfo> fields;
    if (!user || user->get_account_type() != "Owner") return fields;

    LinkedList<VenueFacilityRecord> facilities;
    std::string error;
    bool allows60 = false, allows90 = false;
    if (venue_service_.getFacilities(facilities, error)) {
        for (auto* n = facilities.get_head(); n; n = n->next) {
            if (n->data.ownerId == user->get_user_id()) {
                allows60 = n->data.allows60Minutes;
                allows90 = n->data.allows90Minutes;
                break;
            }
        }
    }
    for (const auto& f : danhSachSan_) {
        if (f.ownerUserId == user->get_user_id()) {
            SanBongInfo vf = f;
            vf.giaThueGio = 0;
            for (auto* n = bangGiaCoSo_.get_head(); n != nullptr; n = n->next) {
                const auto& p = n->data;
                if (p.ownerId == user->get_user_id() && p.fieldSize == f.loaiSan &&
                    p.durationMinutes > 0 && p.priceVnd > 0 &&
                    ((p.durationMinutes == 60 && allows60) || (p.durationMinutes == 90 && allows90))) {
                    long long hp = p.priceVnd * 60 / p.durationMinutes;
                    if (vf.giaThueGio == 0 || hp < vf.giaThueGio) vf.giaThueGio = static_cast<int>(hp);
                }
            }
            fields.push_back(vf);
        }
    }
    return fields;
}

SanBongInfo* Facade::getSanBong(const std::string& maSan) {
    User* owner = Session::getInstance().getUser();
    if (!owner || owner->get_account_type() != "Owner") return nullptr;
    for (auto& s : danhSachSan_) {
        if (s.maSan == maSan && s.ownerUserId == owner->get_user_id()) return &s;
    }
    return nullptr;
}

KetQua Facade::taoNhieuSan(int loaiSan, int soLuong, const std::string& quyDinh,
                           std::vector<std::string>& outMaSan) {
    outMaSan.clear();
    User* owner = Session::getInstance().getUser();
    if (!owner || owner->get_account_type() != "Owner") return {false, u8"Vui lòng đăng nhập bằng tài khoản chủ sân trước khi tạo sân."};
    if (loaiSan != 5 && loaiSan != 7 && loaiSan != 11) return {false, u8"Loại sân không hợp lệ."};
    if (soLuong <= 0) return {false, u8"Số lượng sân cần tạo phải lớn hơn 0."};

    LinkedList<VenueFieldRecord> fields;
    std::string error;
    if (!venue_repository_.loadFields(fields, error)) return {false, error};

    int seq = 1;
    const std::string prefix = "S" + std::to_string(loaiSan) + "-";
    for (auto* n = fields.get_head(); n; n = n->next) {
        if (n->data.fieldId.rfind(prefix, 0) == 0) {
            try {
                int ex = std::stoi(n->data.fieldId.substr(prefix.size()));
                if (ex >= seq) seq = ex + 1;
            } catch (...) {}
        }
    }

    for (int i = 0; i < soLuong; ++i) {
        int stt = seq + i;
        char maBuf[16];
        std::snprintf(maBuf, sizeof(maBuf), "S%d-%02d", loaiSan, stt);
        VenueFieldRecord newF{maBuf, owner->get_user_id(),
                              u8"Sân " + std::to_string(loaiSan) + u8" số " +
                              (stt < 10 ? "0" : "") + std::to_string(stt),
                              loaiSan, 0, quyDinh};
        if (!field_service_.createField(owner->get_user_id(), newF, error)) return {false, error};
        danhSachSan_.push_back({newF.fieldId, newF.ownerId, newF.name, newF.size, 0, false, newF.rules});
        outMaSan.push_back(maBuf);
    }
    venue_repository_.loadPrices(bangGiaCoSo_, error);
    return {true, ""};
}

KetQua Facade::suaSanBong(const std::string& maSan, const std::string& tenSan, int loaiSan, int giaThue) {
    (void)giaThue;
    auto* san = getSanBong(maSan);
    if (!san) return {false, u8"Không tìm thấy sân bóng với mã: " + maSan};
    User* owner = Session::getInstance().getUser();
    if (!owner) return {false, u8"Vui lòng đăng nhập lại."};

    VenueFieldRecord updated{maSan, owner->get_user_id(), tenSan, loaiSan, san->dangBaoTri ? 1 : 0, san->quyDinh};
    std::string error;
    if (!venue_service_.updateField(owner->get_user_id(), updated, error)) return {false, error};
    san->tenSan = tenSan; san->loaiSan = loaiSan;
    return {true, ""};
}

KetQua Facade::toggleBaoTriSan(const std::string& maSan) {
    auto* san = getSanBong(maSan);
    if (!san) return {false, u8"Không tìm thấy sân bóng với mã: " + maSan};
    User* owner = Session::getInstance().getUser();
    if (!owner) return {false, u8"Vui lòng đăng nhập lại."};

    FieldStatus status = san->dangBaoTri ? FieldStatus::ACTIVE : FieldStatus::MAINTENANCE;
    std::string error;
    if (!field_service_.setFieldStatus(owner->get_user_id(), maSan, status, error)) return {false, error};
    san->dangBaoTri = !san->dangBaoTri;
    return {true, ""};
}

KetQua Facade::xoaSanBong(const std::string& maSan) {
    User* owner = Session::getInstance().getUser();
    if (!owner || owner->get_account_type() != "Owner") return {false, u8"Vui lòng đăng nhập bằng tài khoản chủ sân."};
    std::string error;
    if (!venue_service_.deleteField(owner->get_user_id(), maSan, error)) return {false, error};
    auto it = std::remove_if(danhSachSan_.begin(), danhSachSan_.end(),
                             [&](const SanBongInfo& s) { return s.maSan == maSan && s.ownerUserId == owner->get_user_id(); });
    if (it != danhSachSan_.end()) { danhSachSan_.erase(it, danhSachSan_.end()); return {true, ""}; }
    return {false, u8"Không tìm thấy sân bóng cần xóa."};
}

std::vector<KhungGioInfo> Facade::getDanhSachKhungGio() const {
    std::vector<KhungGioInfo> slots;
    User* owner = Session::getInstance().getUser();
    if (!owner || owner->get_account_type() != "Owner") return slots;

    LinkedList<VenueFacilityRecord> facilities;
    std::string error;
    if (!venue_service_.getFacilities(facilities, error)) return slots;

    std::string openTime, closeTime;
    int dur = 0;
    for (auto* n = facilities.get_head(); n; n = n->next) {
        if (n->data.ownerId == owner->get_user_id()) {
            openTime = n->data.openTime;
            closeTime = n->data.closeTime;
            dur = n->data.allows90Minutes ? 90 : (n->data.allows60Minutes ? 60 : 0);
            break;
        }
    }
    if (dur == 0) return slots;
    const int op = gioSangPhut(openTime), cl = gioSangPhut(closeTime);
    std::map<std::string, bool> starts;
    for (auto* n = bangGiaCoSo_.get_head(); n; n = n->next) {
        if (n->data.ownerId != owner->get_user_id() || n->data.durationMinutes != dur) continue;
        int s = gioSangPhut(n->data.startTime);
        if (s >= op && s + dur <= cl) starts[n->data.startTime] = true;
    }
    int idx = 1;
    for (const auto& item : starts) {
        int s = gioSangPhut(item.first), e = s + dur;
        char endText[6], id[16];
        std::snprintf(endText, sizeof(endText), "%02d:%02d", e / 60, e % 60);
        std::snprintf(id, sizeof(id), "KG%02d", idx++);
        slots.push_back({id, item.first, endText, s >= 17 * 60 && s < 21 * 60});
    }
    return slots;
}

KetQua Facade::sinhKhungGioTuDong(const std::string& gioMo, const std::string& gioDong, int buocPhut) {
    int startM = gioSangPhut(gioMo), endM = gioSangPhut(gioDong);
    User* owner = Session::getInstance().getUser();
    if (!owner || owner->get_account_type() != "Owner") return {false, u8"Cần đăng nhập bằng tài khoản chủ sân để sửa giờ hoạt động."};
    if (!DateTime::isValidTime(gioMo) || !DateTime::isValidTime(gioDong) ||
        endM <= startM || (buocPhut != 60 && buocPhut != 90) || endM - startM < buocPhut) {
        return {false, u8"Giờ hoạt động hoặc thời lượng ca không hợp lệ."};
    }
    std::string error;
    if (!venue_service_.updateFacilityHours(owner->get_user_id(), gioMo, gioDong,
                                           buocPhut == 60, buocPhut == 90, error)) {
        return {false, error};
    }
    for (const auto& acc : danhSachUser_) {
        if (acc && acc->get_user_id() == owner->get_user_id()) {
            if (auto own = std::dynamic_pointer_cast<Owner>(acc)) {
                own->set_open_time(gioMo); own->set_close_time(gioDong);
                own->setAllowedSlotDurations(buocPhut == 60, buocPhut == 90);
            }
            break;
        }
    }
    danhSachKhungGio_.clear();
    int cur = startM, stt = 1;
    while (cur + buocPhut <= endM) {
        char idBuf[16];
        std::snprintf(idBuf, sizeof(idBuf), "KG%02d", stt++);
        danhSachKhungGio_.push_back({idBuf, phutSangGio(cur), phutSangGio(cur + buocPhut),
                                    (cur >= 17 * 60 + 30 && cur < 20 * 60 + 30)});
        cur += buocPhut;
    }
    venue_repository_.loadPrices(bangGiaCoSo_, error);
    return {true, ""};
}

int Facade::getGiaMaTran(const std::string& khungGioStr, int loaiSan) const {
    User* owner = Session::getInstance().getUser();
    if (!owner || owner->get_account_type() != "Owner") return 0;
    std::string start, end;
    int duration = 0;
    if (!tachKhungGio(khungGioStr, start, end, duration)) return 0;
    for (auto* n = bangGiaCoSo_.get_head(); n; n = n->next) {
        const auto& p = n->data;
        if (p.ownerId == owner->get_user_id() && p.fieldSize == loaiSan &&
            p.durationMinutes == duration && p.startTime == start) {
            return static_cast<int>(p.priceVnd);
        }
    }
    return 0;
}

KetQua Facade::capNhatGiaMaTran(const std::string& khungGioStr, int loaiSan, int giaMoi, bool apDungTatCaCungLoai) {
    std::string start, end;
    int duration = 0;
    if (!tachKhungGio(khungGioStr, start, end, duration)) return {false, u8"Khung giờ hoặc thời lượng không hợp lệ."};
    User* owner = Session::getInstance().getUser();
    if (!owner || owner->get_account_type() != "Owner") return {false, u8"Cần đăng nhập bằng tài khoản chủ sân để sửa giá."};
    std::string error;
    if (!venue_service_.setFieldPrice(owner->get_user_id(), loaiSan, duration, start, giaMoi, error, apDungTatCaCungLoai)) {
        return {false, error};
    }
    venue_repository_.loadPrices(bangGiaCoSo_, error);
    return {true, ""};
}

std::map<std::pair<std::string, std::string>, Widgets::SlotInfo> Facade::getTinhTrangSlotTheoNgay(const std::string& ngay) {
    std::map<std::pair<std::string, std::string>, Widgets::SlotInfo> maTran;
    User* owner = Session::getInstance().getUser();
    if (!owner || owner->get_account_type() != "Owner") return maTran;

    LinkedList<VenueFieldRecord> fields;
    std::string error;
    if (!venue_service_.getFields(owner->get_user_id(), 0, true, fields, error)) return maTran;

    LinkedList<VenueFacilityRecord> facilities;
    if (!venue_service_.getFacilities(facilities, error)) return maTran;

    int dur = 0;
    for (auto* n = facilities.get_head(); n; n = n->next) {
        if (n->data.ownerId == owner->get_user_id()) {
            dur = n->data.allows90Minutes ? 90 : (n->data.allows60Minutes ? 60 : 0);
            break;
        }
    }
    if (dur == 0) return maTran;

    for (auto* fn = fields.get_head(); fn; fn = fn->next) {
        LinkedList<VenueSlotRecord> slots;
        if (!venue_service_.getSlots(fn->data.fieldId, ngay, dur, slots, error, true)) continue;
        for (auto* sn = slots.get_head(); sn; sn = sn->next) {
            std::string slotKey = sn->data.startTime + " - " + sn->data.endTime;
            Widgets::SlotInfo info;
            if (fn->data.status != 0) {
                info.status = Widgets::SlotStatus::BaoTri;
                info.ghiChu = fn->data.rules;
            } else {
                info.status = !sn->data.bookingId.empty() ? Widgets::SlotStatus::DaDat
                            : (sn->data.past ? Widgets::SlotStatus::DaQua : Widgets::SlotStatus::Trong);
                info.giaTien = static_cast<int>(sn->data.priceVnd);
                info.ghiChu = sn->data.bookingId;
            }
            maTran[{fn->data.fieldId, slotKey}] = info;
        }
    }
    return maTran;
}

std::vector<KhachHangInfo> Facade::getDanhSachKhachHang() const { return danhSachKhachHang_; }

std::vector<KhachHangInfo> Facade::getDanhSachKhachHangCuaChuSan() const {
    User* owner = Session::getInstance().getUser();
    if (!owner) return danhSachKhachHang_;
    std::unordered_map<std::string, bool> booked;
    for (const auto& b : danhSachDonDat_) if (b.ownerUserId == owner->get_user_id()) booked[b.sdt] = true;
    std::vector<KhachHangInfo> list;
    for (const auto& kh : danhSachKhachHang_) if (booked[kh.sdt] || kh.biChan) list.push_back(kh);
    return list;
}

KhachHangInfo* Facade::timKhachHangTheoSDT(const std::string& sdt) {
    // LÝ DO CHỌN: HashTable tra cứu chỉ mục khách hàng theo SĐT trong O(1).
    size_t idx = 0;
    if (mapIndexKhachSDT_.search(sdt, idx) && idx < danhSachKhachHang_.size()) return &danhSachKhachHang_[idx];
    return nullptr;
}

std::vector<KhachHangInfo> Facade::getDanhSachDen() const {
    std::vector<KhachHangInfo> ds;
    for (const auto& kh : danhSachKhachHang_) if (kh.biChan) ds.push_back(kh);
    return ds;
}

std::vector<KhachHangInfo> Facade::getDeXuatChan(int soLanHuyToiThieu) const {
    std::vector<KhachHangInfo> deXuat;
    LinkedList<Customer> candidates;
    std::string error;
    customer_service_.getBlacklistCandidates(soLanHuyToiThieu, candidates, error);
    for (auto* n = candidates.get_head(); n; n = n->next) {
        deXuat.push_back({n->data.getUserId(), n->data.get_name(), n->data.get_phone_number(),
                          n->data.getBookingCount(), n->data.getCancelCount(), false, "", "", 0});
    }
    return deXuat;
}

KetQua Facade::themVaoDanhSachDen(const std::string& sdt, const std::string& lyDo) {
    std::string error;
    if (!customer_service_.addToBlacklist(sdt, lyDo, error)) return {false, error};
    auto* kh = timKhachHangTheoSDT(sdt);
    if (kh) {
        kh->biChan = true;
        kh->lyDoChan = lyDo.empty() ? u8"Hủy sân nhiều lần không thông báo trước" : lyDo;
        kh->ngayChan = DateTime::today();
        kh->mucUuDaiPhanTram = 0;
    }
    return {true, ""};
}

KetQua Facade::goKhoiDanhSachDen(const std::string& sdt) {
    std::string error;
    if (!customer_service_.removeFromBlacklist(sdt, error)) return {false, error};
    auto* kh = timKhachHangTheoSDT(sdt);
    if (kh) { kh->biChan = false; kh->lyDoChan.clear(); kh->ngayChan.clear(); }
    return {true, ""};
}

std::vector<NguongUuDai> Facade::getNguongUuDai() const { return danhSachNguongUuDai_; }

KetQua Facade::capNhatNguongUuDai(int id, int soLanToiThieu, int phanTramGiam) {
    std::string error;
    if (!customer_service_.updateDiscountThreshold(id, soLanToiThieu, phanTramGiam, error)) return {false, error};
    for (auto& n : danhSachNguongUuDai_) {
        if (n.id == id) { n.soLanToiThieu = soLanToiThieu; n.phanTramGiam = phanTramGiam; break; }
    }
    return {true, ""};
}

KetQua Facade::apDungUuDaiChoKhach() {
    std::string error;
    customer_service_.applyDiscountsToAll(error);
    for (auto& kh : danhSachKhachHang_) {
        kh.mucUuDaiPhanTram = customer_service_.getCustomerDiscountPercentage(kh.sdt, error);
    }
    return {true, ""};
}
std::vector<DanhGiaInfo> Facade::getDanhSachDanhGia() const {
    User* owner = Session::getInstance().getUser();
    std::vector<DanhGiaInfo> reviews;
    if (!owner) return reviews;
    for (const auto& r : danhSachDanhGia_) {
        for (const auto& f : danhSachSan_) {
            if (f.maSan == r.maSan && f.ownerUserId == owner->get_user_id()) { reviews.push_back(r); break; }
        }
    }
    return reviews;
}

float Facade::getDiemTrungBinhToanCoSo() const {
    float score = 0.0f; std::string err;
    return customer_service_.getAverageRatingOverall(score, err) ? score : 0.0f;
}

std::map<std::string, float> Facade::getDiemTrungBinhTungSan() const {
    std::map<std::string, float> result, ratings;
    for (const auto& s : getDanhSachSanBongCuaChuSan()) result[s.maSan] = 0.0f;
    std::string err;
    if (customer_service_.getAverageRatingsMap(ratings, err)) {
        for (auto& item : result) {
            auto it = ratings.find(item.first);
            if (it != ratings.end()) item.second = it->second;
        }
    }
    return result;
}

std::vector<DonDatSanInfo> Facade::getLichSuDatCuaKhach(const std::string& sdt) const {
    std::vector<DonDatSanInfo> lichSu;
    for (const auto& b : danhSachDonDat_) if (b.sdt == sdt) lichSu.push_back(b);
    return lichSu;
}

std::vector<DonDatSanInfo> Facade::getLichSuDatCuaKhachCuaChuSan(const std::string& sdt) const {
    User* owner = Session::getInstance().getUser();
    std::vector<DonDatSanInfo> history;
    if (!owner) return history;
    for (const auto& b : getLichSuDatCuaKhach(sdt)) if (b.ownerUserId == owner->get_user_id()) history.push_back(b);
    return history;
}

std::vector<DanhGiaInfo> Facade::getDanhGiaCuaKhach(const std::string& sdt) const {
    std::vector<DanhGiaInfo> list;
    for (const auto& r : danhSachDanhGia_) if (r.sdt == sdt) list.push_back(r);
    return list;
}

std::vector<DanhGiaInfo> Facade::getDanhGiaCuaKhachCuaChuSan(const std::string& sdt) const {
    User* owner = Session::getInstance().getUser();
    std::vector<DanhGiaInfo> list;
    if (!owner) return list;
    for (const auto& r : getDanhGiaCuaKhach(sdt)) {
        for (const auto& s : danhSachSan_) {
            if (s.maSan == r.maSan && s.ownerUserId == owner->get_user_id()) { list.push_back(r); break; }
        }
    }
    return list;
}

std::vector<GiaiDauInfo> Facade::getDanhSachGiaiDau() const {
    User* owner = Session::getInstance().getUser();
    std::vector<GiaiDauInfo> list;
    if (!owner || owner->get_account_type() != "Owner") return list;
    for (const auto& g : danhSachGiaiDau_) {
        for (const auto& f : danhSachSan_) {
            if (f.maSan == g.maSan && f.ownerUserId == owner->get_user_id()) { list.push_back(g); break; }
        }
    }
    return list;
}

std::vector<GiaiDauInfo> Facade::getYeuCauGiaiDauChoDuyet() const {
    std::vector<GiaiDauInfo> ds;
    for (const auto& g : getDanhSachGiaiDau()) if (g.trangThai == TrangThaiGiaiDau::ChoDuyet) ds.push_back(g);
    return ds;
}

int Facade::getSoLuongYeuCauChoDuyet() const { return static_cast<int>(getYeuCauGiaiDauChoDuyet().size()); }

GiaiDauInfo* Facade::getGiaiDau(const std::string& maGiai) {
    User* owner = Session::getInstance().getUser();
    if (!owner || owner->get_account_type() != "Owner") return nullptr;
    for (auto& g : danhSachGiaiDau_) {
        if (g.maGiai != maGiai) continue;
        for (const auto& f : danhSachSan_) {
            if (f.maSan == g.maSan && f.ownerUserId == owner->get_user_id()) return &g;
        }
    }
    return nullptr;
}

KetQua Facade::duyetGiaiDau(const std::string& maGiai) {
    std::string error;
    if (!tournament_service_.duyetGiaiDau(maGiai, error)) return {false, error};
    for (auto& g : danhSachGiaiDau_) {
        if (g.maGiai == maGiai) { g.trangThai = TrangThaiGiaiDau::DaDuyet; break; }
    }
    return {true, ""};
}

KetQua Facade::tuChoiGiaiDau(const std::string& maGiai, const std::string& lyDo) {
    std::string error;
    if (!tournament_service_.tuChoiGiaiDau(maGiai, lyDo, error)) return {false, error};
    for (auto& g : danhSachGiaiDau_) {
        if (g.maGiai == maGiai) {
            g.trangThai = TrangThaiGiaiDau::TuChoi;
            g.ghiChuTuChoi = lyDo.empty() ? u8"Trùng lịch sân hoặc quy mô không phù hợp." : lyDo;
            break;
        }
    }
    return {true, ""};
}

KetQua Facade::taoGiaiDauThuCong(const GiaiDauInfo& giaiMoi) {
    Tournament t(giaiMoi.maGiai, giaiMoi.sdtKhach, giaiMoi.tenGiai, giaiMoi.maSan,
                 giaiMoi.ngayBatDau, giaiMoi.ngayKetThuc, giaiMoi.moTa,
                 TournamentStatus::CONFIRMED, "", giaiMoi.tenKhachDat,
                 giaiMoi.khungGio, giaiMoi.ghiChuTuChoi);
    std::string outId, error;
    if (!tournament_service_.taoGiaiDauThuCong(t, outId, error)) return {false, error};
    GiaiDauInfo g = giaiMoi;
    g.maGiai = outId;
    g.trangThai = TrangThaiGiaiDau::DaDuyet;
    danhSachGiaiDau_.push_back(g);
    return {true, ""};
}

KetQua Facade::suaGiaiDau(const GiaiDauInfo& giaiCapNhat) {
    Tournament t(giaiCapNhat.maGiai, giaiCapNhat.sdtKhach, giaiCapNhat.tenGiai, giaiCapNhat.maSan,
                 giaiCapNhat.ngayBatDau, giaiCapNhat.ngayKetThuc, giaiCapNhat.moTa,
                 static_cast<TournamentStatus>(static_cast<int>(giaiCapNhat.trangThai)),
                 "", giaiCapNhat.tenKhachDat, giaiCapNhat.khungGio, giaiCapNhat.ghiChuTuChoi);
    std::string error;
    if (!tournament_service_.suaGiaiDau(t, error)) return {false, error};
    for (auto& g : danhSachGiaiDau_) {
        if (g.maGiai == giaiCapNhat.maGiai) { g = giaiCapNhat; break; }
    }
    return {true, ""};
}

KetQua Facade::huyGiaiDau(const std::string& maGiai) {
    std::string error;
    if (!tournament_service_.huyGiaiDau(maGiai, error)) return {false, error};
    auto it = std::remove_if(danhSachGiaiDau_.begin(), danhSachGiaiDau_.end(),
                             [&](const GiaiDauInfo& g) { return g.maGiai == maGiai; });
    if (it != danhSachGiaiDau_.end()) danhSachGiaiDau_.erase(it, danhSachGiaiDau_.end());
    return {true, ""};
}

bool Facade::laKhachHangBiChan(const std::string& sdt) const {
    bool b = false; std::string r; customer_service_.isBlacklisted(sdt, b, r); return b;
}

int Facade::getSoLanHuyCuaKhach(const std::string& sdt) const {
    Customer c("", "", "", "", ""); std::string err;
    return customer_service_.findCustomer(sdt, c, err) ? c.getCancelCount() : 0;
}

int Facade::getSoLanDatCuaKhach(const std::string& sdt) const {
    Customer c("", "", "", "", ""); std::string err;
    return customer_service_.findCustomer(sdt, c, err) ? c.getBookingCount() : 0;
}

int Facade::getMucUuDaiCuaKhach(const std::string& sdt) const {
    std::string err; return customer_service_.getCustomerDiscountPercentage(sdt, err);
}

KetQua Facade::datNhieuCaMoi(const std::string&, const std::string&, const std::string&, const std::string&,
                             const LinkedList<std::string>&, int, std::string&) { return {true, ""}; }

KetQua Facade::huyDonDat(const std::string& maDon, const std::string& sdtKhach) {
    User* user = Session::getInstance().getUser();
    if (!user || user->get_account_type() != "Customer" || user->get_phone_number() != sdtKhach) {
        return {false, u8"Chỉ người thuê sở hữu đơn mới được hủy."};
    }
    std::string customerId = user->get_user_id();
    std::string error;
    if (!booking_service_.cancelBooking(maDon, customerId, error)) return {false, error};
    for (auto& b : danhSachDonDat_) if (b.maDon == maDon) b.trangThai = "CANCELLED";
    return {true, ""};
}

KetQua Facade::xacNhanDaHoanThanhDon(const std::string& maDon) {
    User* owner = Session::getInstance().getUser();
    if (!owner || owner->get_account_type() != "Owner") return {false, u8"Chỉ chủ sân mới được xác nhận hoàn tất đơn."};
    std::string error;
    if (!booking_service_.completeBooking(owner->get_user_id(), maDon, error)) return {false, error};
    for (auto& b : danhSachDonDat_) {
        if (b.maDon == maDon && b.ownerUserId == owner->get_user_id()) b.trangThai = "COMPLETED";
    }
    return {true, ""};
}

std::vector<GiaiDauInfo> Facade::getGiaiDauCuaUser(const std::string& sdt) const {
    std::vector<GiaiDauInfo> ds;
    for (const auto& g : danhSachGiaiDau_) if (g.sdtKhach == sdt) ds.push_back(g);
    return ds;
}

KetQua Facade::guiYeuCauTaoGiai(const GiaiDauInfo& giaiMoi) {
    if (laKhachHangBiChan(giaiMoi.sdtKhach)) return {false, u8"Tài khoản đang bị khóa nên không thể gửi yêu cầu tạo giải."};
    Tournament t(giaiMoi.maGiai, giaiMoi.sdtKhach, giaiMoi.tenGiai, giaiMoi.maSan,
                 giaiMoi.ngayBatDau, giaiMoi.ngayKetThuc, giaiMoi.moTa,
                 TournamentStatus::REQUESTED, "", giaiMoi.tenKhachDat,
                 giaiMoi.khungGio, giaiMoi.ghiChuTuChoi);
    std::string outId, error;
    if (!tournament_service_.guiYeuCauTaoGiai(t, outId, error)) return {false, error};
    GiaiDauInfo g = giaiMoi;
    g.maGiai = outId;
    g.trangThai = TrangThaiGiaiDau::ChoDuyet;
    danhSachGiaiDau_.push_back(g);
    return {true, ""};
}

std::vector<SanBongInfo> Facade::getDanhSachSanDaThueXong(const std::string& sdt) const {
    std::vector<SanBongInfo> ds;
    std::unordered_map<std::string, bool> daThem;
    for (const auto& d : getLichSuDatCuaKhach(sdt)) {
        if ((d.trangThai == "COMPLETED" || d.trangThai == u8"Đã hoàn thành" || d.trangThai == u8"Đã thanh toán") && !daThem[d.maSan]) {
            for (const auto& s : danhSachSan_) {
                if (s.maSan == d.maSan) { ds.push_back(s); daThem[d.maSan] = true; break; }
            }
        }
    }
    return ds;
}

KetQua Facade::guiHoacSuaDanhGia(const std::string& sdt, const std::string& tenKhach,
                                const std::string& maSan, int soSao, const std::string& noiDung,
                                const std::string& danhGiaId) {
    std::string rId = danhGiaId;
    if (rId.empty()) {
        char buf[32];
        std::snprintf(buf, sizeof(buf), "RV%02d", static_cast<int>(danhSachDanhGia_.size() + 1));
        rId = buf;
    }
    Review r(rId, sdt, maSan, "", soSao, DateTime::today(), noiDung, tenKhach);
    std::string error;
    if (!customer_service_.addOrUpdateReview(r, error)) return {false, error};

    bool found = false;
    for (auto& dg : danhSachDanhGia_) {
        if (dg.id == rId) { dg.soSao = soSao; dg.noiDung = noiDung; found = true; break; }
    }
    if (!found) {
        danhSachDanhGia_.insert(danhSachDanhGia_.begin(), {rId, tenKhach, sdt, maSan, soSao, DateTime::today(), noiDung});
    }
    return {true, ""};
}

KetQua Facade::capNhatThongTinUser(const std::string& sdt, const std::string& hoTenMoi) { return capNhatThongTinUser(sdt, hoTenMoi, sdt); }

KetQua Facade::capNhatThongTinUser(const std::string& sdtCu, const std::string& hoTenMoi, const std::string& sdtMoi) {
    std::string error;
    if (!account_service_.updateUser(sdtCu, hoTenMoi, sdtMoi, error)) return {false, error};
    danhSachUser_ = account_service_.getAccounts();

    size_t idx = 0;
    if (mapIndexKhachSDT_.search(sdtCu, idx) && idx < danhSachKhachHang_.size()) {
        danhSachKhachHang_[idx].hoTen = hoTenMoi;
        danhSachKhachHang_[idx].sdt = sdtMoi;
        mapIndexKhachSDT_.remove(sdtCu);
        mapIndexKhachSDT_.insert(sdtMoi, idx);
    }
    if (sdtCu != sdtMoi) {
        auto it = mapMatKhau_.find(sdtCu);
        if (it != mapMatKhau_.end()) {
            mapMatKhau_[sdtMoi] = it->second;
            mapMatKhau_.erase(it);
        }
    }
    return {true, ""};
}

KetQua Facade::doiMatKhauUser(const std::string& sdt, const std::string& matKhauCu, const std::string& matKhauMoi) {
    auto it = mapMatKhau_.find(sdt);
    std::string curPw = (it != mapMatKhau_.end()) ? it->second : "123456";
    if (curPw != matKhauCu) return {false, u8"Mật khẩu hiện tại không chính xác!"};

    std::string error;
    if (!account_service_.resetPassword(sdt, matKhauMoi, error)) return {false, error};
    mapMatKhau_[sdt] = matKhauMoi;
    danhSachUser_ = account_service_.getAccounts();
    return {true, ""};
}

}  // namespace GUI::App
