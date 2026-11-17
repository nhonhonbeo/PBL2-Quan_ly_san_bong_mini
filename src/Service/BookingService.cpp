#include "Service/BookingService.h"
#include "Service/CustomerService.h"
#include "Service/PaymentService.h"

#include "Booking/Booking.h"
#include "Booking/BookingDetail.h"
#include "Core/DateTime.h"
#include "Core/HashTable.h"
#include "Repository/BookingRepository.h"
#include "Repository/VenueRepository.h"
#include "Service/VenueService.h"

BookingService::BookingService(BookingRepository& bookingRepository,
                               CustomerService& customerService,
                               PaymentService* paymentService)
    : booking_repository_(bookingRepository),
      customer_service_(customerService),
      venue_service_(nullptr),
      venue_repository_(nullptr),
      payment_service_(paymentService),
      default_venue_repo_(),
      default_venue_service_(default_venue_repo_) {
    venue_repository_ = &default_venue_repo_;
    venue_service_ = &default_venue_service_;
}

BookingService::BookingService(BookingRepository& bookingRepository,
                               CustomerService& customerService,
                               VenueService* venueService,
                               VenueRepository* venueRepository,
                               PaymentService* paymentService)
    : booking_repository_(bookingRepository),
      customer_service_(customerService),
      venue_service_(venueService),
      venue_repository_(venueRepository),
      payment_service_(paymentService),
      default_venue_repo_(),
      default_venue_service_(default_venue_repo_) {
    if (!venue_repository_) venue_repository_ = &default_venue_repo_;
    if (!venue_service_) venue_service_ = &default_venue_service_;
}

void BookingService::setPaymentService(PaymentService* paymentService) {
    payment_service_ = paymentService;
}

void BookingService::setVenueDependencies(VenueService* venueService, VenueRepository* venueRepository) {
    venue_service_ = venueService ? venueService : &default_venue_service_;
    venue_repository_ = venueRepository ? venueRepository : &default_venue_repo_;
}

