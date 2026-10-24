#include "Repository/BookingRepository.h"

#include "Booking/Booking.h"
#include "Booking/BookingDetail.h"
#include "Storage/FileManager.h"

namespace {

std::string statusToText(BookingStatus status) {
    if (status == BookingStatus::CONFIRMED) return "CONFIRMED";
    if (status == BookingStatus::COMPLETED) return "COMPLETED";
    if (status == BookingStatus::CANCELLED) return "CANCELLED";
    return "PENDING";
}

BookingStatus textToStatus(const std::string& status) {
    if (status == "CONFIRMED") return BookingStatus::CONFIRMED;
    if (status == "COMPLETED") return BookingStatus::COMPLETED;
    if (status == "CANCELLED" || status == u8"Đã hủy") {
        return BookingStatus::CANCELLED;
    }
    if (status == u8"Đã xác nhận") {
        return BookingStatus::CONFIRMED;
    }
    if (status == u8"Đã hoàn tất") {
        return BookingStatus::COMPLETED;
    }
    return BookingStatus::PENDING;
}

bool toRecord(const Booking& booking, VenueBookingRecord& record,
              std::string& error) {
    const LinkedList<BookingDetail>& details = booking.getDetails();
    auto* detail = details.get_head();
    if (detail == nullptr || detail->next != nullptr) {
        error = "Chỉ hỗ trợ lưu đơn gồm đúng một khung giờ.";
        return false;
    }
    record.bookingId = booking.getBookingId();
    record.fieldId = booking.getFieldId();
    record.ownerId = booking.getOwnerId();
    record.customerId = booking.getCustomerId();
    record.date = booking.getDate();
    record.startTime = detail->data.getStartTime();
    record.endTime = detail->data.getEndTime();
    record.durationMinutes = detail->data.getDurationMinutes();
    record.priceVnd = detail->data.getAmountVnd();
    record.status = statusToText(booking.getStatus());
    return true;
}

}  // Kết thúc không gian tên ẩn danh

BookingRepository::BookingRepository(FileManager& fileManager)
    : file_manager_(fileManager) {}

bool BookingRepository::loadAll(
    LinkedList<VenueBookingRecord>& bookings,
    std::string& error
) const {
    VenueRepository repository;
    return repository.loadBookings(bookings, error);
}

bool BookingRepository::saveAll(
    const LinkedList<VenueBookingRecord>& bookings,
    std::string& error
) const {
    VenueRepository repository;
    return repository.saveBookings(bookings, error);
}

bool BookingRepository::findById(const std::string& bookingId, Booking& out,
                                 std::string& error) const {
    LinkedList<VenueBookingRecord> bookings;
    if (!loadAll(bookings, error)) return false;
    for (auto* node = bookings.get_head(); node != nullptr; node = node->next) {
        const VenueBookingRecord& record = node->data;
        if (record.bookingId != bookingId) continue;
        out = Booking(
            record.bookingId, record.customerId, record.fieldId, record.date,
            textToStatus(record.status), record.ownerId
        );
        out.addDetail(BookingDetail(
            record.bookingId, record.startTime, record.endTime,
            record.durationMinutes, record.priceVnd
        ));
        return true;
    }
    error = "Không tìm thấy đơn đặt sân.";
    return false;
}

bool BookingRepository::save(const Booking& booking, std::string& error) {
    LinkedList<VenueBookingRecord> bookings;
    if (!loadAll(bookings, error)) return false;
    VenueBookingRecord record;
    if (!toRecord(booking, record, error)) return false;
    for (auto* node = bookings.get_head(); node != nullptr; node = node->next) {
        if (node->data.bookingId == record.bookingId) {
            error = "Mã đơn đặt sân đã tồn tại.";
            return false;
        }
    }
    VenueRepository repository;
    if (!record.customerId.empty()) {
        std::string customerName;
        std::string customerPhone;
        std::string lookupError;
        if (repository.getCustomerDetails(
                record.customerId, customerName, customerPhone, lookupError)) {
            record.customerName = customerName;
            record.customerPhone = customerPhone;
        }
    }
    bookings.push_back(record);
    return repository.saveBookings(bookings, error);
}

bool BookingRepository::update(const Booking& booking, std::string& error) {
    LinkedList<VenueBookingRecord> bookings;
    if (!loadAll(bookings, error)) return false;
    VenueBookingRecord updated;
    if (!toRecord(booking, updated, error)) return false;
    for (auto* node = bookings.get_head(); node != nullptr; node = node->next) {
        VenueBookingRecord& existing = node->data;
        if (existing.bookingId != updated.bookingId) continue;
        if (existing.fieldId != updated.fieldId ||
            existing.ownerId != updated.ownerId ||
            existing.customerId != updated.customerId ||
            existing.date != updated.date) {
            error = "Không thể thay đổi thông tin chủ thể của đơn đặt sân.";
            return false;
        }
        updated.customerName = existing.customerName;
        updated.customerPhone = existing.customerPhone;
        existing = updated;
        VenueRepository repository;
        return repository.saveBookings(bookings, error);
    }
    error = "Không tìm thấy đơn đặt sân cần cập nhật.";
    return false;
}
