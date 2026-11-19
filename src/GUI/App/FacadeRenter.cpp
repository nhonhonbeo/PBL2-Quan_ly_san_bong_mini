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

int Facade::getSoLanHuyCuaKhach(const std::string& sdt) const {
    Customer c("", "", "", "", ""); std::string err;
    return customer_service_.findCustomer(sdt, c, err) ? c.getCancelCount() : 0;
}

int Facade::getSoLanDatCuaKhach(const std::string& sdt) const {
    Customer c("", "", "", "", ""); std::string err;
    return customer_service_.findCustomer(sdt, c, err) ? c.getBookingCount() : 0;
}

KetQua Facade::datNhieuCaMoi(
    const std::string& maSan,
    const std::string& sdtKhach,
    const std::string& tenKhach,
    const std::string& ngay,
    const LinkedList<std::string>& khungGio,
    int loaiSan,
    std::string& outMaDon
) {
    outMaDon.clear();
    User* customer = Session::getInstance().getUser();
    if (!customer || customer->get_account_type() != "Customer" ||
        customer->get_phone_number() != sdtKhach) {
        return {false, u8"Vui lòng đăng nhập bằng đúng tài khoản người thuê."};
    }

    const auto field = std::find_if(
        danhSachSan_.begin(), danhSachSan_.end(),
        [&maSan, loaiSan](const SanBongInfo& item) {
            return item.maSan == maSan && item.loaiSan == loaiSan;
        }
    );
    if (field == danhSachSan_.end()) {
        return {false, u8"Không tìm thấy sân phù hợp với loại sân đã chọn."};
    }

    LinkedList<VenueBookingRequest> requests;
    for (auto* slot = khungGio.get_head(); slot != nullptr; slot = slot->next) {
        std::string start;
        std::string end;
        int duration = 0;
        if (!tachKhungGio(slot->data, start, end, duration)) {
            return {false, u8"Khung giờ đặt sân không hợp lệ."};
        }
        VenueBookingRequest request;
        request.customerId = customer->get_user_id();
        request.customerName = tenKhach;
        request.customerPhone = sdtKhach;
        request.fieldId = maSan;
        request.date = ngay;
        request.startTime = start;
        request.durationMinutes = duration;
        requests.push_back(request);
    }

    LinkedList<std::string> bookingIds;
    std::string error;
    if (!booking_service_.createBookings(requests, bookingIds, error)) {
        return {false, error};
    }
    for (auto* id = bookingIds.get_head(); id != nullptr; id = id->next) {
        if (!outMaDon.empty()) outMaDon += ", ";
        outMaDon += id->data;
    }
    refreshCache();
    return {true, ""};
}

KetQua Facade::huyDonDat(const std::string& maDon, const std::string& sdtKhach) {
    User* user = Session::getInstance().getUser();
    if (!user || user->get_account_type() != "Customer" || user->get_phone_number() != sdtKhach) {
        return {false, u8"Chỉ người thuê sở hữu đơn mới được hủy."};
    }
    std::string error;
    if (!booking_service_.cancelBooking(maDon, user->get_user_id(), error)) return {false, error};
    refreshCache();
    return {true, ""};
}

KetQua Facade::xacNhanDaHoanThanhDon(const std::string& maDon) {
    User* owner = Session::getInstance().getUser();
    if (!owner || owner->get_account_type() != "Owner") return {false, u8"Chỉ chủ sân mới được xác nhận hoàn tất đơn."};
    std::string error;
    if (!booking_service_.completeBooking(owner->get_user_id(), maDon, error)) return {false, error};
    refreshCache();
    return {true, ""};
}

std::vector<GiaiDauInfo> Facade::getGiaiDauCuaUser(const std::string& sdt) const {
    std::vector<GiaiDauInfo> ds;
    for (const auto& g : danhSachGiaiDau_) if (g.sdtKhach == sdt) ds.push_back(g);
    return ds;
}

KetQua Facade::guiYeuCauTaoGiai(const GiaiDauInfo& giaiMoi) {
    Tournament t = toTournament(giaiMoi, TournamentStatus::REQUESTED);
    std::string outId, error;
    if (!tournament_service_.guiYeuCauTaoGiai(t, outId, error)) return {false, error};
    refreshCache();
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
    Review r(danhGiaId, sdt, maSan, "", soSao, DateTime::today(), noiDung, tenKhach);
    std::string error;
    if (!customer_service_.addOrUpdateReview(r, error)) return {false, error};
    refreshCache();
    return {true, ""};
}

KetQua Facade::capNhatThongTinUser(const std::string& sdt, const std::string& hoTenMoi) { return capNhatThongTinUser(sdt, hoTenMoi, sdt); }

KetQua Facade::capNhatThongTinUser(const std::string& sdtCu, const std::string& hoTenMoi, const std::string& sdtMoi) {
    std::string error;
    if (!account_service_.updateUser(sdtCu, hoTenMoi, sdtMoi, error)) return {false, error};
    refreshCache();
    return {true, ""};
}

KetQua Facade::doiMatKhauUser(const std::string& sdt, const std::string& matKhauCu, const std::string& matKhauMoi) {
    std::string err;
    if (!account_service_.authenticate(sdt, matKhauCu, err)) return {false, u8"Mật khẩu hiện tại không chính xác!"};
    if (!account_service_.resetPassword(sdt, matKhauMoi, err)) return {false, err};
    refreshCache();
    return {true, ""};
}

}  // namespace GUI::App
