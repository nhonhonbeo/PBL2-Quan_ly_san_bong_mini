#include "Repository/BookingRepository.h"

#include "Booking/Booking.h"
#include "Booking/BookingDetail.h"
#include "Storage/FileManager.h"

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
        if (node->data.bookingId != bookingId) continue;
        out = Booking::fromRecord(node->data);
        return true;
    }
    error = "Không tìm thấy đơn đặt sân.";
    return false;
}

bool BookingRepository::save(const Booking& booking, std::string& error) {
    LinkedList<VenueBookingRecord> bookings;
    if (!loadAll(bookings, error)) return false;
    VenueBookingRecord record = booking.toRecord();
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
    VenueBookingRecord updated = booking.toRecord();
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
