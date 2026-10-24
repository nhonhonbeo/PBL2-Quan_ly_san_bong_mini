#pragma once

#include <string>
#include "Core/LinkedList.h"
#include "Booking/BookingDetail.h"

enum class BookingStatus { PENDING, CONFIRMED, COMPLETED, CANCELLED };

// LÝ DO CHỌN: LinkedList tránh phụ thuộc STL container cho danh sách ca nghiệp vụ.
class Booking {
    std::string booking_id_, customer_id_, field_id_, owner_id_, date_;
    BookingStatus status_;
    LinkedList<BookingDetail> details_;
    long long total_vnd_;
public:
    Booking();
    Booking(const std::string& bookingId, const std::string& customerId,
            const std::string& fieldId, const std::string& date,
            BookingStatus status = BookingStatus::PENDING,
            const std::string& ownerId = "");
    Booking(const Booking& other);
    Booking& operator=(const Booking& other);
    const std::string& getBookingId() const; const std::string& getCustomerId() const;
    const std::string& getFieldId() const; const std::string& getDate() const;
    const std::string& getOwnerId() const;
    BookingStatus getStatus() const; long long getTotalVnd() const;
    const LinkedList<BookingDetail>& getDetails() const;
    void addDetail(const BookingDetail& detail); void setStatus(BookingStatus status);
};
