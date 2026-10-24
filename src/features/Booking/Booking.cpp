#include "Booking/Booking.h"
Booking::Booking()
    : status_(BookingStatus::PENDING), total_vnd_(0) {}
Booking::Booking(const std::string& id, const std::string& customer,
                 const std::string& field, const std::string& date,
                 BookingStatus status, const std::string& ownerId)
    : booking_id_(id), customer_id_(customer), field_id_(field),
      owner_id_(ownerId), date_(date), status_(status), total_vnd_(0) {}
Booking::Booking(const Booking& other)
    : booking_id_(other.booking_id_), customer_id_(other.customer_id_),
      field_id_(other.field_id_), owner_id_(other.owner_id_), date_(other.date_),
      status_(other.status_), total_vnd_(other.total_vnd_) {
    for (auto* node = other.details_.get_head(); node != nullptr; node = node->next) {
        details_.push_back(node->data);
    }
}
Booking& Booking::operator=(const Booking& other) {
    if (this == &other) return *this;
    booking_id_ = other.booking_id_;
    customer_id_ = other.customer_id_;
    field_id_ = other.field_id_;
    owner_id_ = other.owner_id_;
    date_ = other.date_;
    status_ = other.status_;
    total_vnd_ = other.total_vnd_;
    details_.clear();
    for (auto* node = other.details_.get_head(); node != nullptr; node = node->next) {
        details_.push_back(node->data);
    }
    return *this;
}
const std::string& Booking::getBookingId() const { return booking_id_; } const std::string& Booking::getCustomerId() const { return customer_id_; }
const std::string& Booking::getFieldId() const { return field_id_; } const std::string& Booking::getDate() const { return date_; }
const std::string& Booking::getOwnerId() const { return owner_id_; }
BookingStatus Booking::getStatus() const { return status_; } long long Booking::getTotalVnd() const { return total_vnd_; }
const LinkedList<BookingDetail>& Booking::getDetails() const { return details_; }
void Booking::addDetail(const BookingDetail& detail) { details_.push_back(detail); total_vnd_ += detail.getAmountVnd(); }
void Booking::setStatus(BookingStatus status) { status_ = status; }
