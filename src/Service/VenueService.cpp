#include "Service/VenueService.h"

#include <cstdio>
#include <string>

#include "Core/DateTime.h"
#include "Core/HashTable.h"

namespace {

int phutSangSo(const std::string& time) {
    if (!DateTime::isValidTime(time)) return -1;
    return std::stoi(time.substr(0, 2)) * 60 + std::stoi(time.substr(3, 2));
}

std::string gioTuPhut(int totalMinutes) {
    char time[6];
    std::snprintf(
        time,
        sizeof(time),
        "%02d:%02d",
        totalMinutes / 60,
        totalMinutes % 60
    );
    return time;
}

bool laDonHuy(const std::string& status) {
    return status == "CANCELLED" || status == u8"Đã hủy";
}

bool laNgayHopLe(const std::string& date) {
    if (DateTime::isValidDate(date)) return true;
    if (date.size() == 10 && date[2] == '/' && date[5] == '/') {
        return DateTime::isValidDate(
            date.substr(6, 4) + "-" + date.substr(3, 2) + "-" +
            date.substr(0, 2)
        );
    }
    return false;
}

std::string chuanHoaNgay(const std::string& date) {
    if (date.size() == 10 && date[2] == '/' && date[5] == '/') {
        return date.substr(6, 4) + "-" + date.substr(3, 2) + "-" +
               date.substr(0, 2);
    }
    return date;
}

bool coSizeHopLe(int size) {
    return size == 5 || size == 7 || size == 11;
}

bool coThoiLuongHopLe(int duration) {
    return duration == 60 || duration == 90;
}

bool timGia(const LinkedList<VenuePriceRecord>& prices,
            const std::string& ownerId, int fieldSize, int duration,
            const std::string& startTime, long long& amount) {
    for (auto* node = prices.get_head(); node != nullptr; node = node->next) {
        const VenuePriceRecord& price = node->data;
        if (price.ownerId == ownerId && price.fieldSize == fieldSize &&
            price.durationMinutes == duration && price.startTime == startTime) {
            amount = price.priceVnd;
            return true;
        }
    }
    return false;
}

bool xungDot(int startA, int endA, int startB, int endB) {
    return startA < endB && startB < endA;
}

}  // Kết thúc không gian tên ẩn danh

VenueService::VenueService(const VenueRepository& repository)
    : repository_(repository) {}

bool VenueService::getFacilities(
    LinkedList<VenueFacilityRecord>& facilities,
    std::string& error
) const {
    return repository_.loadFacilities(facilities, error);
}

bool VenueService::getLowestPrice(
    const std::string& ownerId,
    long long& priceVnd,
    std::string& error
) const {
    LinkedList<VenuePriceRecord> prices;
    if (!repository_.loadPrices(prices, error)) return false;
    priceVnd = 0;
    for (auto* node = prices.get_head(); node != nullptr; node = node->next) {
        const VenuePriceRecord& price = node->data;
        if (price.ownerId == ownerId &&
            (priceVnd == 0 || price.priceVnd < priceVnd)) {
            priceVnd = price.priceVnd;
        }
    }
    return true;
}

bool VenueService::getFields(
    const std::string& ownerId,
    int fieldSize,
    bool includeUnavailable,
    LinkedList<VenueFieldRecord>& fields,
    std::string& error
) const {
    LinkedList<VenueFieldRecord> allFields;
    LinkedList<VenueFacilityRecord> facilities;
    if (!repository_.loadFields(allFields, error) ||
        !repository_.loadFacilities(facilities, error)) {
        return false;
    }
    bool ownerExists = false;
    for (auto* node = facilities.get_head(); node != nullptr; node = node->next) {
        ownerExists = ownerExists || node->data.ownerId == ownerId;
    }
    if (!ownerExists) {
        error = "Không tìm thấy cơ sở của chủ sân.";
        return false;
    }

    fields.clear();
    for (auto* node = allFields.get_head(); node != nullptr; node = node->next) {
        const VenueFieldRecord& field = node->data;
        if (field.ownerId == ownerId &&
            (fieldSize == 0 || field.size == fieldSize) &&
            (includeUnavailable || field.status == 0)) {
            fields.push_back(field);
        }
    }
    return true;
}

