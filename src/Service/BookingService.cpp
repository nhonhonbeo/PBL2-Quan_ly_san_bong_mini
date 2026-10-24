#include "Service/BookingService.h"

#include "Booking/Booking.h"
#include "Booking/BookingDetail.h"
#include "Core/DateTime.h"
#include "Core/HashTable.h"
#include "Repository/BookingRepository.h"
#include "Repository/VenueRepository.h"
#include "Service/VenueService.h"

BookingService::BookingService(BookingRepository& bookingRepository,
                               CustomerService& customerService)
    : booking_repository_(bookingRepository),
      customer_service_(customerService) {}

bool BookingService::createBooking(const Booking& booking,
                                   std::string& error) {
    const LinkedList<BookingDetail>& details = booking.getDetails();
    const auto* detail = details.get_head();
    if (detail == nullptr || detail->next != nullptr) {
        error = "Chỉ hỗ trợ đặt đúng một khung giờ cho mỗi đơn.";
        return false;
    }
    VenueRepository repository;
    std::string customerName;
    std::string customerPhone;
    if (!repository.getCustomerDetails(
            booking.getCustomerId(), customerName, customerPhone, error)) {
        return false;
    }
    VenueBookingRequest request;
    request.bookingId = booking.getBookingId();
    request.customerId = booking.getCustomerId();
    request.customerName = customerName;
    request.customerPhone = customerPhone;
    request.fieldId = booking.getFieldId();
    request.date = booking.getDate();
    request.startTime = detail->data.getStartTime();
    request.durationMinutes = detail->data.getDurationMinutes();
    std::string createdBookingId;
    return createBooking(request, createdBookingId, error);
}

bool BookingService::createBooking(const VenueBookingRequest& request,
                                   std::string& bookingId,
                                   std::string& error) {
    VenueRepository repository;
    VenueService service(repository);
    return service.createBooking(request, bookingId, error);
}

bool BookingService::createBookings(
    const LinkedList<VenueBookingRequest>& requests,
    LinkedList<std::string>& bookingIds,
    std::string& error
) {
    bookingIds.clear();
    if (requests.isEmpty()) {
        error = "Chọn ít nhất một khung giờ trước khi xác nhận.";
        return false;
    }

    const VenueBookingRequest& first = requests.get_head()->data;
    int previousEnd = -1;
    for (auto* node = requests.get_head(); node != nullptr; node = node->next) {
        const VenueBookingRequest& request = node->data;
        if (request.fieldId != first.fieldId ||
            DateTime::toIsoDate(request.date) !=
                DateTime::toIsoDate(first.date) ||
            request.customerId != first.customerId ||
            request.durationMinutes != first.durationMinutes ||
            !DateTime::isValidTime(request.startTime)) {
            error = "Các ca phải thuộc cùng sân, ngày, khách và thời lượng.";
            return false;
        }
        const int start = std::stoi(request.startTime.substr(0, 2)) * 60 +
                          std::stoi(request.startTime.substr(3, 2));
        if (previousEnd >= 0 && start != previousEnd) {
            error = "Chỉ có thể đặt các ca liên tiếp trên cùng sân và ngày.";
            return false;
        }
        previousEnd = start + request.durationMinutes;
    }

    VenueRepository repository;
    LinkedList<VenueBookingRecord> originalBookings;
    if (!repository.loadBookings(originalBookings, error)) return false;

    VenueService service(repository);
    for (auto* node = requests.get_head(); node != nullptr; node = node->next) {
        std::string bookingId;
        if (!service.createBooking(node->data, bookingId, error)) {
            std::string rollbackError;
            if (!repository.saveBookings(originalBookings, rollbackError)) {
                error += " Không thể khôi phục dữ liệu sau lỗi: " + rollbackError;
            }
            bookingIds.clear();
            return false;
        }
        bookingIds.push_back(bookingId);
    }
    return true;
}

bool BookingService::cancelBooking(const std::string& bookingId,
                                   const std::string& customerId,
                                   std::string& error) {
    LinkedList<VenueBookingRecord> bookings;
    if (!booking_repository_.loadAll(bookings, error)) return false;
    for (auto* node = bookings.get_head(); node != nullptr; node = node->next) {
        VenueBookingRecord& booking = node->data;
        if (booking.bookingId != bookingId) continue;
        if (booking.customerId != customerId) {
            error = "Không có quyền hủy đơn đặt sân của người khác.";
            return false;
        }
        if (booking.status == "COMPLETED" || booking.status == "CANCELLED" ||
            booking.status == u8"Đã hoàn tất" ||
            booking.status == u8"Đã hủy") {
            error = "Đơn đã hoàn tất hoặc đã hủy, không thể hủy thêm.";
            return false;
        }
        booking.status = "CANCELLED";
        return booking_repository_.saveAll(bookings, error);
    }
    error = "Không tìm thấy đơn đặt sân cần hủy.";
    return false;
}

bool BookingService::completeBooking(const std::string& requesterOwnerId,
                                     const std::string& bookingId,
                                     std::string& error) {
    LinkedList<VenueBookingRecord> bookings;
    if (!booking_repository_.loadAll(bookings, error)) return false;
    for (auto* node = bookings.get_head(); node != nullptr; node = node->next) {
        VenueBookingRecord& booking = node->data;
        if (booking.bookingId != bookingId) continue;
        if (booking.ownerId != requesterOwnerId) {
            error = "Không có quyền xác nhận đơn của cơ sở khác.";
            return false;
        }
        if (booking.status == "CANCELLED" ||
            booking.status == u8"Đã hủy") {
            error = "Không thể hoàn tất đơn đã hủy.";
            return false;
        }
        if (booking.status == "COMPLETED" ||
            booking.status == u8"Đã hoàn thành") {
            error = "Đơn này đã được xác nhận hoàn tất.";
            return false;
        }
        std::string bookingDate = booking.date;
        if (bookingDate.size() == 10 && bookingDate[2] == '/' &&
            bookingDate[5] == '/') {
            bookingDate = bookingDate.substr(6, 4) + "-" +
                          bookingDate.substr(3, 2) + "-" +
                          bookingDate.substr(0, 2);
        }
        const int dateOrder = DateTime::compareDate(
            bookingDate, DateTime::today()
        );
        if (dateOrder > 0 ||
            (dateOrder == 0 &&
             DateTime::compareTime(DateTime::nowTime(), booking.endTime) < 0)) {
            error = "Chỉ có thể xác nhận hoàn tất sau khi ca đã kết thúc.";
            return false;
        }
        booking.status = "COMPLETED";
        return booking_repository_.saveAll(bookings, error);
    }
    error = "Không tìm thấy đơn đặt sân cần hoàn tất.";
    return false;
}
