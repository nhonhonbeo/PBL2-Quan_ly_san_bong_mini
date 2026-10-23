#pragma once
#include <string>
class Booking; class BookingRepository; class CustomerService;
// LÝ DO CHỌN: BookingService là nơi duy nhất kiểm tra lịch trống trước khi lưu đơn.
class BookingService {
    BookingRepository& booking_repository_; CustomerService& customer_service_;
public:
    BookingService(BookingRepository& bookingRepository, CustomerService& customerService);
    bool createBooking(const Booking& booking, std::string& error);
    bool cancelBooking(const std::string& bookingId, const std::string& customerId, std::string& error);
};