bool VenueService::getSlots(
    const std::string& fieldId,
    const std::string& date,
    int durationMinutes,
    LinkedList<VenueSlotRecord>& slots,
    std::string& error,
    bool allowPastDate
) const {
    error.clear();
    slots.clear();
    if (!coThoiLuongHopLe(durationMinutes) || !laNgayHopLe(date)) {
        error = "Ngày hoặc thời lượng ca không hợp lệ.";
        return false;
    }
    if (!allowPastDate && DateTime::isPastDate(chuanHoaNgay(date))) {
        error = "Không thể xem khung giờ của ngày trong quá khứ.";
        return false;
    }

    LinkedList<VenueFacilityRecord> facilities;
    LinkedList<VenueFieldRecord> fields;
    LinkedList<VenuePriceRecord> prices;
    LinkedList<VenueBookingRecord> bookings;
    if (!repository_.loadFacilities(facilities, error) ||
        !repository_.loadFields(fields, error) ||
        !repository_.loadPrices(prices, error) ||
        !repository_.loadBookings(bookings, error)) {
        return false;
    }

    HashTable<std::string, VenueFacilityRecord*> facilityIndex;
    HashTable<std::string, VenueFieldRecord*> fieldIndex;
    for (auto* node = facilities.get_head(); node != nullptr; node = node->next) {
        facilityIndex.insert(node->data.ownerId, &node->data);
    }
    for (auto* node = fields.get_head(); node != nullptr; node = node->next) {
        fieldIndex.insert(node->data.fieldId, &node->data);
    }

    VenueFieldRecord* field = nullptr;
    if (!fieldIndex.search(fieldId, field) || field == nullptr) {
        error = "Không tìm thấy sân cần xem lịch.";
        return false;
    }
    if (field->status != 0) {
        error = "Sân hiện không nhận đặt chỗ.";
        return false;
    }
    VenueFacilityRecord* facility = nullptr;
    if (!facilityIndex.search(field->ownerId, facility) || facility == nullptr) {
        error = "Không tìm thấy thông tin cơ sở của sân.";
        return false;
    }
    if ((durationMinutes == 60 && !facility->allows60Minutes) ||
        (durationMinutes == 90 && !facility->allows90Minutes)) {
        error = "Cơ sở không hỗ trợ thời lượng ca đã chọn.";
        return false;
    }

    const int opening = phutSangSo(facility->openTime);
    const int closing = phutSangSo(facility->closeTime);
    if (opening < 0 || closing <= opening) {
        error = "Giờ hoạt động của cơ sở không hợp lệ.";
        return false;
    }
    for (int start = opening; start + durationMinutes <= closing; start += 30) {
        VenueSlotRecord slot;
        slot.startTime = gioTuPhut(start);
        slot.endTime = gioTuPhut(start + durationMinutes);
        if (!timGia(prices, field->ownerId, field->size, durationMinutes,
                    slot.startTime, slot.priceVnd)) {
            continue;
        }
        slot.past = DateTime::isPastSlot(date, slot.startTime);
        slot.available = !slot.past;
        for (auto* node = bookings.get_head(); node != nullptr; node = node->next) {
            const VenueBookingRecord& booking = node->data;
            if (booking.fieldId != fieldId ||
                chuanHoaNgay(booking.date) != chuanHoaNgay(date) ||
                laDonHuy(booking.status)) {
                continue;
            }
            const int bookedStart = phutSangSo(booking.startTime);
            const int bookedEnd = phutSangSo(booking.endTime);
            if (bookedStart >= 0 && bookedEnd > bookedStart &&
                xungDot(start, start + durationMinutes, bookedStart, bookedEnd)) {
                slot.available = false;
                slot.bookingId = booking.bookingId;
                break;
            }
        }
        slots.push_back(slot);
    }
    return true;
}