bool BookingService::createBooking(const Booking& booking,
                                   std::string& error) {
    const LinkedList<BookingDetail>& details = booking.getDetails();
    const auto* detail = details.get_head();
    if (detail == nullptr || detail->next != nullptr) {
        error = "Chỉ hỗ trợ đặt đúng một khung giờ cho mỗi đơn.";
        return false;
    }
    std::string customerName;
    std::string customerPhone;
    if (!venue_repository_->getCustomerDetails(
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
    bool isBlocked = false;
    std::string blockReason;
    std::string checkErr;
    if (!customer_service_.isBlacklisted(request.customerId, isBlocked, blockReason, checkErr)) {
        error = checkErr.empty() ? "Loi kiem tra danh sach den." : checkErr;
        return false;
    }
    if (isBlocked) {
        error = "Tai khoan cua ban dang bi khoa do vi pham" +
                (blockReason.empty() ? "." : (": " + blockReason));
        return false;
    }

    LinkedList<VenueBookingRecord> originalBookings;
    if (!booking_repository_.loadAll(originalBookings, error)) {
        return false;
    }

    bool ok = venue_service_->createBooking(request, bookingId, error);
    if (!ok) {
        return false;
    }

    if (payment_service_) {
        // Lấy giá tiền chính xác của đơn đặt sân vừa tạo
        LinkedList<VenueBookingRecord> currentBookings;
        std::string loadErr;
        long long price = 0;
        if (booking_repository_.loadAll(currentBookings, loadErr)) {
            for (auto* n = currentBookings.get_head(); n != nullptr; n = n->next) {
                if (n->data.bookingId == bookingId) {
                    price = n->data.priceVnd;
                    break;
                }
            }
        }

        std::string invId, invErr;
        if (!payment_service_->createInvoice(bookingId, price, invId, invErr)) {
            // Rollback đơn đặt sân nếu tạo hóa đơn thất bại để tránh dữ liệu mồ côi
            std::string rollbackErr;
            if (!booking_repository_.saveAll(originalBookings, rollbackErr)) {
                error = "Tao hoa don that bai: " + invErr +
                        ". Khong the khoi phuc don dat san: " + rollbackErr;
            } else {
                error = "Tao hoa don that bai: " + invErr;
            }
            bookingId.clear();
            return false;
        }
    }
    return true;
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
    bool isBlocked = false;
    std::string blockReason;
    std::string checkErr;
    if (!customer_service_.isBlacklisted(first.customerId, isBlocked, blockReason, checkErr)) {
        error = checkErr.empty() ? "Loi kiem tra danh sach den." : checkErr;
        return false;
    }
    if (isBlocked) {
        error = "Tai khoan cua ban dang bi khoa do vi pham" +
                (blockReason.empty() ? "." : (": " + blockReason));
        return false;
    }

    // 1. Kiểm tra tính hợp lệ cơ bản của chuỗi ca liên tiếp
    int previousEnd = -1;
    for (auto* node = requests.get_head(); node != nullptr; node = node->next) {
        const VenueBookingRequest& request = node->data;
        if (request.fieldId != first.fieldId ||
            DateTime::toIsoDate(request.date) != DateTime::toIsoDate(first.date) ||
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

    // 2. Tải bản sao trạng thái ban đầu của Bookings để rollback khi gặp lỗi
    LinkedList<VenueBookingRecord> originalBookings;
    if (!booking_repository_.loadAll(originalBookings, error)) {
        return false;
    }

    // 3. Tiến hành đặt từng ca kèm theo dõi danh sách để rollback nguyên khối nếu bất kỳ bước nào thất bại
    LinkedList<std::string> createdBookingIds;
    LinkedList<std::string> createdInvoiceIds;
    bool allSuccess = true;

    for (auto* node = requests.get_head(); node != nullptr; node = node->next) {
        std::string currentBookingId;
        if (!venue_service_->createBooking(node->data, currentBookingId, error)) {
            allSuccess = false;
            break;
        }
        createdBookingIds.push_back(currentBookingId);

        if (payment_service_) {
            // Xác định chính xác giá tiền của đơn vừa ghi nhận
            LinkedList<VenueBookingRecord> currentBookings;
            std::string loadErr;
            long long price = 0;
            if (booking_repository_.loadAll(currentBookings, loadErr)) {
                for (auto* bNode = currentBookings.get_head(); bNode != nullptr; bNode = bNode->next) {
                    if (bNode->data.bookingId == currentBookingId) {
                        price = bNode->data.priceVnd;
                        break;
                    }
                }
            }

            std::string invId, invErr;
            if (!payment_service_->createInvoice(currentBookingId, price, invId, invErr)) {
                error = "Tao hoa don that bai: " + invErr;
                allSuccess = false;
                break;
            }
            createdInvoiceIds.push_back(currentBookingId);
        }
    }

    if (!allSuccess) {
        // Rollback toàn bộ đơn đặt sân
        std::string rollbackErr;
        if (!booking_repository_.saveAll(originalBookings, rollbackErr)) {
            error += ". Loi nghiem trong: Khong the khoi phuc trang thai bookings: " + rollbackErr;
        }
        // Rollback toàn bộ hóa đơn đã phát hành
        if (payment_service_) {
            for (auto* invNode = createdInvoiceIds.get_head(); invNode != nullptr; invNode = invNode->next) {
                std::string delErr;
                if (!payment_service_->rollbackInvoice(invNode->data, delErr)) { error += ". Khong the huy hoa don: " + delErr; }
            }
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
    for (auto* node = bookings.get_head(); node != nullptr; node = node->next) {
        VenueBookingRecord& booking = node->data;
        if (booking.bookingId != bookingId) continue;

        // Nghiệp vụ tự thân của Đơn đặt sân (Rich Domain Model)
        Booking domainBooking = Booking::fromRecord(booking);
        std::string domainErr;
        if (!domainBooking.cancel(customerId, domainErr)) {
            error = domainErr;
            return false;
        }

        booking.status = Booking::statusToText(domainBooking.getStatus());
        if (!booking_repository_.saveAll(bookings, error)) return false;

        std::string custErr;
        customer_service_.recordCancellation(booking.customerId, booking.bookingId, custErr);

        if (payment_service_) {
            std::string payErr;
            payment_service_->cancelInvoice(booking.bookingId, payErr);
        }
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
    for (auto* node = bookings.get_head(); node != nullptr; node = node->next) {
        VenueBookingRecord& booking = node->data;
        if (booking.bookingId != bookingId) continue;

        // Nghiệp vụ tự thân của Đơn đặt sân (Rich Domain Model)
        Booking domainBooking = Booking::fromRecord(booking);
        std::string domainErr;
        if (!domainBooking.canBeCompletedBy(requesterOwnerId, domainErr)) {
            error = domainErr;
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

        domainBooking.complete(requesterOwnerId, domainErr);
        booking.status = Booking::statusToText(domainBooking.getStatus());
        if (!booking_repository_.saveAll(bookings, error)) return false;

        std::string custErr;
        customer_service_.recordSuccessfulBooking(booking.customerId, booking.bookingId, custErr);

        if (payment_service_) {
            std::string invId = "INV-" + booking.bookingId;
            std::string payId, payErr;
            payment_service_->processPayment(invId, booking.priceVnd, PaymentMethod::TRANSFER, payId, payErr);
        }
        return true;
    }
    error = "Không tìm thấy đơn đặt sân cần hoàn tất.";
    return false;
}



