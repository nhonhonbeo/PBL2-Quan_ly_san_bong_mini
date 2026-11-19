#include "Repository/BookingRepository.h"

#include "Booking/Booking.h"
#include "Booking/BookingDetail.h"

BookingRepository::BookingRepository(VenueRepository& venueRepository)
    : venue_repository_(venueRepository) {}

Booking BookingRepository::toDomain(const VenueBookingRecord& record) {
    Booking booking(record.bookingId, record.customerId, record.fieldId,
                    record.date, Booking::textToStatus(record.status),
                    record.ownerId);
    booking.addDetail(BookingDetail(
        record.bookingId, record.startTime, record.endTime,
        record.durationMinutes, record.priceVnd
    ));
    return booking;
}

VenueBookingRecord BookingRepository::toRecord(const Booking& booking) {
    VenueBookingRecord record;
    record.bookingId = booking.getBookingId();
    record.fieldId = booking.getFieldId();
    record.ownerId = booking.getOwnerId();
    record.customerId = booking.getCustomerId();
    record.date = booking.getDate();
    record.status = Booking::statusToText(booking.getStatus());
    const auto* detail = booking.getDetails().get_head();
    if (detail != nullptr) {
        record.startTime = detail->data.getStartTime();
        record.endTime = detail->data.getEndTime();
        record.durationMinutes = detail->data.getDurationMinutes();
        record.priceVnd = detail->data.getAmountVnd();
    }
    return record;
}

bool BookingRepository::loadAll(
    LinkedList<VenueBookingRecord>& bookings,
    std::string& error
) const {
    return venue_repository_.loadBookings(bookings, error);
}

bool BookingRepository::saveAll(
    const LinkedList<VenueBookingRecord>& bookings,
    std::string& error
) const {
    return venue_repository_.saveBookings(bookings, error);
}

bool BookingRepository::findById(const std::string& bookingId, Booking& out,
                                 std::string& error) const {
    LinkedList<VenueBookingRecord> bookings;
    if (!loadAll(bookings, error)) return false;
    for (auto* node = bookings.get_head(); node != nullptr; node = node->next) {
        if (node->data.bookingId != bookingId) continue;
        out = toDomain(node->data);
        return true;
    }
    error = "Không tìm thấy đơn đặt sân.";
    return false;
}

bool BookingRepository::save(const Booking& booking, std::string& error) {
    LinkedList<VenueBookingRecord> bookings;
    if (!loadAll(bookings, error)) return false;
    VenueBookingRecord record = toRecord(booking);
    for (auto* node = bookings.get_head(); node != nullptr; node = node->next) {
        if (node->data.bookingId == record.bookingId) {
            error = "Mã đơn đặt sân đã tồn tại.";
            return false;
        }
    }
    bookings.push_back(record);
    return venue_repository_.saveBookings(bookings, error);
}

bool BookingRepository::update(const Booking& booking, std::string& error) {
    LinkedList<VenueBookingRecord> bookings;
    if (!loadAll(bookings, error)) return false;
    VenueBookingRecord updated = toRecord(booking);
    VenueBookingRecord* existing = bookings.findIf(
        [&updated](const VenueBookingRecord& record) {
            return record.bookingId == updated.bookingId;
        });
    if (existing != nullptr) {
        if (existing->fieldId != updated.fieldId ||
            existing->ownerId != updated.ownerId ||
            existing->customerId != updated.customerId ||
            existing->date != updated.date) {
            error = "Không thể thay đổi thông tin chủ thể của đơn đặt sân.";
            return false;
        }
        updated.customerName = existing->customerName;
        updated.customerPhone = existing->customerPhone;
        *existing = updated;
        return venue_repository_.saveBookings(bookings, error);
    }
    error = "Không tìm thấy đơn đặt sân cần cập nhật.";
    return false;
}
