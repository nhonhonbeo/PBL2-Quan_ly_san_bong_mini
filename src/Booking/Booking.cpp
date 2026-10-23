#include "Booking/Booking.h"
Booking::Booking(const std::string& id, const std::string& customer, const std::string& field, const std::string& date, BookingStatus status) : booking_id_(id), customer_id_(customer), field_id_(field), date_(date), status_(status), total_vnd_(0) {}
const std::string& Booking::getBookingId() const { return booking_id_; } const std::string& Booking::getCustomerId() const { return customer_id_; }
const std::string& Booking::getFieldId() const { return field_id_; } const std::string& Booking::getDate() const { return date_; }
BookingStatus Booking::getStatus() const { return status_; } long long Booking::getTotalVnd() const { return total_vnd_; }
const LinkedList<BookingDetail>& Booking::getDetails() const { return details_; }
void Booking::addDetail(const BookingDetail& detail) { details_.push_back(detail); total_vnd_ += detail.getAmountVnd(); }
void Booking::setStatus(BookingStatus status) { status_ = status; }
