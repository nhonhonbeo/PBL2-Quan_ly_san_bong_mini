#include "Customer/Customer.h"

CustomerProfile::CustomerProfile(const std::string& userId, int bookingCount,
                                 int cancelCount)
    : user_id_(userId), booking_count_(bookingCount),
      cancel_count_(cancelCount) {}

const std::string& CustomerProfile::getUserId() const { return user_id_; }

int CustomerProfile::getBookingCount() const { return booking_count_; }

int CustomerProfile::getCancelCount() const { return cancel_count_; }

void CustomerProfile::recordBooking() { ++booking_count_; }

void CustomerProfile::recordCancellation() { ++cancel_count_; }

void CustomerProfile::setStatistics(int bookingCount, int cancelCount) {
    booking_count_ = bookingCount;
    cancel_count_ = cancelCount;
}
