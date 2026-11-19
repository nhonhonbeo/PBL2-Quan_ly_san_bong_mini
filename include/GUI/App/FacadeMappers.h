#pragma once
// Hàm tiện ích nội bộ dùng chung cho các file Facade*.cpp (không phải API công khai).
// LÝ DO CHỌN: tách khỏi Facade.cpp để mỗi file Facade* chỉ chứa một nhóm chức năng
// mà vẫn dùng chung một bộ hàm ánh xạ (tránh sao chép).
#include <string>

#include "Core/DateTime.h"
#include "GUI/App/Facade.h"
#include "Tournament/Tournament.h"

namespace GUI::App::detail {

inline bool tachKhungGio(const std::string& text, std::string& start, std::string& end, int& duration) {
    const std::size_t sep = text.find(" - ");
    if (sep == std::string::npos) return false;
    start = text.substr(0, sep); end = text.substr(sep + 3);
    if (!DateTime::isValidTime(start) || !DateTime::isValidTime(end)) return false;
    duration = (std::stoi(end.substr(0, 2)) * 60 + std::stoi(end.substr(3, 2))) -
               (std::stoi(start.substr(0, 2)) * 60 + std::stoi(start.substr(3, 2)));
    return duration == 60 || duration == 90;
}

inline int gioSangPhut(const std::string& timeStr) {
    return (timeStr.length() == 5 && timeStr[2] == ':') ?
        std::stoi(timeStr.substr(0, 2)) * 60 + std::stoi(timeStr.substr(3, 2)) : 0;
}


inline Tournament toTournament(const GiaiDauInfo& g, TournamentStatus st) {
    return Tournament(g.maGiai, g.sdtKhach, g.tenGiai, g.maSan,
                      g.ngayBatDau, g.ngayKetThuc, g.moTa,
                      st, "", g.tenKhachDat, g.khungGio, g.ghiChuTuChoi);
}

inline GiaiDauInfo toGiaiDauInfo(const Tournament& t) {
    return {t.getTournamentId(), t.getName(), t.getRequesterName(), t.getRequesterUserId(),
            t.getStartDate(), t.getEndDate(), t.getRequestedFieldId(), t.getTimeSlot(),
            t.getNote(), t.getRejectReason(), static_cast<TrangThaiGiaiDau>(static_cast<int>(t.getStatus()))};
}

inline DonDatSanInfo toBookingInfo(const ReportBookingRecord& record) {
    DonDatSanInfo b;
    b.maDon = record.bookingId; b.maSan = record.fieldId; b.ownerUserId = record.ownerUserId;
    b.customerId = record.customerId; b.tenKhachHang = record.customerName; b.sdt = record.customerPhone;
    b.ngay = record.date; b.khungGio = record.timeSlot.empty() ? (record.startTime + " - " + record.endTime) : record.timeSlot;
    b.thoiLuongPhut = record.durationMinutes; b.giaTien = static_cast<int>(record.amountVnd); b.trangThai = record.status;
    return b;
}

}  // namespace GUI::App::detail