bool VenueService::setFieldPrice(
    const std::string& requesterOwnerId,
    int fieldSize,
    int durationMinutes,
    const std::string& startTime,
    long long priceVnd,
    std::string& error,
    bool allStarts
) const {
    error.clear();
    if (!coSizeHopLe(fieldSize) || !coThoiLuongHopLe(durationMinutes) ||
        (!allStarts && phutSangSo(startTime) < 0) || priceVnd <= 0) {
        error = "Loại sân, ca, thời lượng hoặc giá không hợp lệ.";
        return false;
    }
    LinkedList<VenueFacilityRecord> facilities;
    LinkedList<VenuePriceRecord> prices;
    if (!repository_.loadFacilities(facilities, error) ||
        !repository_.loadPrices(prices, error)) {
        return false;
    }
    bool ownerExists = false;
    for (auto* node = facilities.get_head(); node != nullptr; node = node->next) {
        ownerExists = ownerExists || node->data.ownerId == requesterOwnerId;
    }
    if (!ownerExists) {
        error = "Không có quyền sửa bảng giá của cơ sở này.";
        return false;
    }
    bool updated = false;
    for (auto* node = prices.get_head(); node != nullptr; node = node->next) {
        VenuePriceRecord& price = node->data;
        if (price.ownerId == requesterOwnerId && price.fieldSize == fieldSize &&
            price.durationMinutes == durationMinutes &&
            (allStarts || price.startTime == startTime)) {
            price.priceVnd = priceVnd;
            updated = true;
            if (!allStarts) return repository_.savePrices(prices, error);
        }
    }
    if (allStarts) {
        if (!updated) {
            error = "Cơ sở chưa có khung giờ thuộc loại sân và thời lượng này.";
            return false;
        }
    } else if (!updated) {
        prices.push_back({
            requesterOwnerId, fieldSize, durationMinutes, startTime, priceVnd
        });
    }
    return repository_.savePrices(prices, error);
}

bool VenueService::updateFacilityHours(
    const std::string& requesterOwnerId,
    const std::string& openTime,
    const std::string& closeTime,
    bool allows60Minutes,
    bool allows90Minutes,
    std::string& error
) const {
    if (requesterOwnerId.empty() || !DateTime::isValidTime(openTime) ||
        !DateTime::isValidTime(closeTime) ||
        DateTime::compareTime(openTime, closeTime) >= 0 ||
        (!allows60Minutes && !allows90Minutes)) {
        error = "Giờ mở cửa hoặc đóng cửa không hợp lệ.";
        return false;
    }
    LinkedList<VenueFacilityRecord> facilities;
    if (!repository_.loadFacilities(facilities, error)) return false;
    bool ownerExists = false;
    for (auto* node = facilities.get_head(); node != nullptr; node = node->next) {
        ownerExists = ownerExists || node->data.ownerId == requesterOwnerId;
    }
    if (!ownerExists) {
        error = "Không có quyền thay đổi giờ hoạt động của cơ sở này.";
        return false;
    }
    return repository_.updateFacilityHours(
        requesterOwnerId, openTime, closeTime, allows60Minutes,
        allows90Minutes, error
    );
}

bool VenueService::createField(
    const std::string& requesterOwnerId,
    const VenueFieldRecord& field,
    std::string& error
) const {
    if (requesterOwnerId.empty() || field.ownerId != requesterOwnerId ||
        field.fieldId.empty() || field.name.empty() ||
        !coSizeHopLe(field.size) || field.status < 0 || field.status > 2) {
        error = "Thông tin sân không hợp lệ hoặc không thuộc chủ sân đang đăng nhập.";
        return false;
    }
    LinkedList<VenueFacilityRecord> facilities;
    LinkedList<VenueFieldRecord> fields;
    if (!repository_.loadFacilities(facilities, error) ||
        !repository_.loadFields(fields, error)) {
        return false;
    }
    bool ownerExists = false;
    for (auto* node = facilities.get_head(); node != nullptr; node = node->next) {
        ownerExists = ownerExists || node->data.ownerId == requesterOwnerId;
    }
    for (auto* node = fields.get_head(); node != nullptr; node = node->next) {
        if (node->data.fieldId == field.fieldId) {
            error = "Mã sân đã tồn tại trên toàn hệ thống.";
            return false;
        }
    }
    if (!ownerExists) {
        error = "Không tìm thấy chủ sân.";
        return false;
    }
    fields.push_back(field);
    return repository_.saveFields(fields, error) &&
           repository_.seedIfNeeded(error);
}

