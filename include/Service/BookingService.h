#pragma once
#include <string>
#include "Core/LinkedList.h"
#include "Repository/VenueRepository.h"
#include "Service/VenueService.h"

class Booking;
class BookingRepository;
class CustomerService;
class PaymentService;
class VenueRepository;

// LÝ DO CHỌN: BookingService tập trung quản lý vòng đời đơn đặt sân, nhận VenueService,
// VenueRepository, CustomerService và PaymentService qua Dependency Injection.
class BookingService {
private:
    BookingRepository& booking_repository_;
    CustomerService& customer_service_;
    VenueService* venue_service_;
    VenueRepository* venue_repository_;
    PaymentService* payment_service_;
    VenueRepository default_venue_repo_;
    VenueService default_venue_service_;

public:
    BookingService(BookingRepository& bookingRepository,
                   CustomerService& customerService,
                   PaymentService* paymentService = nullptr);
    BookingService(BookingRepository& bookingRepository,
                   CustomerService& customerService,
                   VenueService* venueService,
                   VenueRepository* venueRepository,
                   PaymentService* paymentService = nullptr);
    void setPaymentService(PaymentService* paymentService);
    void setVenueDependencies(VenueService* venueService, VenueRepository* venueRepository);

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
