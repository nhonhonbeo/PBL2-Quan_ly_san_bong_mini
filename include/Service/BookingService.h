#pragma once
#include <string>
#include "Core/LinkedList.h"
#include "Repository/VenueRepository.h"
#include "Service/VenueService.h"
class Booking; class BookingRepository; class CustomerService;
// LÝ DO CHỌN: BookingService là nơi duy nhất kiểm tra lịch trống trước khi lưu đơn.
class BookingService {
    BookingRepository& booking_repository_; CustomerService& customer_service_;
public:
    BookingService(BookingRepository& bookingRepository, CustomerService& customerService);
    bool createBooking(const Booking& booking, std::string& error);
    bool createBooking(const VenueBookingRequest& request,
                       std::string& bookingId, std::string& error);
    bool createBookings(const LinkedList<VenueBookingRequest>& requests,
                        LinkedList<std::string>& bookingIds,
                        std::string& error);
    bool cancelBooking(const std::string& bookingId, const std::string& customerId, std::string& error);
    bool completeBooking(const std::string& requesterOwnerId,
                         const std::string& bookingId, std::string& error);
};
