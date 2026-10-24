#include "Customer/Discount.h"

Discount::Discount(const std::string& discountId,
                   const std::string& customerId, int percentage,
                   int qualifiedBookingCount, const std::string& grantedAt,
                   DiscountStatus status, const std::string& usedBookingId)
    : discount_id_(discountId), customer_id_(customerId), percentage_(percentage),
      qualified_booking_count_(qualifiedBookingCount), granted_at_(grantedAt),
      status_(status), used_booking_id_(usedBookingId) {}

const std::string& Discount::getDiscountId() const { return discount_id_; }

const std::string& Discount::getCustomerId() const { return customer_id_; }

int Discount::getPercentage() const { return percentage_; }

int Discount::getQualifiedBookingCount() const {
    return qualified_booking_count_;
}

const std::string& Discount::getGrantedAt() const { return granted_at_; }

DiscountStatus Discount::getStatus() const { return status_; }

const std::string& Discount::getUsedBookingId() const {
    return used_booking_id_;
}

bool Discount::isAvailable() const {
    return status_ == DiscountStatus::AVAILABLE;
}

void Discount::markUsed(const std::string& bookingId) {
    status_ = DiscountStatus::USED;
    used_booking_id_ = bookingId;
}

void Discount::markExpired() { status_ = DiscountStatus::EXPIRED; }
