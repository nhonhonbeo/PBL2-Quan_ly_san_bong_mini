#include "GUI/App/Facade.h"

#include <algorithm>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <sstream>
#include "Core/DateTime.h"
#include "Field/Field.h"
#include "GUI/App/Session.h"
#include "GUI/App/FacadeMappers.h"

namespace GUI::App {

using namespace detail;

bool Facade::getBaoCaoDashboard(ReportDashboardMetrics& metrics, LinkedList<ReportRevenueDay>& doanhThuTheoNgay,
                                LinkedList<ReportBookingRecord>& luotDatGanDay, std::string& error) const {
    User* user = Session::getInstance().getUser();
    if (!user || user->get_user_id().empty()) {
        error = u8"Không xác định được chủ sân đang đăng nhập.";
        return false;
    }
    return report_service_.buildOwnerDashboard(user->get_user_id(), metrics,
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
    refreshCache();
    return true;
}
std::vector<SanBongInfo> Facade::getDanhSachSanBong() const { return danhSachSan_; }

std::vector<SanBongInfo> Facade::getDanhSachSanBongCuaChuSan() const {
    User* user = Session::getInstance().getUser();
    std::vector<SanBongInfo> fields;
    if (!user || user->get_account_type() != "Owner") return fields;
    for (const auto& f : danhSachSan_) if (f.ownerUserId == user->get_user_id()) fields.push_back(f);
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
    LinkedList<std::string> ids;
    std::string error;
    if (!field_service_.createMultipleFields(owner->get_user_id(), loaiSan, soLuong, quyDinh, ids, error)) {
        return {false, error};
    }
    for (auto* n = ids.get_head(); n; n = n->next) outMaSan.push_back(n->data);
    refreshCache();
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
    if (!field_service_.updateField(owner->get_user_id(), updated, error)) return {false, error};
    refreshCache();
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
    refreshCache();
    return {true, ""};
}

KetQua Facade::xoaSanBong(const std::string& maSan) {
    User* owner = Session::getInstance().getUser();
    if (!owner || owner->get_account_type() != "Owner") return {false, u8"Vui lòng đăng nhập bằng tài khoản chủ sân."};
    std::string error;
    if (!field_service_.deleteField(owner->get_user_id(), maSan, error)) return {false, error};
    refreshCache();
    return {true, ""};
}

bool Facade::loadAnhSan(std::string& error) {
    LinkedList<VenueFieldImageRecord> records;
    if (!venue_service_.loadFieldImages(records, error)) return false;
    std::map<std::string, std::string> loadedImages;
    for (auto* node = records.get_head(); node != nullptr; node = node->next) {
        loadedImages[node->data.fieldId] = node->data.path;
    }
    if (records.isEmpty()) {
        loadedImages["S5-01"] = "assets/images/san_co_fifa_vip.jpg";
        loadedImages["S5-04"] = "assets/images/san_co_fifa_vip.jpg";
        loadedImages["S7-01"] = "assets/images/san_den_led_night.jpg";
        loadedImages["S11-01"] = "assets/images/san_ngoai_troi_view.jpg";
    }
    mapAnhSan_.swap(loadedImages);
    return true;
}

bool Facade::saveAnhSan(std::string& error) {
    LinkedList<VenueFieldImageRecord> records;
    for (const auto& pair : mapAnhSan_) {
        records.push_back({pair.first, pair.second});
    }
    return venue_service_.saveFieldImages(records, error);
}

KetQua Facade::uploadAnhSan(const std::string& maSan, const std::string& duongDanAnh) {
    User* owner = Session::getInstance().getUser();
    if (!owner || owner->get_account_type() != "Owner") {
        return {false, u8"Vui lòng đăng nhập bằng tài khoản chủ sân để tải lên ảnh."};
    }
    auto* san = getSanBong(maSan);
    if (!san) {
        return {false, u8"Không tìm thấy sân bóng hoặc bạn không có quyền sửa sân này."};
    }
    std::map<std::string, std::string> updatedImages = mapAnhSan_;
    updatedImages[maSan] = duongDanAnh;
    LinkedList<VenueFieldImageRecord> records;
    for (const auto& image : updatedImages) {
        records.push_back({image.first, image.second});
    }
    std::string error;
    if (!venue_service_.saveFieldImages(records, error)) {
        return {false, error};
    }
    mapAnhSan_ = std::move(updatedImages);
    san->hinhAnh = duongDanAnh;
    return {true, ""};
}

std::string Facade::getAnhSan(const std::string& maSan) const {
    auto it = mapAnhSan_.find(maSan);
    if (it != mapAnhSan_.end()) return it->second;
    return "";
}

std::vector<KhungGioInfo> Facade::getDanhSachKhungGio() const {
    std::vector<KhungGioInfo> slots;
    User* owner = Session::getInstance().getUser();
    if (!owner || owner->get_account_type() != "Owner") return slots;
    LinkedList<VenueSlotRecord> vSlots;
    std::string error;
    if (!venue_service_.getFacilitySlots(owner->get_user_id(), vSlots, error)) return slots;
    int idx = 1;
    for (auto* n = vSlots.get_head(); n; n = n->next) {
        char idBuf[16];
        std::snprintf(idBuf, sizeof(idBuf), "KG%02d", idx++);
        const int s = gioSangPhut(n->data.startTime);
        slots.push_back({idBuf, n->data.startTime, n->data.endTime, s >= 17 * 60 && s < 21 * 60});
    }
    return slots;
}

KetQua Facade::sinhKhungGioTuDong(const std::string& gioMo, const std::string& gioDong, int buocPhut) {
    User* owner = Session::getInstance().getUser();
    if (!owner || owner->get_account_type() != "Owner") return {false, u8"Cần đăng nhập bằng tài khoản chủ sân để sửa giờ hoạt động."};
    std::string error;
    if (!venue_service_.updateFacilityHours(owner->get_user_id(), gioMo, gioDong,
                                           buocPhut == 60, buocPhut == 90, error)) {
        return {false, error};
    }
    refreshCache();
    return {true, ""};
}

int Facade::getGiaMaTran(const std::string& khungGioStr, int loaiSan) const {
    User* owner = Session::getInstance().getUser();
    if (!owner || owner->get_account_type() != "Owner") return 0;
    std::string start, end; int duration = 0;
    if (!tachKhungGio(khungGioStr, start, end, duration)) return 0;
    const auto key = std::make_tuple(owner->get_user_id(), loaiSan, duration, start);
    const auto price = giaMaTranCache_.find(key);
    return price == giaMaTranCache_.end()
        ? 0
        : static_cast<int>(price->second);
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
    refreshCache();
    return {true, ""};
}

std::map<std::pair<std::string, std::string>, Widgets::SlotInfo> Facade::getTinhTrangSlotTheoNgay(const std::string& ngay) {
    std::map<std::pair<std::string, std::string>, Widgets::SlotInfo> maTran;
    User* owner = Session::getInstance().getUser();
    if (!owner || owner->get_account_type() != "Owner") return maTran;

    std::string error;
    int dur = venue_service_.getFacilitySlotDuration(owner->get_user_id(), error);
    if (dur == 0) return maTran;

    LinkedList<VenueFieldRecord> fields;
    if (!venue_service_.getFields(owner->get_user_id(), 0, true, fields, error)) return maTran;

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

}  // namespace GUI::App