bool VenueService::updateField(
    const std::string& requesterOwnerId,
    const VenueFieldRecord& field,
    std::string& error
) const {
    LinkedList<VenueFieldRecord> fields;
    if (!repository_.loadFields(fields, error)) return false;
    for (auto* node = fields.get_head(); node != nullptr; node = node->next) {
        VenueFieldRecord& current = node->data;
        if (current.fieldId != field.fieldId) continue;
        if (current.ownerId != requesterOwnerId ||
            field.ownerId != requesterOwnerId) {
            error = "Không có quyền sửa sân của chủ sân khác.";
            return false;
        }
        if (field.name.empty() || !coSizeHopLe(field.size) ||
            field.status < 0 || field.status > 2) {
            error = "Thông tin sân không hợp lệ.";
            return false;
        }
        current = field;
        return repository_.saveFields(fields, error) &&
               repository_.seedIfNeeded(error);
    }
    error = "Không tìm thấy sân cần cập nhật.";
    return false;
}

bool VenueService::deleteField(
    const std::string& requesterOwnerId,
    const std::string& fieldId,
    std::string& error
) const {
    LinkedList<VenueFieldRecord> fields;
    LinkedList<VenueBookingRecord> bookings;
    if (!repository_.loadFields(fields, error) ||
        !repository_.loadBookings(bookings, error)) {
        return false;
    }
    bool found = false;
    for (auto* node = fields.get_head(); node != nullptr; node = node->next) {
        if (node->data.fieldId != fieldId) continue;
        if (node->data.ownerId != requesterOwnerId) {
            error = "Không có quyền xóa sân của chủ sân khác.";
            return false;
        }
        found = true;
        break;
    }
    if (!found) {
        error = "Không tìm thấy sân cần xóa.";
        return false;
    }
    for (auto* node = bookings.get_head(); node != nullptr; node = node->next) {
        if (node->data.fieldId == fieldId && !laDonHuy(node->data.status)) {
            error = "Không thể xóa sân đã có dữ liệu đặt chỗ; hãy chuyển sang ngừng hoạt động.";
            return false;
        }
    }
    fields.removeIf([&fieldId](const VenueFieldRecord& field) {
        return field.fieldId == fieldId;
    });
    return repository_.saveFields(fields, error);
}

