#include "Booking/Booking.h"

Booking::Booking()
    : status_(BookingStatus::PENDING), total_vnd_(0) {}

Booking::Booking(const std::string& id, const std::string& customer, const std::string& field,
                 const std::string& date, BookingStatus status, const std::string& ownerId)
    : booking_id_(id), customer_id_(customer), field_id_(field), owner_id_(ownerId),
      date_(date), status_(status), total_vnd_(0) {}

// LÝ DO CHỌN: LinkedList tự quản lý bộ nhớ nhưng không có hàm tạo sao chép, nên sao chép mặc định
// sẽ dùng chung các node và bị giải phóng hai lần. Vì vậy phải sao chép sâu từng BookingDetail.
Booking::Booking(const Booking& other)
    : booking_id_(other.booking_id_), customer_id_(other.customer_id_), field_id_(other.field_id_),
      owner_id_(other.owner_id_), date_(other.date_), status_(other.status_),
      total_vnd_(other.total_vnd_) {
    other.details_.forEach([this](const BookingDetail& detail) { details_.push_back(detail); });
}

Booking& Booking::operator=(const Booking& other) {
    if (this == &other) {
        return *this;
    }
    booking_id_ = other.booking_id_;
    customer_id_ = other.customer_id_;
    field_id_ = other.field_id_;
    owner_id_ = other.owner_id_;
    date_ = other.date_;
    status_ = other.status_;
    total_vnd_ = other.total_vnd_;
    details_.clear();
    other.details_.forEach([this](const BookingDetail& detail) { details_.push_back(detail); });
    return *this;
}

const std::string& Booking::getBookingId() const { return booking_id_; }
const std::string& Booking::getCustomerId() const { return customer_id_; }
const std::string& Booking::getFieldId() const { return field_id_; }
const std::string& Booking::getDate() const { return date_; }
const std::string& Booking::getOwnerId() const { return owner_id_; }
BookingStatus Booking::getStatus() const { return status_; }
long long Booking::getTotalVnd() const { return total_vnd_; }
const LinkedList<BookingDetail>& Booking::getDetails() const { return details_; }

void Booking::addDetail(const BookingDetail& detail) {
    details_.push_back(detail);
    total_vnd_ += detail.getAmountVnd();
}


void Booking::setStatus(BookingStatus status) { status_ = status; }

bool Booking::canBeCancelledBy(const std::string& customerId, std::string& error) const {
    if (!customer_id_.empty() && customerId != customer_id_) {
        error = u8"Không có quyền hủy đơn đặt sân của khách hàng khác.";
        return false;
    }
    if (status_ == BookingStatus::COMPLETED || status_ == BookingStatus::CANCELLED) {
        error = u8"Đơn đã hoàn tất hoặc đã hủy, không thể hủy thêm.";
        return false;
    }
    return true;
}

bool Booking::cancel(const std::string& customerId, std::string& error) {
    if (!canBeCancelledBy(customerId, error)) return false;
    status_ = BookingStatus::CANCELLED;
    return true;
}

bool Booking::canBeCompletedBy(const std::string& ownerId, std::string& error) const {
    if (!owner_id_.empty() && ownerId != owner_id_) {
        error = u8"Không có quyền xác nhận đơn của cơ sở khác.";
        return false;
    }
    if (status_ == BookingStatus::CANCELLED) {
        error = u8"Không thể hoàn tất đơn đã hủy.";
        return false;
    }
    if (status_ == BookingStatus::COMPLETED) {
        error = u8"Đơn này đã được xác nhận hoàn tất.";
        return false;
    }
    return true;
}

bool Booking::complete(const std::string& ownerId, std::string& error) {
    if (!canBeCompletedBy(ownerId, error)) return false;
    status_ = BookingStatus::COMPLETED;
    return true;
}

bool Booking::isActive() const {
    return status_ == BookingStatus::CONFIRMED || status_ == BookingStatus::PENDING;
}

bool Booking::isCompleted() const {
    return status_ == BookingStatus::COMPLETED;
}

bool Booking::isCancelled() const {
    return status_ == BookingStatus::CANCELLED;
}

long long Booking::calculateTotalVnd() const {
    long long sum = 0;
    for (auto* n = details_.get_head(); n != nullptr; n = n->next) {
        sum += n->data.getAmountVnd();
    }
    return sum;
}
