#include "Service/BookingService.h"
#include "Service/CustomerService.h"

#include "Booking/Booking.h"
#include "Booking/BookingDetail.h"
#include "Account/Customer.h"
#include "Core/DateTime.h"
#include "Core/HashTable.h"
#include "Repository/BookingRepository.h"
#include "Service/VenueService.h"

namespace {

bool validateBookingCustomer(
    CustomerService& customerService,
    const VenueBookingRequest& request,
    std::string& error
) {
    bool isBlocked = false;
    std::string blockReason;
    if (!customerService.isBlacklisted(
            request.customerId, isBlocked, blockReason, error)) {
        if (error.empty()) error = "Không thể kiểm tra danh sách đen.";
        return false;
    }
    if (isBlocked) {
        error = "Tai khoan cua ban dang bi khoa do vi pham" +
                (blockReason.empty() ? "." : (": " + blockReason));
        return false;
    }

    Customer customer;
    if (!customerService.findCustomer(request.customerId, customer, error)) {
        return false;
    }
    if (customer.get_phone_number() != request.customerPhone ||
        (!request.customerName.empty() &&
         customer.get_name() != request.customerName)) {
        error = "Thông tin người đặt không khớp tài khoản đang đăng nhập.";
        return false;
    }
    return true;
}

}  // Kết thúc không gian tên ẩn danh

BookingService::BookingService(BookingRepository& bookingRepository,
                               CustomerService& customerService,
                               VenueService& venueService)
    : booking_repository_(bookingRepository),
      customer_service_(customerService),
      venue_service_(venueService) {}

bool BookingService::createBooking(const Booking& booking,
                                   std::string& error) {
    const LinkedList<BookingDetail>& details = booking.getDetails();
    const auto* detail = details.get_head();
    if (detail == nullptr || detail->next != nullptr) {
        error = "Chỉ hỗ trợ đặt đúng một khung giờ cho mỗi đơn.";
        return false;
    }
    Customer customer;
    if (!customer_service_.findCustomer(booking.getCustomerId(), customer, error)) {
        return false;
    }
    VenueBookingRequest request;
    request.bookingId = booking.getBookingId();
    request.customerId = booking.getCustomerId();
    request.customerName = customer.get_name();
    request.customerPhone = customer.get_phone_number();
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
    if (!validateBookingCustomer(customer_service_, request, error)) return false;
    return venue_service_.createBooking(request, bookingId, error);
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
    if (!validateBookingCustomer(customer_service_, first, error)) return false;

    // 1. Kiểm tra tính hợp lệ cơ bản của chuỗi ca liên tiếp
    int previousEnd = -1;
    for (auto* node = requests.get_head(); node != nullptr; node = node->next) {
        const VenueBookingRequest& request = node->data;
        if (request.fieldId != first.fieldId ||
            DateTime::toIsoDate(request.date) != DateTime::toIsoDate(first.date) ||
            request.customerId != first.customerId ||
            request.customerName != first.customerName ||
            request.customerPhone != first.customerPhone ||
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

    // 2. Tải bản sao trạng thái ban đầu của Bookings để rollback khi gặp lỗi
    LinkedList<VenueBookingRecord> originalBookings;
    if (!booking_repository_.loadAll(originalBookings, error)) {
        return false;
    }

    // 3. Tiến hành đặt từng ca kèm theo dõi danh sách để rollback nguyên khối nếu bất kỳ bước nào thất bại
    LinkedList<std::string> createdBookingIds;
    bool allSuccess = true;

    for (auto* node = requests.get_head(); node != nullptr; node = node->next) {
        std::string currentBookingId;
        if (!venue_service_.createBooking(node->data, currentBookingId, error)) {
            allSuccess = false;
            break;
        }
        createdBookingIds.push_back(currentBookingId);
    }

    if (!allSuccess) {
        // Rollback toàn bộ đơn đặt sân
        std::string rollbackErr;
        if (!booking_repository_.saveAll(originalBookings, rollbackErr)) {
            error += ". Loi nghiem trong: Khong the khoi phuc trang thai bookings: " + rollbackErr;
        }
        bookingIds.clear();
        return false;
    }

    bookingIds = createdBookingIds;
    return true;
}

bool BookingService::cancelBooking(const std::string& bookingId,
                                   const std::string& customerId,
                                   std::string& error) {
    LinkedList<VenueBookingRecord> bookings;
    if (!booking_repository_.loadAll(bookings, error)) return false;
    const LinkedList<VenueBookingRecord> originalBookings = bookings;
    VenueBookingRecord* booking = bookings.findIf(
        [&bookingId](const VenueBookingRecord& record) {
            return record.bookingId == bookingId;
        });
    if (booking != nullptr) {

        Booking domainBooking = BookingRepository::toDomain(*booking);
        std::string domainErr;
        if (!domainBooking.cancel(customerId, domainErr)) {
            error = domainErr;
            return false;
        }

        booking->status = Booking::statusToText(domainBooking.getStatus());
        if (!booking_repository_.saveAll(bookings, error)) return false;

        std::string custErr;
        if (!customer_service_.recordCancellation(
                booking->customerId, booking->bookingId, custErr)) {
            error = "Không thể cập nhật lịch sử hủy: " + custErr;
            std::string bookingRollbackError;
            if (!booking_repository_.saveAll(originalBookings,
                                             bookingRollbackError)) {
                error += "; không thể khôi phục booking: " +
                         bookingRollbackError;
            }
            return false;
        }
        error.clear();
        return true;
    }
    error = "Không tìm thấy đơn đặt sân cần hủy.";
    return false;
}

bool BookingService::completeBooking(const std::string& requesterOwnerId,
                                     const std::string& bookingId,
                                     std::string& error) {
    LinkedList<VenueBookingRecord> bookings;
    if (!booking_repository_.loadAll(bookings, error)) return false;
                                     const LinkedList<VenueBookingRecord> originalBookings = bookings;
                                     VenueBookingRecord* booking = bookings.findIf(
                                         [&bookingId](const VenueBookingRecord& record) {
                                             return record.bookingId == bookingId;
                                         });
                                     if (booking != nullptr) {

                                         Booking domainBooking = BookingRepository::toDomain(*booking);
        std::string domainErr;
        if (!domainBooking.canBeCompletedBy(requesterOwnerId, domainErr)) {
            error = domainErr;
            return false;
        }

        std::string bookingDate = booking->date;
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
             DateTime::compareTime(DateTime::nowTime(), booking->endTime) < 0)) {
            error = "Chỉ có thể xác nhận hoàn tất sau khi ca đã kết thúc.";
            return false;
        }

        domainBooking.complete(requesterOwnerId, domainErr);
        booking->status = Booking::statusToText(domainBooking.getStatus());
        if (!booking_repository_.saveAll(bookings, error)) return false;

        std::string custErr;
        if (!customer_service_.recordSuccessfulBooking(
                booking->customerId, booking->bookingId, custErr)) {
            error = "Không thể cập nhật lịch sử đặt thành công: " + custErr;
            std::string rollbackError;
            if (!booking_repository_.saveAll(originalBookings, rollbackError)) {
                error += "; không thể khôi phục booking: " + rollbackError;
            }
            return false;
        }
        error.clear();
        return true;
    }
    error = "Không tìm thấy đơn đặt sân cần hoàn tất.";
    return false;
}