bool VenueService::createBooking(
    const VenueBookingRequest& request,
    std::string& bookingId,
    std::string& error
) const {
    error.clear();
    if (request.customerId.empty() || request.customerPhone.empty() ||
        !coThoiLuongHopLe(request.durationMinutes) ||
        !laNgayHopLe(request.date)) {
        error = "Thông tin khách, ngày hoặc thời lượng đặt sân không hợp lệ.";
        return false;
    }
    const std::string date = chuanHoaNgay(request.date);
    if (DateTime::isPastDate(date)) {
        error = "Không thể đặt sân vào ngày trong quá khứ.";
        return false;
    }
    LinkedList<VenueFacilityRecord> facilities;
    LinkedList<VenueFieldRecord> fields;
    LinkedList<VenuePriceRecord> prices;
    LinkedList<VenueBookingRecord> bookings;
    if (!repository_.loadFacilities(facilities, error) ||
        !repository_.loadFields(fields, error) ||
        !repository_.loadPrices(prices, error) ||
        !repository_.loadBookings(bookings, error)) {
        return false;
    }
    HashTable<std::string, VenueBookingRecord*> bookingsById;
    for (auto* node = bookings.get_head(); node != nullptr; node = node->next) {
        bookingsById.insert(node->data.bookingId, &node->data);
    }
    std::string accountName;
    std::string accountPhone;
    if (!repository_.getCustomerDetails(
            request.customerId, accountName, accountPhone, error)) {
        return false;
    }
    if (accountPhone != request.customerPhone ||
        (!request.customerName.empty() && accountName != request.customerName)) {
        error = "Thông tin người đặt không khớp tài khoản đang đăng nhập.";
        return false;
    }
    VenueFieldRecord* field = nullptr;
    HashTable<std::string, VenueFieldRecord*> fieldsById;
    for (auto* node = fields.get_head(); node != nullptr; node = node->next) {
        fieldsById.insert(node->data.fieldId, &node->data);
    }
    if (!fieldsById.search(request.fieldId, field) || field == nullptr ||
        field->status != 0) {
        error = "Không tìm thấy sân đang nhận đặt chỗ.";
        return false;
    }
    VenueFacilityRecord* facility = nullptr;
    HashTable<std::string, VenueFacilityRecord*> facilitiesByOwner;
    for (auto* node = facilities.get_head(); node != nullptr; node = node->next) {
        facilitiesByOwner.insert(node->data.ownerId, &node->data);
    }
    if (!facilitiesByOwner.search(field->ownerId, facility) || facility == nullptr) {
        error = "Không tìm thấy cơ sở của sân.";
        return false;
    }
    if ((request.durationMinutes == 60 && !facility->allows60Minutes) ||
        (request.durationMinutes == 90 && !facility->allows90Minutes)) {
        error = "Cơ sở không hỗ trợ thời lượng ca đã chọn.";
        return false;
    }
    std::string blacklistReason;
    if (repository_.isCustomerBlacklisted(
            request.customerId,
            request.customerPhone,
            blacklistReason,
            error)) {
        error = "Tài khoản đang bị chặn đặt sân: " + blacklistReason;
        return false;
    }
    if (!error.empty()) return false;

    const int start = phutSangSo(request.startTime);
    const int opening = phutSangSo(facility->openTime);
    const int closing = phutSangSo(facility->closeTime);
    if (start < opening || start + request.durationMinutes > closing) {
        error = "Khung giờ nằm ngoài giờ hoạt động của cơ sở.";
        return false;
    }
    if (date == DateTime::today() &&
        start <= phutSangSo(DateTime::nowTime())) {
        error = "Khung giờ đã qua hoặc đang diễn ra hôm nay.";
        return false;
    }
    long long priceVnd = 0;
    if (!timGia(prices, field->ownerId, field->size,
                request.durationMinutes, request.startTime, priceVnd)) {
        error = "Cơ sở chưa cấu hình giá cho ca đã chọn.";
        return false;
    }
    for (auto* node = bookings.get_head(); node != nullptr; node = node->next) {
        const VenueBookingRecord& oldBooking = node->data;
        if (oldBooking.fieldId != request.fieldId ||
            chuanHoaNgay(oldBooking.date) != date || laDonHuy(oldBooking.status)) {
            continue;
        }
        const int oldStart = phutSangSo(oldBooking.startTime);
        const int oldEnd = phutSangSo(oldBooking.endTime);
        if (oldStart >= 0 && oldEnd > oldStart &&
            xungDot(start, start + request.durationMinutes, oldStart, oldEnd)) {
            error = "Khung giờ này đã được đặt.";
            return false;
        }
    }

    if (!request.bookingId.empty()) {
        VenueBookingRecord* existing = nullptr;
        if (bookingsById.search(request.bookingId, existing)) {
            error = "Mã đơn đặt sân đã tồn tại.";
            return false;
        }
        bookingId = request.bookingId;
    } else {
        int sequence = bookings.size() + 1;
        bool duplicate = true;
        while (duplicate) {
            char id[32];
            std::snprintf(id, sizeof(id), "BK-%06d", sequence++);
            bookingId = id;
            VenueBookingRecord* existing = nullptr;
            duplicate = bookingsById.search(bookingId, existing);
        }
    }
    bookings.push_back({
        bookingId,
        field->fieldId,
        field->ownerId,
        request.customerId,
        request.customerName,
        request.customerPhone,
        date,
        request.startTime,
        gioTuPhut(start + request.durationMinutes),
        request.durationMinutes,
        priceVnd,
        "CONFIRMED"
    });
    return repository_.saveBookings(bookings, error);
}
